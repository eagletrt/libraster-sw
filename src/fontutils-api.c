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
static inline void prv_draw_rle_series(uint8_t count, uint8_t value, uint16_t x, uint16_t y, float multiplier, int16_t glyph_width, int16_t *current_x, int16_t *current_y, uint32_t color, draw_line_callback_t line_callback) {
    if (value < 30) {
        *current_x += count;
        *current_y += *current_x / glyph_width;
        *current_x %= glyph_width;
        return;
    }

    uint32_t blended_color = (color & 0x00ffffff) | ((uint32_t)value << 24);

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
        line_callback(start_x, start_y + j, draw_width, blended_color);
    }

    *current_x += count;
    *current_y += *current_x / glyph_width;
    *current_x %= glyph_width;
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
 */
static inline void prv_render_glyph(const struct Glyph *glyph, enum FontName font, uint16_t x, uint16_t y, float multiplier, uint32_t color, draw_line_callback_t line_callback) {
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

        prv_draw_rle_series(count1, value1, x, y, multiplier, glyph_width, &current_x, &current_y, color, line_callback);
        prv_draw_rle_series(count2, value2, x, y, multiplier, glyph_width, &current_x, &current_y, color, line_callback);
    }
}

void draw_text(uint16_t x, uint16_t y, enum FontAlign align, enum FontName font, const char *__restrict__ text, uint32_t color, uint16_t pixel_size, draw_line_callback_t line_callback) {
    if (align != FONT_ALIGN_LEFT) {
        uint16_t len = text_length(text, pixel_size, font);
        if (align == FONT_ALIGN_CENTER)
            x -= len / 2;
        else if (align == FONT_ALIGN_RIGHT)
            x -= len;
    }

    uint8_t glyph_height = fonts[font].glyphs[0].height;
    float multiplier = glyph_height ? (float)pixel_size / glyph_height : 1.0f;

    register char c;
    while ((c = *text++)) {
        const struct Glyph *glyph = find_glyph(font, c);
        if (glyph != 0) {
            prv_render_glyph(glyph, font, x, y, multiplier, color, line_callback);
            x += glyph->width * multiplier;
        }
    }
}

uint16_t text_length(const char *__restrict__ text, uint16_t pixel_size, enum FontName font) {
    float tot = 0;
    uint8_t glyph_height = fonts[font].glyphs[0].height;
    float multiplier = glyph_height ? (float)pixel_size / glyph_height : 1.0f;

    register char c;
    while ((c = *text++)) {
        const struct Glyph *glyph = find_glyph(font, c);
        if (glyph != 0) {
            tot += glyph->width * multiplier;
        }
    }
    return (uint16_t)tot;
}

uint8_t get_alpha(uint32_t color) {
    return color >> 24;
}

uint8_t get_red(uint32_t color) {
    return color >> 16;
}

uint8_t get_green(uint32_t color) {
    return color >> 8;
}

uint8_t get_blue(uint32_t color) {
    return color;
}
