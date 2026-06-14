/*!
 * \file font-api.c
 * \date 2024-12-30
 * \author Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Glyph rendering implementation.
 *
 * \details Glyphs are stored as RLE-compressed SDF coverage values, packed
 *     as pairs of (alpha_byte, count_byte). The alpha byte is the coverage
 *     value for that run and is emitted unchanged into the ARGB alpha
 *     channel.
 *
 *     Scaling uses Q16 fixed-point so the renderer never needs an FPU.
 *
 *     Text input is UTF-8: the renderer decodes one Unicode codepoint at a
 *     time and dispatches the lookup through the per-font switch-case
 *     function carried by the Font struct, so the library never walks a
 *     glyph array.
 */

#include "font-api.h"
#include "eagletrt.h"
#include <stdbool.h>
#include <stddef.h>

/*!
 * \brief Coverage values strictly below this threshold are treated as fully
 *     transparent and skipped, since the corresponding rectangle would be
 *     barely visible while still costing a callback.
 *
 * \details The user can override the threshold by defining \c FONT_ALPHA_THRESHOLD before including the header.
 */
#ifndef FONT_ALPHA_THRESHOLD
#define FONT_ALPHA_THRESHOLD (30U)
#endif

/*!
 * \brief Q16 fixed-point one half, used for rounding before the right shift.
 */
#define FONT_Q16_HALF (0x8000U)

#define FONT_UTF8_1_BYTE_MASK (0x80U)                  /*!< Mask to identify 1-byte (ASCII) UTF-8 sequences. */
#define FONT_UTF8_2_BYTE_MASK (0xE0U)                  /*!< Mask to identify 2-byte UTF-8 sequences. */
#define FONT_UTF8_3_BYTE_MASK (0xF0U)                  /*!< Mask to identify 3-byte UTF-8 sequences. */
#define FONT_UTF8_4_BYTE_MASK (0xF8U)                  /*!< Mask to identify 4-byte UTF-8 sequences. */
#define FONT_UTF8_1_BYTE_MASK_RESULT (0x00U)           /*!< Expected result after masking the lead byte of a 1-byte UTF-8 sequence. */
#define FONT_UTF8_2_BYTE_MASK_RESULT (0xC0U)           /*!< Expected result after masking the lead byte of a 2-byte UTF-8 sequence. */
#define FONT_UTF8_3_BYTE_MASK_RESULT (0xE0U)           /*!< Expected result after masking the lead byte of a 3-byte UTF-8 sequence. */
#define FONT_UTF8_4_BYTE_MASK_RESULT (0xF0U)           /*!< Expected result after masking the lead byte of a 4-byte UTF-8 sequence. */
#define FONT_UTF8_CONTINUATION_MASK (0xC0U)            /*!< Mask to identify UTF-8 continuation bytes. */
#define FONT_UTF8_CONTINUATION_MASK_RESULT (0x80U)     /*!< Expected result after masking a UTF-8 continuation byte. */
#define FONT_UTF8_CONTINUATION_EXTRACTION_MASK (0x3FU) /*!< Mask to extract the payload bits from a UTF-8 continuation byte. */
#define FONT_UTF8_2_BYTE_EXTRACTION_MASK (0x1FU)       /*!< Mask to extract the payload bits from the lead byte of a 2-byte UTF-8 sequence. */
#define FONT_UTF8_3_BYTE_EXTRACTION_MASK (0x0FU)       /*!< Mask to extract the payload bits from the lead byte of a 3-byte UTF-8 sequence. */
#define FONT_UTF8_4_BYTE_EXTRACTION_MASK (0x07U)       /*!< Mask to extract the payload bits from the lead byte of a 4-byte UTF-8 sequence. */

/*!
 * \brief Multiply an unsigned value by a Q16 multiplier with rounding.
 *
 * \param[in] value          Value to scale.
 * \param[in] multiplier_q16 Q16 fixed-point multiplier.
 *
 * \return The product, rounded to the nearest integer.
 */
EAGLETRT_STATIC_INLINE uint32_t prv_q16_multiply(uint32_t value, uint32_t multiplier_q16) {
    constexpr uint32_t shift = 16U;
    return (value * multiplier_q16 + FONT_Q16_HALF) >> shift;
}

/*!
 * \brief Decode one UTF-8 codepoint from a NUL-terminated byte stream.
 *
 * \details The leading byte selects the sequence length:
 *     - \c 0xxxxxxx -> 1 byte  (ASCII)
 *     - \c 110xxxxx -> 2 bytes
 *     - \c 1110xxxx -> 3 bytes
 *     - \c 11110xxx -> 4 bytes
 *
 * \param[in]  text       Pointer to a byte in a NULL-terminated UTF-8 string.
 * \param[out] codepoint  Decoded Unicode codepoint, or 0 on invalid input.
 *
 * \return Number of bytes consumed from \p text (1..4). Never returns 0
 *     so that a caller's iteration always makes progress.
 */
