/**
 * @file test-main.c
 * @brief Unit-test entry point.
 *
 * @author Alessandro Bridi [ale.bridi15@gmail.com]
 * @date 2025-03-23
 */

#include "unity.h"

// fontutils
void check_get_alpha(void);
void check_get_red(void);
void check_get_green(void);
void check_get_blue(void);
void check_color_components_combined(void);
void check_color_individual_assignment(void);
void check_find_glyph_present(void);
void check_find_glyph_missing(void);
void check_find_glyph_null_font(void);
void check_font_length_native_size(void);
void check_font_length_empty_string(void);
void check_font_length_null_text(void);
void check_font_length_null_font(void);
void check_font_length_single_char(void);
void check_font_length_longer_text(void);
void check_font_length_different_sizes(void);
void check_font_length_skips_unknown_chars(void);

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(check_get_alpha);
    RUN_TEST(check_get_red);
    RUN_TEST(check_get_green);
    RUN_TEST(check_get_blue);
    RUN_TEST(check_color_components_combined);
    RUN_TEST(check_color_individual_assignment);

    RUN_TEST(check_find_glyph_present);
    RUN_TEST(check_find_glyph_missing);
    RUN_TEST(check_find_glyph_null_font);

    RUN_TEST(check_font_length_native_size);
    RUN_TEST(check_font_length_empty_string);
    RUN_TEST(check_font_length_null_text);
    RUN_TEST(check_font_length_null_font);
    RUN_TEST(check_font_length_single_char);
    RUN_TEST(check_font_length_longer_text);
    RUN_TEST(check_font_length_different_sizes);
    RUN_TEST(check_font_length_skips_unknown_chars);

    return UNITY_END();
}
