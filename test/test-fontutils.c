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

void check_font_api_length() {
    uint16_t len = font_api_length("Test", 10, FONT_KONEXY);
    TEST_ASSERT_GREATER_THAN(0, len);
}

int main() {
    UNITY_BEGIN();

    RUN_TEST(check_get_alpha);
    RUN_TEST(check_get_red);
    RUN_TEST(check_get_green);
    RUN_TEST(check_get_blue);
    RUN_TEST(check_font_api_length);

    UNITY_END();
}