EAGLETRT_STATIC uint8_t prv_utf8_decode(const char *text, uint32_t *codepoint) {
    const uint8_t lead = (uint8_t)text[0];

    // if it's standard ASCII
    if ((lead & FONT_UTF8_1_BYTE_MASK) == FONT_UTF8_1_BYTE_MASK_RESULT) {
        *codepoint = lead;
        return 1U;
    }

    uint8_t expected_byte_count;
    uint32_t accumulator;
    if ((lead & FONT_UTF8_2_BYTE_MASK) == FONT_UTF8_2_BYTE_MASK_RESULT) {
        expected_byte_count = 2U;
        accumulator = lead & FONT_UTF8_2_BYTE_EXTRACTION_MASK;
    } else if ((lead & FONT_UTF8_3_BYTE_MASK) == FONT_UTF8_3_BYTE_MASK_RESULT) {
        expected_byte_count = 3U;
        accumulator = lead & FONT_UTF8_3_BYTE_EXTRACTION_MASK;
    } else if ((lead & FONT_UTF8_4_BYTE_MASK) == FONT_UTF8_4_BYTE_MASK_RESULT) {
        expected_byte_count = 4U;
        accumulator = lead & FONT_UTF8_4_BYTE_EXTRACTION_MASK;
    } else {
        *codepoint = 0U;
        return 1U;
    }

    for (uint8_t i = 1U; i < expected_byte_count; ++i) {
        const uint8_t continuation = (uint8_t)text[i];
        if ((continuation & FONT_UTF8_CONTINUATION_MASK) != FONT_UTF8_CONTINUATION_MASK_RESULT) {
            *codepoint = 0U;
            return 1U;
        }
        accumulator = (accumulator << 6) | (continuation & FONT_UTF8_CONTINUATION_EXTRACTION_MASK);
    }
    *codepoint = accumulator;
    return expected_byte_count;
}

/*!
 * \brief Emit a single coverage run, splitting it at glyph row boundaries.
 *
 * \details The run starts at glyph-local position (\p *current_x, \p *ccurrent_y) and is
 *     advanced in place.
 *
 * \param[in]     alpha          Coverage value (already in the upper half of a byte).
 * \param[in]     count          Number of glyph-native pixels in the run.
 * \param[in]     glyph_width    Glyph width in native pixels.
 * \param[in]     multiplier_q16 Q16 scaling factor (unused when \p no_scale is true).
 * \param[in]     origin_x       Screen X origin of the glyph.
 * \param[in]     origin_y       Screen Y origin of the glyph.
 * \param[in]     base_argb      Base ARGB color with the alpha channel masked out.
 * \param[in,out] current_x      Glyph-local X cursor.
 * \param[in,out] current_y      Glyph-local Y cursor.
 * \param[in]     draw           Rectangle-fill callback.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_ERROR if the draw callback reported an error.
 */
EAGLETRT_STATIC enum RasterReturnCode prv_emit_run(uint8_t alpha, uint16_t count, uint16_t glyph_width, uint32_t multiplier_q16, uint16_t origin_x, uint16_t origin_y, uint32_t base_argb, int16_t *current_x, int16_t *current_y, raster_draw_rectangle_callback draw) {
    if (alpha < FONT_ALPHA_THRESHOLD) {
        int16_t total = *current_x + count;
        *current_x = total % glyph_width;
        *current_y += total / glyph_width;
        return RASTER_RC_OK;
    }

    struct Color color = { .argb = base_argb | ((uint32_t)alpha << 24) };

    while (count > 0U) {
        uint16_t available = (uint16_t)(glyph_width - (uint16_t)*current_x);
        uint16_t take = count < available ? count : available;

        uint16_t rectangle_x;
        uint16_t rectangle_y;
        uint16_t rectangle_w;
        uint16_t rectangle_h;

        uint32_t scaled_x_q = (uint32_t)*current_x * multiplier_q16;
        uint32_t scaled_y_q = (uint32_t)*current_y * multiplier_q16;
        uint32_t next_x_q = (uint32_t)(*current_x + take) * multiplier_q16;
        uint32_t next_y_q = (uint32_t)(*current_y + 1u) * multiplier_q16;
        uint16_t scaled_x = (uint16_t)(scaled_x_q >> 16);
        uint16_t scaled_y = (uint16_t)(scaled_y_q >> 16);
        uint16_t next_x = (uint16_t)((next_x_q + FONT_Q16_HALF) >> 16);
        uint16_t next_y = (uint16_t)((next_y_q + FONT_Q16_HALF) >> 16);
        rectangle_x = (uint16_t)(origin_x + scaled_x);
        rectangle_y = (uint16_t)(origin_y + scaled_y);
        rectangle_w = next_x > scaled_x ? (uint16_t)(next_x - scaled_x) : 1u;
        rectangle_h = next_y > scaled_y ? (uint16_t)(next_y - scaled_y) : 1u;

        if (draw(rectangle_x, rectangle_y, rectangle_w, rectangle_h, color) != RASTER_RC_OK) {
            return RASTER_RC_ERROR;
        }

        *current_x += (int16_t)take;
        count -= take;
        if (*current_x >= (int16_t)glyph_width) {
            *current_x = 0;
            (*current_y)++;
        }
    }

    return RASTER_RC_OK;
}

