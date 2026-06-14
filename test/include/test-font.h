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
 *
 *     Glyph lookup is keyed by Unicode codepoint. The fixture maps a handful
 *     of ASCII glyphs plus the degree sign (U+00B0) so tests can exercise
 *     multi-byte UTF-8 decoding without pulling in a real font.
 */

#ifndef TEST_FONT_H
#define TEST_FONT_H

#include "font.h"

#include <stddef.h>

/*!
 * \brief One fully opaque pixel encoded as an RLE pair:
 *     - byte 0: alpha (0xFF = opaque)
 *     - byte 1: count (1 pixel)
 */
static const uint8_t test_sdf_data[] = {
    0xFF, 1,
};

static const struct FontGlyph test_glyphs[] = {
    { 0x0020U, 0, 2, 4, 20 },  /* ' '                      */
    { 0x0041U, 0, 2, 10, 20 }, /* 'A'                      */
    { 0x0042U, 0, 2, 12, 20 }, /* 'B'                      */
    { 0x0043U, 0, 2, 14, 20 }, /* 'C'                      */
    { 0x0054U, 0, 2, 8, 20 },  /* 'T'                      */
    { 0x0065U, 0, 2, 9, 20 },  /* 'e'                      */
    { 0x0073U, 0, 2, 7, 20 },  /* 's'                      */
    { 0x0074U, 0, 2, 6, 20 },  /* 't'                      */
    { 0x00B0U, 0, 2, 5, 20 },  /* '°' (U+00B0, 2-byte UTF-8) */
};

static const struct FontGlyph *test_find_glyph(uint32_t codepoint) {
    switch (codepoint) {
        case 0x0020U: return &test_glyphs[0];
        case 0x0041U: return &test_glyphs[1];
        case 0x0042U: return &test_glyphs[2];
        case 0x0043U: return &test_glyphs[3];
        case 0x0054U: return &test_glyphs[4];
        case 0x0065U: return &test_glyphs[5];
        case 0x0073U: return &test_glyphs[6];
        case 0x0074U: return &test_glyphs[7];
        case 0x00B0U: return &test_glyphs[8];
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
