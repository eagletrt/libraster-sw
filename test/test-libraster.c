/**
 * @file test-libraster.c
 * @brief Test suite for libraster.c file
 *
 * @author Alessandro Bridi [ale.bridi15@gmail.com] 
 * @date 2025-03-23
 */ 

#include "unity-h"
#include "libraster-api.h"
#include "fontutils-api.h"

void check_get_box_found() {
    Box boxes[] = {
        { 1, 0x1, { 1, 1, 1, 1 }, 0, 0, NULL, NULL },
        { 1, 0x2, { 1, 1, 1, 1 }, 0, 0, NULL, NULL }
    };
    
    Box *b = get_box(boxes, 2, 0x1);
    TEST_ASSERT_NOT_NULL(b);
}

void check_get_box_not_found() {
    Box boxes[] = {
        { 1, 0x1, { 1, 1, 1, 1 }, 0, 0, NULL, NULL },
        { 1, 0x2, { 1, 1, 1, 1 }, 0, 0, NULL, NULL }
    };
    
    Box *b = get_box(boxes, 2, 0x3);
    TEST_ASSERT_NULL(b);
}

void check_label_building() {
    Label l;
    FontName default;
    create_label(&l, "Hello, World", (Coords){ 1, 1 }, default, 15, FONT_ALIGN_CENTER);

    TEST_ASSERT_NOT_NULL(l);
}

void check_value_building() {
    Value v;
    FontName default;
    create_value(&v, 12.3, false, (Coords){ 1, 1 }, default, 15, FONT_ALIGN_CENTER, (Colors){ .interpolation = (LinearInterpolation){0, 0, 0.0, 0.0}}, INTERPOLATION);

    TEST_ASSERT_NOT_NULL(v);
}

int main() {
    UNITY_BEGIN();

    RUN_TEST(check_get_box_found);
    RUN_TEST(check_get_box_not_found);
    RUN_TEST(check_label_building);
    RUN_TEST(check_value_building);

    UNITY_END();
}

