/*!
 * \file test-font.h
 * \brief Synthetic, hand-crafted Font used by the unit tests.
 *
 * \details The library no longer ships a default font, so the tests carry a
 *     tiny font with known widths to validate font_api_length and glyph
 *     lookup without invoking the generator.
 */

#ifndef TEST_FONT_H
#define TEST_FONT_H

#include "fontutils.h"

static const struct Glyph test_glyphs[] = {
    { ' ', 0, 0, 4, 20 },
    { 'A', 0, 0, 10, 20 },
    { 'B', 0, 0, 12, 20 },
    { 'C', 0, 0, 14, 20 },
    { 'T', 0, 0, 8, 20 },
    { 'e', 0, 0, 9, 20 },
    { 's', 0, 0, 7, 20 },
    { 't', 0, 0, 6, 20 },
};

static const uint8_t test_sdf_data[] = { 0 };

const struct Font test_font = {
    .base_size = 20,
    .sdf_data = test_sdf_data,
    .glyphs = test_glyphs,
    .glyph_count = sizeof(test_glyphs) / sizeof(test_glyphs[0]),
};

#endif // TEST_FONT_H
