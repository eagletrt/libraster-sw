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
    uint8_t a = get_alpha(0xff000000);
    TEST_ASSERT_EQUAL_UINT8(0xff, a);
}

void check_get_red() {
    uint8_t r = get_red(0x00ff0000);
    TEST_ASSERT_EQUAL_UINT8(0xff, r);
}

void check_get_green() {
    uint8_t g = get_green(0x0000ff00);
    TEST_ASSERT_EQUAL_UINT8(0xff, g);
}

void check_get_blue() {
    uint8_t b = get_blue(0x000000ff);
    TEST_ASSERT_EQUAL_UINT8(0xff, b);
}

int main() {
    UNITY_BEGIN();

    RUN_TEST(check_get_alpha);
    RUN_TEST(check_get_red);
    RUN_TEST(check_get_green);
    RUN_TEST(check_get_blue);

    UNITY_END();
}

