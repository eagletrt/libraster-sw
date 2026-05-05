/*!
 * \file fontutils-api.c
 * \date 2024-12-30
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief FontUtils APIs functions implementations
 *
 * \details A rasterizer is a software renderer that works by writing pixels to
 *     a framebuffer.
 *     This implementation lets the user define 3 callbacks that defines the
 *     technique used to write them, so that hardware accelerators can be used
 *     to archive big speedups.
 */

#include "fontutils-api.h"
#include "eagletrt.h"
#include "fontutils.h"
#include "raster.h"
#include <stddef.h>

/*!
 * \brief Draw a run-length encoded series of pixel_size
 *
 * \details This function draws a series of pixels encoded with run-length
 *     encoding (RLE). It calculates the position and size of the pixels to be
 *     drawn based on the provided parameters, including scaling multiplier.
 * 
 * \param[in] count Number of pixels in the series
 * \param[in] value Alpha value of the pixels in the series
 * \param[in] x X position of the glyph
 * \param[in] y Y position of the glyph
 * \param[in] multiplier Scaling multiplier
 * \param[in] glyph_width Width of the glyph
 * \param[in,out] current_x Current X position in the glyph
 * \param[in,out] current_y Current Y position in the glyph
 * \param[in] color Base color of the glyph
 * \param[in] line_callback Callback to draw a horizontal line of pixels
 */
EAGLETRT_STATIC_INLINE enum FontReturnCode prv_font_api_draw_rle_series(uint8_t count, uint8_t value, uint16_t x, uint16_t y, float multiplier, int16_t glyph_width, int16_t *current_x, int16_t *current_y, struct Color color, font_draw_line_callback line_callback) {
    if (value < 30) {
        *current_x += count;
        *current_y += *current_x / glyph_width;
        *current_x %= glyph_width;
        return FONT_RC_OK;
    }

    uint32_t blended_color = (color.argb & 0x00ffffff) | ((uint32_t)value << 24);

    int16_t start_x = x + (*current_x * multiplier);
    int16_t start_y = y + (*current_y * multiplier);
    int16_t end_x = x + ((*current_x + count) * multiplier);
    int16_t end_y = y + ((*current_y + 1) * multiplier);
    int16_t draw_width = (int16_t)(end_x - start_x + 0.5f);
    int16_t draw_height = (int16_t)(end_y - start_y + 0.5f);

    if (draw_width < 1)
        draw_width = 1;
    if (draw_height < 1)
        draw_height = 1;

    // Fill any potential gaps when scaling by ensuring consecutive rows are drawn
    for (int j = 0; j < draw_height; ++j) {
        if (line_callback(start_x, start_y + j, draw_width, (struct Color){ .argb = blended_color }) == FONT_RC_ERROR) {
            return FONT_RC_ERROR;
        }
    }

    *current_x += count;
    *current_y += *current_x / glyph_width;
    *current_x %= glyph_width;
    return FONT_RC_OK;
}

/*!
 * \brief Render a glyph at a specified position with scaling and color
 *
 * \details This function renders a glyph at the specified position (x, y)
 *     with the given scaling multiplier and color. It processes the glyph's
 *     SDF data using run-length encoding (RLE) to efficiently draw the pixels.
 * 
 * \param[in] glyph Pointer to the Glyph structure to be rendered
 * \param[in] font Font name enumeration
 * \param[in] x X position to render the glyph
 * \param[in] y Y position to render the glyph
 * \param[in] multiplier Scaling multiplier for the glyph size
 * \param[in] color Base color of the glyph
 * \param[in] line_callback Callback to draw a horizontal line of pixels
 *
 * \retval FONT_RC_OK if the glyph was rendered successfully
 * \retval FONT_RC_ERROR if there was an error rendering the glyph
 * \retval FONT_RC_NULL_POINTER if the glyph pointer or line_callback is NULL
 */
EAGLETRT_STATIC_INLINE enum RasterReturnCode prv_font_api_render_glyph(const struct Glyph *glyph, enum FontName font, uint16_t x, uint16_t y, float multiplier, struct Color color, font_draw_line_callback line_callback) {
    if (glyph == NULL || line_callback == NULL) {
        return RASTER_RC_NULL_POINTER;
    }

    const uint8_t *data = &fonts[font].sdf_data[glyph->offset];
    uint16_t remaining_size = glyph->size;
    uint16_t glyph_width = glyph->width;
    uint16_t glyph_height = glyph->height;

    int16_t current_x = 0;
    int16_t current_y = 0;

    while (remaining_size > 0 && current_y < glyph_height) {
        uint8_t value_raw = *data++;
        uint8_t value1 = (value_raw & 0xF0);
        uint8_t value2 = (value_raw << 4);
        uint8_t count1 = *data++;
        uint8_t count2 = *data++;
        remaining_size -= 2;

        enum FontReturnCode rc1 = prv_font_api_draw_rle_series(count1, value1, x, y, multiplier, glyph_width, &current_x, &current_y, color, line_callback);
        enum FontReturnCode rc2 = prv_font_api_draw_rle_series(count2, value2, x, y, multiplier, glyph_width, &current_x, &current_y, color, line_callback);

        if (rc1 != FONT_RC_OK || rc2 != FONT_RC_OK) {
            return RASTER_RC_ERROR;
        }
    }

    return RASTER_RC_OK;
}

enum FontReturnCode font_api_draw(uint16_t x, uint16_t y, enum FontAlign align, enum FontName font, const char *__restrict__ text, struct Color color, uint16_t pixel_size, font_draw_line_callback line_callback) {
    if (text == NULL || line_callback == NULL) {
        return FONT_RC_NULL_POINTER;
    }

    // Adjust x position based on alignment
    if (align != FONT_ALIGN_LEFT) {
        uint16_t len = font_api_length(text, pixel_size, font);
        if (len == 0) {
            return FONT_RC_OK; // Nothing to draw, so consider it successful
        }
        if (align == FONT_ALIGN_CENTER)
            x -= len / 2;
        else if (align == FONT_ALIGN_RIGHT)
            x -= len;
    }

    // Calculate scaling multiplier
    uint8_t glyph_height = fonts[font].glyphs[0].height;
    float multiplier = glyph_height ? (float)pixel_size / glyph_height : 1.0f;

    // Render each character in the text
    register char c;
    while ((c = *text++)) {
        const struct Glyph *glyph = fonts_find_glyph(font, c);
        if (glyph != NULL) {
            prv_font_api_render_glyph(glyph, font, x, y, multiplier, color, line_callback);
            x += glyph->width * multiplier;
        }
    }
}

uint16_t font_api_length(const char *__restrict__ text, uint16_t pixel_size, enum FontName font) {
    if (text == NULL) {
        return 0;
    }
    float tot = 0;
    uint8_t glyph_height = fonts[font].glyphs[0].height;
    float multiplier = glyph_height ? (float)pixel_size / glyph_height : 1.0f;

    register char c;
    while ((c = *text++)) {
        const struct Glyph *glyph = fonts_find_glyph(font, c);
        if (glyph != NULL) {
            tot += glyph->width * multiplier;
        }
    }
    return (uint16_t)tot;
}
