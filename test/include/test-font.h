/*!
 * \file test-font.h
 * \brief Synthetic, hand-crafted Font used by the unit tests.
 *
 * \details The library does not ship a default font, so the tests carry a
 *     tiny in-line one with known widths to validate font_api_length, glyph
 *     lookup, and basic render dispatch without invoking the generator.
 *
 *     The full font lives in this header (everything is static so each test
 *     program gets its own private copy and there are no link conflicts).
 *     Every glyph shares the same single-pixel, fully opaque SDF blob, so
 *     calls to font_api_draw at native size produce one rectangle per glyph
 *     and the render path can be exercised through FFF.
 */

#ifndef TEST_FONT_H
#define TEST_FONT_H

#include "font.h"

#include <stddef.h>

/*!
 * \brief One fully opaque pixel encoded as an RLE triplet:
 *     - byte 0: alpha pair (0xF0, 0xF0)
 *     - byte 1: count for the first run (1 pixel)
 *     - byte 2: count for the second run (0 pixels, skipped)
 */
static const uint8_t test_sdf_data[] = {
    0xFF, 1, 0,
};

static const struct FontGlyph test_glyphs[] = {
    { ' ', 0, 3, 4, 20 },
    { 'A', 0, 3, 10, 20 },
    { 'B', 0, 3, 12, 20 },
    { 'C', 0, 3, 14, 20 },
    { 'T', 0, 3, 8, 20 },
    { 'e', 0, 3, 9, 20 },
    { 's', 0, 3, 7, 20 },
    { 't', 0, 3, 6, 20 },
};

static const struct FontGlyph *test_find_glyph(char c) {
    switch (c) {
        case ' ': return &test_glyphs[0];
        case 'A': return &test_glyphs[1];
        case 'B': return &test_glyphs[2];
        case 'C': return &test_glyphs[3];
        case 'T': return &test_glyphs[4];
        case 'e': return &test_glyphs[5];
        case 's': return &test_glyphs[6];
        case 't': return &test_glyphs[7];
        default: return NULL;
    }
}

static const struct Font test_font = {
    .base_size = 20,
    .sdf_data = test_sdf_data,
    .glyphs = test_glyphs,
    .glyph_count = sizeof(test_glyphs) / sizeof(test_glyphs[0]),
    .find_glyph = test_find_glyph,
};

#endif // TEST_FONT_H
