/**
 * @file test-fontutils.c
 * @brief Test suite for fontutils-api.c
 *
 * @author Alessandro Bridi [ale.bridi15@gmail.com]
 * @date 2025-03-23
 */

#include "fontutils-api.h"
#include "test-font.h"
#include "unity.h"

void check_get_alpha(void) {
    struct Color color = { .argb = 0xFF000000 };
    TEST_ASSERT_EQUAL_UINT8(0xFF, color.a);
}

void check_get_red(void) {
    struct Color color = { .argb = 0x00FF0000 };
    TEST_ASSERT_EQUAL_UINT8(0xFF, color.r);
}

void check_get_green(void) {
    struct Color color = { .argb = 0x0000FF00 };
    TEST_ASSERT_EQUAL_UINT8(0xFF, color.g);
}

void check_get_blue(void) {
    struct Color color = { .argb = 0x000000FF };
    TEST_ASSERT_EQUAL_UINT8(0xFF, color.b);
}

void check_color_components_combined(void) {
    struct Color color = { .argb = 0xAABBCCDD };
    TEST_ASSERT_EQUAL_UINT8(0xAA, color.a);
    TEST_ASSERT_EQUAL_UINT8(0xBB, color.r);
    TEST_ASSERT_EQUAL_UINT8(0xCC, color.g);
    TEST_ASSERT_EQUAL_UINT8(0xDD, color.b);
}

void check_color_individual_assignment(void) {
    struct Color color;
    color.a = 0x12;
    color.r = 0x34;
    color.g = 0x56;
    color.b = 0x78;
    TEST_ASSERT_EQUAL_UINT32(0x12345678, color.argb);
}

void check_find_glyph_present(void) {
    const struct Glyph *g = font_find_glyph(&test_font, 'A');
    TEST_ASSERT_NOT_NULL(g);
    TEST_ASSERT_EQUAL_INT('A', g->character);
    TEST_ASSERT_EQUAL_UINT16(10, g->width);
}

void check_find_glyph_missing(void) {
    TEST_ASSERT_NULL(font_find_glyph(&test_font, 'Z'));
}

void check_find_glyph_null_font(void) {
    TEST_ASSERT_NULL(font_find_glyph(NULL, 'A'));
}

// At native size (no scaling): width is the sum of glyph widths.
void check_font_length_native_size(void) {
    // "Test" = T(8) + e(9) + s(7) + t(6) = 30
    uint16_t len = font_api_length("Test", test_font.base_size, &test_font);
    TEST_ASSERT_EQUAL_UINT16(30, len);
}

void check_font_length_empty_string(void) {
    TEST_ASSERT_EQUAL_UINT16(0, font_api_length("", test_font.base_size, &test_font));
}

void check_font_length_null_text(void) {
    TEST_ASSERT_EQUAL_UINT16(0, font_api_length(NULL, test_font.base_size, &test_font));
}

void check_font_length_null_font(void) {
    TEST_ASSERT_EQUAL_UINT16(0, font_api_length("ABC", 20, NULL));
}

void check_font_length_single_char(void) {
    // 'A' width 10 at native size 20.
    TEST_ASSERT_EQUAL_UINT16(10, font_api_length("A", test_font.base_size, &test_font));
}

void check_font_length_longer_text(void) {
    uint16_t a = font_api_length("A", test_font.base_size, &test_font);
    uint16_t ab = font_api_length("AB", test_font.base_size, &test_font);
    TEST_ASSERT_GREATER_THAN(a, ab);
}

void check_font_length_different_sizes(void) {
    // base_size = 20; double the size -> double the width.
    uint16_t small = font_api_length("Test", 20, &test_font);
    uint16_t big = font_api_length("Test", 40, &test_font);
    TEST_ASSERT_GREATER_THAN(small, big);
    TEST_ASSERT_EQUAL_UINT16(small * 2, big);
}

void check_font_length_skips_unknown_chars(void) {
    // 'Z' is not in test_font; should be skipped.
    uint16_t with_z = font_api_length("AZB", test_font.base_size, &test_font);
    uint16_t without = font_api_length("AB", test_font.base_size, &test_font);
    TEST_ASSERT_EQUAL_UINT16(without, with_z);
}
