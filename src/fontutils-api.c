/*!
 * \file fontutils-api.c
 * \date 2024-12-30
 * \author Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Glyph rendering implementation.
 *
 * \details Glyphs are stored as RLE-compressed SDF coverage values, packed
 *     as triplets of (value_byte, count1, count2). The high nibble of the
 *     value byte is the alpha for the first run, the low nibble is the
 *     alpha for the second run; both are scaled to the high half of an
 *     8-bit channel before being emitted.
 *
 *     Scaling uses Q16 fixed-point so the renderer never needs an FPU.
 *     A fast path bypasses the multiplications when the requested pixel
 *     size matches the font's native size.
 *
 *     Glyph lookup is delegated to a per-font switch-case function carried
 *     by the Font struct, so the library never has to walk a glyph array.
 */

#include "fontutils-api.h"
#include "eagletrt.h"
#include <stdbool.h>
#include <stddef.h>

/*!
 * \brief Coverage values strictly below this threshold are treated as fully
 *     transparent and skipped, since the corresponding rectangle would be
 *     barely visible while still costing a callback.
 */
#define FONT_ALPHA_THRESHOLD (30u)

/*!
 * \brief Q16 fixed-point one half, used for rounding before the right shift.
 */
#define FONT_Q16_HALF (0x8000u)

/*!
 * \brief Multiply an unsigned value by a Q16 multiplier with rounding.
 *
 * \param[in] value   Value to scale.
 * \param[in] mul_q16 Q16 fixed-point multiplier.
 *
 * \return The product, rounded to the nearest integer.
 */
EAGLETRT_STATIC_INLINE uint32_t prv_q16_mul(uint32_t value, uint32_t mul_q16) {
    return (value * mul_q16 + FONT_Q16_HALF) >> 16;
}

/*!
 * \brief Emit a single coverage run, splitting it at glyph row boundaries.
 *
 * \details The run starts at glyph-local position (\p *cx, \p *cy) and is
 *     advanced in place. When \p no_scale is true the run maps 1:1 to
 *     screen pixels; otherwise it goes through the Q16 scale.
 *
 * \param[in]     alpha     Coverage value (already in the upper half of a byte).
 * \param[in]     count     Number of glyph-native pixels in the run.
 * \param[in]     gw        Glyph width in native pixels.
 * \param[in]     no_scale  True when no scaling is required.
 * \param[in]     mul_q16   Q16 scaling factor (unused when \p no_scale is true).
 * \param[in]     ox        Screen X origin of the glyph.
 * \param[in]     oy        Screen Y origin of the glyph.
 * \param[in]     base_argb Base ARGB color with the alpha channel masked out.
 * \param[in,out] cx        Glyph-local X cursor.
 * \param[in,out] cy        Glyph-local Y cursor.
 * \param[in]     draw      Rectangle-fill callback.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_ERROR if the draw callback reported an error.
 */
EAGLETRT_STATIC enum RasterReturnCode prv_emit_run(uint8_t alpha, uint16_t count, uint16_t gw, bool no_scale, uint32_t mul_q16, uint16_t ox, uint16_t oy, uint32_t base_argb, int16_t *cx, int16_t *cy, raster_draw_rectangle_callback draw) {
    if (alpha < FONT_ALPHA_THRESHOLD) {
        uint32_t total = (uint32_t)(uint16_t)*cx + count;
        *cx = (int16_t)(total % gw);
        *cy += (int16_t)(total / gw);
        return RASTER_RC_OK;
    }

    struct Color color = { .argb = base_argb | ((uint32_t)alpha << 24) };

    while (count > 0u) {
        uint16_t avail = (uint16_t)(gw - (uint16_t)*cx);
        uint16_t take = count < avail ? count : avail;

        uint16_t px;
        uint16_t py;
        uint16_t pw;
        uint16_t ph;

        if (no_scale) {
            px = (uint16_t)(ox + (uint16_t)*cx);
            py = (uint16_t)(oy + (uint16_t)*cy);
            pw = take;
            ph = 1u;
        } else {
            uint32_t sx_q = (uint32_t)(uint16_t)*cx * mul_q16;
            uint32_t sy_q = (uint32_t)(uint16_t)*cy * mul_q16;
            uint32_t ex_q = (uint32_t)((uint16_t)*cx + take) * mul_q16;
            uint32_t ey_q = (uint32_t)((uint16_t)*cy + 1u) * mul_q16;
            uint16_t sx = (uint16_t)(sx_q >> 16);
            uint16_t sy = (uint16_t)(sy_q >> 16);
            uint16_t ex = (uint16_t)((ex_q + FONT_Q16_HALF) >> 16);
            uint16_t ey = (uint16_t)((ey_q + FONT_Q16_HALF) >> 16);
            px = (uint16_t)(ox + sx);
            py = (uint16_t)(oy + sy);
            pw = ex > sx ? (uint16_t)(ex - sx) : 1u;
            ph = ey > sy ? (uint16_t)(ey - sy) : 1u;
        }

        if (draw(px, py, pw, ph, color) != RASTER_RC_OK) {
            return RASTER_RC_ERROR;
        }

        *cx += (int16_t)take;
        count -= take;
        if (*cx >= (int16_t)gw) {
            *cx = 0;
            (*cy)++;
        }
    }

    return RASTER_RC_OK;
}

