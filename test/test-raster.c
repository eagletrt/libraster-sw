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
    struct RasterBox boxes[] = {
        { true, 0x1, { 1, 1, 1, 1 }, { .argb = 0xFF000000 }, NULL },
        { true, 0x2, { 1, 1, 1, 1 }, { .argb = 0xFF000000 }, NULL }
    };

    struct RasterBox *b = raster_api_get_box(boxes, 2, 0x1);
    TEST_ASSERT_NOT_NULL(b);
}

void check_get_box_not_found() {
    struct RasterBox boxes[] = {
        { true, 0x1, { 1, 1, 1, 1 }, { .argb = 0xFF000000 }, NULL },
        { true, 0x2, { 1, 1, 1, 1 }, { .argb = 0xFF000000 }, NULL }
    };

    struct RasterBox *b = raster_api_get_box(boxes, 2, 0x3);
    TEST_ASSERT_NULL(b);
}

void check_label_building() {
    struct RasterLabel l;
    raster_api_create_label(&l,
                            (union RasterLabelData){ .text = "Hello, World" },
                            LABEL_DATA_STRING,
                            (struct RasterCoords){ 1, 1 },
                            0,
                            15,
                            FONT_ALIGN_CENTER,
                            (struct Color){ .argb = 0xFFFFFFFF });

    TEST_ASSERT_NOT_NULL(&l);
}

void check_label_data_update() {
    struct RasterLabel l;
    raster_api_create_label(&l,
                            (union RasterLabelData){ .int_val = 42 },
                            LABEL_DATA_INT,
                            (struct RasterCoords){ 1, 1 },
                            0,
                            15,
                            FONT_ALIGN_CENTER,
                            (struct Color){ .argb = 0xFFFFFFFF });

    struct RasterBox box = { true, 0x1, { 1, 1, 1, 1 }, { .argb = 0xFF000000 }, &l };

    raster_api_set_label_data(&box, (union RasterLabelData){ .int_val = 100 }, LABEL_DATA_INT);

    TEST_ASSERT_EQUAL_INT32(100, box.label->data.int_val);
}

void check_label_float_1_decimal() {
    struct RasterLabel l;
    raster_api_create_label(&l,
                            (union RasterLabelData){ .float_val = 3.14159f },
                            LABEL_DATA_FLOAT_1,
                            (struct RasterCoords){ 10, 20 },
                            0,
                            20,
                            FONT_ALIGN_LEFT,
                            (struct Color){ .argb = 0xFF00FF00 });

    TEST_ASSERT_EQUAL_FLOAT(3.14159f, l.data.float_val);
    TEST_ASSERT_EQUAL(LABEL_DATA_FLOAT_1, l.type);
}

void check_label_float_2_decimals() {
    struct RasterLabel l;
    raster_api_create_label(&l,
                            (union RasterLabelData){ .float_val = 23.456f },
                            LABEL_DATA_FLOAT_2,
                            (struct RasterCoords){ 10, 20 },
                            0,
                            25,
                            FONT_ALIGN_RIGHT,
                            (struct Color){ .argb = 0xFFFF0000 });

    TEST_ASSERT_EQUAL_FLOAT(23.456f, l.data.float_val);
    TEST_ASSERT_EQUAL(LABEL_DATA_FLOAT_2, l.type);
}

void check_label_float_3_decimals() {
    struct RasterLabel l;
    raster_api_create_label(&l,
                            (union RasterLabelData){ .float_val = 98.765f },
                            LABEL_DATA_FLOAT_3,
                            (struct RasterCoords){ 30, 40 },
                            0,
                            30,
                            FONT_ALIGN_CENTER,
                            (struct Color){ .argb = 0xFF0000FF });

    TEST_ASSERT_EQUAL_FLOAT(98.765f, l.data.float_val);
    TEST_ASSERT_EQUAL(LABEL_DATA_FLOAT_3, l.type);
}

void check_label_string_type() {
    struct RasterLabel l;
    const char *test_str = "Speed";
    raster_api_create_label(&l,
                            (union RasterLabelData){ .text = (char *)test_str },
                            LABEL_DATA_STRING,
                            (struct RasterCoords){ 5, 5 },
                            0,
                            40,
                            FONT_ALIGN_CENTER,
                            (struct Color){ .argb = 0xFFFFFFFF });

    TEST_ASSERT_EQUAL_STRING(test_str, l.data.text);
    TEST_ASSERT_EQUAL(LABEL_DATA_STRING, l.type);
}

