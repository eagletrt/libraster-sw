/**
 * @file test-libraster.c
 * @brief Test suite for raster-api.c file
 *
 * @author Alessandro Bridi [ale.bridi15@gmail.com] 
 * @date 2025-03-23
 */

#include <stddef.h>
#include "unity.h"
#include "raster-api.h"

void check_get_box_found() {
    struct Box boxes[] = {
        { true, 0x1, { 1, 1, 1, 1 }, { .argb = 0xFF000000 }, NULL },
        { true, 0x2, { 1, 1, 1, 1 }, { .argb = 0xFF000000 }, NULL }
    };

    struct Box *b = raster_api_get_box(boxes, 2, 0x1);
    TEST_ASSERT_NOT_NULL(b);
}

void check_get_box_not_found() {
    struct Box boxes[] = {
        { true, 0x1, { 1, 1, 1, 1 }, { .argb = 0xFF000000 }, NULL },
        { true, 0x2, { 1, 1, 1, 1 }, { .argb = 0xFF000000 }, NULL }
    };

    struct Box *b = raster_api_get_box(boxes, 2, 0x3);
    TEST_ASSERT_NULL(b);
}

void check_label_building() {
    struct Label l;
    raster_api_create_label(&l, 
        (union LabelData){ .text = "Hello, World" }, 
        LABEL_DATA_STRING,
        (struct Coords){ 1, 1 }, 
        FONT_KONEXY, 
        15, 
        FONT_ALIGN_CENTER,
        (struct Color){ .argb = 0xFFFFFFFF });

    TEST_ASSERT_NOT_NULL(&l);
}

void check_label_data_update() {
    struct Label l;
    raster_api_create_label(&l, 
        (union LabelData){ .int_val = 42 }, 
        LABEL_DATA_INT,
        (struct Coords){ 1, 1 }, 
        FONT_KONEXY, 
        15, 
        FONT_ALIGN_CENTER,
        (struct Color){ .argb = 0xFFFFFFFF });

    struct Box box = { true, 0x1, { 1, 1, 1, 1 }, { .argb = 0xFF000000 }, &l };
    
    raster_api_set_label_data(&box, (union LabelData){ .int_val = 100 }, LABEL_DATA_INT);
    
    TEST_ASSERT_EQUAL_INT32(100, box.label->data.int_val);
}

int main() {
    UNITY_BEGIN();

    RUN_TEST(check_get_box_found);
    RUN_TEST(check_get_box_not_found);
    RUN_TEST(check_label_building);
    RUN_TEST(check_label_data_update);

    UNITY_END();
}