/*!
 * \brief Render a single glyph at the given screen origin.
 *
 * \param[in] glyph           Glyph to render.
 * \param[in] font            Font owning the glyph.
 * \param[in] origin_x        Screen X origin of the glyph.
 * \param[in] origin_y        Screen Y origin of the glyph.
 * \param[in] multiplier_q16  Q16 scaling factor (unused when \p no_scale is true).
 * \param[in] color           Base text color.
 * \param[in] draw            Rectangle-fill callback.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_ERROR if the draw callback reported an error.
 */
EAGLETRT_STATIC enum RasterReturnCode prv_render_glyph(const struct FontGlyph *glyph, const struct Font *font, uint16_t origin_x, uint16_t origin_y, uint32_t multiplier_q16, struct Color color, raster_draw_rectangle_callback draw) {
    const uint16_t glyph_width = glyph->width;
    const uint16_t glyph_height = glyph->height;
    if (glyph_width == 0U || glyph_height == 0U) {
        return RASTER_RC_OK;
    }

    const uint8_t *data = font->sdf_data + glyph->offset;
    const uint8_t *const end = data + glyph->size;
    const uint32_t base_argb = color.argb & 0x00FFFFFFU;

    int16_t current_x = 0;
    int16_t current_y = 0;

    while (data + 2 <= end && current_y < (int16_t)glyph_height) {
        uint8_t alpha = data[0];
        uint8_t count = data[1];
        data += 2;

        if (count == 0U) {
            continue;
        }

        enum RasterReturnCode rc = prv_emit_run(alpha, count, glyph_width, multiplier_q16, origin_x, origin_y, base_argb, &current_x, &current_y, draw);
        if (rc != RASTER_RC_OK) {
            return rc;
        }
    }

    return RASTER_RC_OK;
}

const struct FontGlyph *font_api_find_glyph(const struct Font *font, uint32_t codepoint) {
    if (font == NULL || font->find_glyph == NULL) {
        return NULL;
    }
    return font->find_glyph(codepoint);
}

uint16_t font_api_length(const char *text, uint16_t pixel_size, const struct Font *font) {
    if (text == NULL || font == NULL || font->base_size == 0U) {
        return 0U;
    }

    const uint32_t mul_q16 = ((uint32_t)pixel_size << 16) / font->base_size;

    uint32_t total = 0u;
    for (const char *p = text; *p != '\0';) {
        uint32_t codepoint = 0U;
        p += prv_utf8_decode(p, &codepoint);
        if (codepoint == 0U) {
            continue;
        }
        const struct FontGlyph *glyph = font_api_find_glyph(font, codepoint);
        if (glyph == NULL) {
            continue;
        }
        total += prv_q16_multiply(glyph->width, mul_q16);
    }

    return total > 0xFFFFU ? 0xFFFFU : (uint16_t)total;
}

enum RasterReturnCode font_api_draw(uint16_t x, uint16_t y, enum FontAlignment alignment, const struct Font *font, const char *text, struct Color color, uint16_t pixel_size, raster_draw_rectangle_callback draw) {
    if (font == NULL || text == NULL || draw == NULL) {
        return RASTER_RC_NULL_POINTER;
    }
    if (font->base_size == 0U) {
        return RASTER_RC_OK;
    }

    if (alignment != FONT_ALIGN_LEFT) {
        uint16_t len = font_api_length(text, pixel_size, font);
        if (len == 0U) {
            return RASTER_RC_OK;
        }
        x -= len;
        if (alignment == FONT_ALIGN_CENTER) {
            x += len / 2U;
        }
    }

    const uint32_t multiplier_q16 = ((uint32_t)pixel_size << 16) / font->base_size;

    for (const char *p = text; *p != '\0';) {
        uint32_t codepoint = 0U;
        p += prv_utf8_decode(p, &codepoint);
        if (codepoint == 0U) {
            continue;
        }
        const struct FontGlyph *glyph = font_api_find_glyph(font, codepoint);
        if (glyph == NULL) {
            continue;
        }

        enum RasterReturnCode rc = prv_render_glyph(glyph, font, x, y, multiplier_q16, color, draw);
        if (rc != RASTER_RC_OK) {
            return rc;
        }

        const uint16_t advance = (uint16_t)prv_q16_multiply(glyph->width, multiplier_q16);
        x += advance;
    }

    return RASTER_RC_OK;
}