void check_get_box_with_multiple_boxes() {
    struct RasterBox boxes[] = {
        { true, 0x1, { 10, 10, 100, 100 }, { .argb = 0xFF000000 }, NULL },
        { true, 0x2, { 120, 10, 100, 100 }, { .argb = 0xFF111111 }, NULL },
        { true, 0x3, { 10, 120, 100, 100 }, { .argb = 0xFF222222 }, NULL },
        { true, 0x4, { 120, 120, 100, 100 }, { .argb = 0xFF333333 }, NULL }
    };

    struct RasterBox *b2 = raster_api_get_box(boxes, 4, 0x2);
    TEST_ASSERT_NOT_NULL(b2);
    TEST_ASSERT_EQUAL_UINT16(0x2, b2->id);
    TEST_ASSERT_EQUAL_UINT16(120, b2->rect.x);

    struct RasterBox *b4 = raster_api_get_box(boxes, 4, 0x4);
    TEST_ASSERT_NOT_NULL(b4);
    TEST_ASSERT_EQUAL_UINT16(0x4, b4->id);
}

void check_get_box_first_element() {
    struct RasterBox boxes[] = {
        { true, 0x10, { 0, 0, 50, 50 }, { .argb = 0xFF000000 }, NULL },
        { true, 0x20, { 50, 0, 50, 50 }, { .argb = 0xFF000000 }, NULL }
    };

    struct RasterBox *b = raster_api_get_box(boxes, 2, 0x10);
    TEST_ASSERT_NOT_NULL(b);
    TEST_ASSERT_EQUAL_UINT16(0x10, b->id);
}

void check_get_box_last_element() {
    struct RasterBox boxes[] = {
        { true, 0x10, { 0, 0, 50, 50 }, { .argb = 0xFF000000 }, NULL },
        { true, 0x20, { 50, 0, 50, 50 }, { .argb = 0xFF000000 }, NULL },
        { true, 0x30, { 100, 0, 50, 50 }, { .argb = 0xFF000000 }, NULL }
    };

    struct RasterBox *b = raster_api_get_box(boxes, 3, 0x30);
    TEST_ASSERT_NOT_NULL(b);
    TEST_ASSERT_EQUAL_UINT16(0x30, b->id);
}

void check_set_label_data_float() {
    struct RasterLabel l;
    raster_api_create_label(&l,
                            (union RasterLabelData){ .float_val = 1.0f },
                            LABEL_DATA_FLOAT_2,
                            (struct RasterCoords){ 0, 0 },
                            0,
                            10,
                            FONT_ALIGN_CENTER,
                            (struct Color){ .argb = 0xFFFFFFFF });

    struct RasterBox box = { true, 0x1, { 0, 0, 100, 100 }, { .argb = 0xFF000000 }, &l };

    raster_api_set_label_data(&box, (union RasterLabelData){ .float_val = 99.99f }, LABEL_DATA_FLOAT_2);

    TEST_ASSERT_EQUAL_FLOAT(99.99f, box.label->data.float_val);
    TEST_ASSERT_EQUAL(LABEL_DATA_FLOAT_2, box.label->type);
}

void check_set_label_data_string() {
    struct RasterLabel l;
    raster_api_create_label(&l,
                            (union RasterLabelData){ .text = "Old" },
                            LABEL_DATA_STRING,
                            (struct RasterCoords){ 0, 0 },
                            0,
                            10,
                            FONT_ALIGN_CENTER,
                            (struct Color){ .argb = 0xFFFFFFFF });

    struct RasterBox box = { true, 0x1, { 0, 0, 100, 100 }, { .argb = 0xFF000000 }, &l };

    const char *new_text = "New";
    raster_api_set_label_data(&box, (union RasterLabelData){ .text = (char *)new_text }, LABEL_DATA_STRING);

    TEST_ASSERT_EQUAL_STRING(new_text, box.label->data.text);
    TEST_ASSERT_EQUAL(LABEL_DATA_STRING, box.label->type);
}

void check_set_label_data_null_box() {
    // This should not crash
    raster_api_set_label_data(NULL, (union RasterLabelData){ .int_val = 100 }, LABEL_DATA_INT);
    TEST_ASSERT_TRUE(true); // If we reach here, it didn't crash
}

void check_box_with_label_properties() {
    struct RasterLabel l;
    raster_api_create_label(&l,
                            (union RasterLabelData){ .int_val = 42 },
                            LABEL_DATA_INT,
                            (struct RasterCoords){ 25, 30 },
                            0,
                            50,
                            FONT_ALIGN_LEFT,
                            (struct Color){ .argb = 0xFFABCDEF });

    TEST_ASSERT_EQUAL_UINT16(25, l.pos.x);
    TEST_ASSERT_EQUAL_UINT16(30, l.pos.y);
    TEST_ASSERT_EQUAL_UINT16(50, l.size);
    TEST_ASSERT_EQUAL(FONT_ALIGN_LEFT, l.align);
    TEST_ASSERT_EQUAL_UINT32(0xFFABCDEF, l.color.argb);
}
