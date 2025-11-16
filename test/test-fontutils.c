/**
 * @file test-fontutils.c
 * @brief Test suite for fontutils-api.c file
 *
 * @author Alessandro Bridi [ale.bridi15@gmail.com] 
 * @date 2025-03-23
 */

#include "unity.h"
#include "fontutils-api.h"

void check_get_alpha() {
    struct Color color = { .argb = 0xff000000 };
    TEST_ASSERT_EQUAL_UINT8(0xff, color.a);
}

void check_get_red() {
    struct Color color = { .argb = 0x00ff0000 };
    TEST_ASSERT_EQUAL_UINT8(0xff, color.r);
}

void check_get_green() {
    struct Color color = { .argb = 0x0000ff00 };
    TEST_ASSERT_EQUAL_UINT8(0xff, color.g);
}

void check_get_blue() {
    struct Color color = { .argb = 0x000000ff };
    TEST_ASSERT_EQUAL_UINT8(0xff, color.b);
}

void check_color_components_combined() {
    struct Color color = { .argb = 0xAABBCCDD };
    TEST_ASSERT_EQUAL_UINT8(0xAA, color.a);
    TEST_ASSERT_EQUAL_UINT8(0xBB, color.r);
    TEST_ASSERT_EQUAL_UINT8(0xCC, color.g);
    TEST_ASSERT_EQUAL_UINT8(0xDD, color.b);
}

void check_color_individual_assignment() {
    struct Color color;
    color.a = 0x12;
    color.r = 0x34;
    color.g = 0x56;
    color.b = 0x78;
    
    TEST_ASSERT_EQUAL_UINT32(0x12345678, color.argb);
}

// Font is set to 0 as default for the following tests
void check_font_api_length() {
    uint16_t len = font_api_length("Test", 10, 0);
    TEST_ASSERT_GREATER_THAN(0, len);
}

void check_font_length_empty_string() {
    uint16_t len = font_api_length("", 10, 0);
    TEST_ASSERT_EQUAL_UINT16(0, len);
}

void check_font_length_single_char() {
    uint16_t len = font_api_length("A", 20, 0);
    TEST_ASSERT_GREATER_THAN(0, len);
}

void check_font_length_longer_text() {
    uint16_t len1 = font_api_length("A", 15, 0);
    uint16_t len2 = font_api_length("AB", 15, 0);
    TEST_ASSERT_GREATER_THAN(len1, len2);
}

void check_font_length_different_sizes() {
    uint16_t len_small = font_api_length("Test", 10, 0);
    uint16_t len_large = font_api_length("Test", 20, 0);
    TEST_ASSERT_GREATER_THAN(len_small, len_large);
}
