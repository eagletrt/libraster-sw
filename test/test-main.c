/**
 * @file test-main.c
 * @brief Main entry point for all unit tests
 *
 * @author Alessandro Bridi [ale.bridi15@gmail.com] 
 * @date 2025-03-23
 */

#include "unity.h"

// fontutils tests declarations
void check_get_alpha(void);
void check_get_red(void);
void check_get_green(void);
void check_get_blue(void);
void check_color_components_combined(void);
void check_color_individual_assignment(void);
void check_font_api_length(void);
void check_font_length_empty_string(void);
void check_font_length_single_char(void);
void check_font_length_longer_text(void);
void check_font_length_different_sizes(void);

// libraster tests declarations
void check_get_box_found(void);
void check_get_box_not_found(void);
void check_label_building(void);
void check_label_data_update(void);
void check_label_float_1_decimal(void);
void check_label_float_2_decimals(void);
void check_label_float_3_decimals(void);
void check_label_string_type(void);
void check_get_box_with_multiple_boxes(void);
void check_get_box_first_element(void);
void check_get_box_last_element(void);
void check_set_label_data_float(void);
void check_set_label_data_string(void);
void check_set_label_data_null_box(void);
void check_box_with_label_properties(void);

int main(void) {
    UNITY_BEGIN();

    // fontutils tests - color component tests
    RUN_TEST(check_get_alpha);
    RUN_TEST(check_get_red);
    RUN_TEST(check_get_green);
    RUN_TEST(check_get_blue);
    RUN_TEST(check_color_components_combined);
    RUN_TEST(check_color_individual_assignment);

    // fontutils tests - font API length tests
    RUN_TEST(check_font_api_length);
    RUN_TEST(check_font_length_empty_string);
    RUN_TEST(check_font_length_single_char);
    RUN_TEST(check_font_length_longer_text);
    RUN_TEST(check_font_length_different_sizes);

    // raster tests - basic box operations
    RUN_TEST(check_get_box_found);
    RUN_TEST(check_get_box_not_found);
    RUN_TEST(check_get_box_with_multiple_boxes);
    RUN_TEST(check_get_box_first_element);
    RUN_TEST(check_get_box_last_element);

    // libraster tests - label creation and data types
    RUN_TEST(check_label_building);
    RUN_TEST(check_label_string_type);
    RUN_TEST(check_label_float_1_decimal);
    RUN_TEST(check_label_float_2_decimals);
    RUN_TEST(check_label_float_3_decimals);
    RUN_TEST(check_box_with_label_properties);

    // libraster tests - label data updates
    RUN_TEST(check_label_data_update);
    RUN_TEST(check_set_label_data_float);
    RUN_TEST(check_set_label_data_string);
    RUN_TEST(check_set_label_data_null_box);

    return UNITY_END();
}