/*!
 * \brief Render a single glyph at the given screen origin.
 *
 * \param[in] glyph    Glyph to render.
 * \param[in] font     Font owning the glyph.
 * \param[in] ox       Screen X origin of the glyph.
 * \param[in] oy       Screen Y origin of the glyph.
 * \param[in] no_scale True when no scaling is required.
 * \param[in] mul_q16  Q16 scaling factor (unused when \p no_scale is true).
 * \param[in] color    Base text color.
 * \param[in] draw     Rectangle-fill callback.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_ERROR if the draw callback reported an error.
 */
EAGLETRT_STATIC enum RasterReturnCode prv_render_glyph(const struct Glyph *glyph, const struct Font *font, uint16_t ox, uint16_t oy, bool no_scale, uint32_t mul_q16, struct Color color, raster_draw_rectangle_callback draw) {
    const uint16_t gw = glyph->width;
    const uint16_t gh = glyph->height;
    if (gw == 0u || gh == 0u) {
        return RASTER_RC_OK;
    }

    const uint8_t *data = font->sdf_data + glyph->offset;
    const uint8_t *const end = data + glyph->size;
    const uint32_t base_argb = color.argb & 0x00FFFFFFu;

    int16_t cx = 0;
    int16_t cy = 0;

    while (data + 3 <= end && cy < (int16_t)gh) {
        uint8_t raw = data[0];
        uint8_t cnt1 = data[1];
        uint8_t cnt2 = data[2];
        data += 3;

        uint8_t alpha1 = (uint8_t)(raw & 0xF0u);
        uint8_t alpha2 = (uint8_t)((raw & 0x0Fu) << 4);

        if (cnt1 > 0u) {
            enum RasterReturnCode rc = prv_emit_run(alpha1, cnt1, gw, no_scale, mul_q16, ox, oy, base_argb, &cx, &cy, draw);
            if (rc != RASTER_RC_OK) {
                return rc;
            }
        }
        if (cnt2 > 0u) {
            enum RasterReturnCode rc = prv_emit_run(alpha2, cnt2, gw, no_scale, mul_q16, ox, oy, base_argb, &cx, &cy, draw);
            if (rc != RASTER_RC_OK) {
                return rc;
            }
        }
    }

    return RASTER_RC_OK;
}

const struct Glyph *font_find_glyph(const struct Font *font, char c) {
    if (font == NULL || font->find_glyph == NULL) {
        return NULL;
    }
    return font->find_glyph(c);
}

uint16_t font_api_length(const char *text, uint16_t pixel_size, const struct Font *font) {
    if (text == NULL || font == NULL || font->base_size == 0u) {
        return 0u;
    }

    const bool no_scale = (pixel_size == font->base_size);
    const uint32_t mul_q16 = no_scale ? 0u : ((uint32_t)pixel_size << 16) / font->base_size;

    uint32_t total = 0u;
    for (const char *p = text; *p != '\0'; ++p) {
        const struct Glyph *glyph = font_find_glyph(font, *p);
        if (glyph == NULL) {
            continue;
        }
        total += no_scale ? glyph->width : prv_q16_mul(glyph->width, mul_q16);
    }

    return total > 0xFFFFu ? 0xFFFFu : (uint16_t)total;
}

enum RasterReturnCode font_api_draw(uint16_t x, uint16_t y, enum FontAlign align, const struct Font *font, const char *text, struct Color color, uint16_t pixel_size, raster_draw_rectangle_callback draw) {
    if (font == NULL || text == NULL || draw == NULL) {
        return RASTER_RC_NULL_POINTER;
    }
    if (font->base_size == 0u) {
        return RASTER_RC_OK;
    }

    if (align != FONT_ALIGN_LEFT) {
        uint16_t len = font_api_length(text, pixel_size, font);
        if (len == 0u) {
            return RASTER_RC_OK;
        }
        if (align == FONT_ALIGN_CENTER) {
            x = (uint16_t)(x - len / 2u);
        } else {
            x = (uint16_t)(x - len);
        }
    }

    const bool no_scale = (pixel_size == font->base_size);
    const uint32_t mul_q16 = no_scale ? 0u : ((uint32_t)pixel_size << 16) / font->base_size;

    for (const char *p = text; *p != '\0'; ++p) {
        const struct Glyph *glyph = font_find_glyph(font, *p);
        if (glyph == NULL) {
            continue;
        }

        enum RasterReturnCode rc = prv_render_glyph(glyph, font, x, y, no_scale, mul_q16, color, draw);
        if (rc != RASTER_RC_OK) {
            return rc;
        }

        const uint16_t advance = no_scale ? glyph->width : (uint16_t)prv_q16_mul(glyph->width, mul_q16);
        x = (uint16_t)(x + advance);
    }

    return RASTER_RC_OK;
}
