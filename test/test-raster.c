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
                            (union RasterLabelFormat){ .string_fmt = raster_api_string_format(0) },
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
                            (union RasterLabelFormat){ .int_fmt = raster_api_int_format(false) },
                            (struct RasterCoords){ 1, 1 },
                            0,
                            15,
                            FONT_ALIGN_CENTER,
                            (struct Color){ .argb = 0xFFFFFFFF });

    struct RasterBox box = { true, 0x1, { 1, 1, 1, 1 }, { .argb = 0xFF000000 }, &l };

    raster_api_set_label_data(&box, (union RasterLabelData){ .int_val = 100 });

    TEST_ASSERT_EQUAL_INT32(100, box.label->data.int_val);
}

void check_label_float_1_decimal() {
    struct RasterLabel l;
    raster_api_create_label(&l,
                            (union RasterLabelData){ .float_val = 3.14159f },
                            LABEL_DATA_FLOAT,
                            (union RasterLabelFormat){ .float_fmt = raster_api_float_format(1) },
                            (struct RasterCoords){ 10, 20 },
                            0,
                            20,
                            FONT_ALIGN_LEFT,
                            (struct Color){ .argb = 0xFF00FF00 });

    TEST_ASSERT_EQUAL_FLOAT_MESSAGE(3.14159f, l.data.float_val, "Float value does not match");
    TEST_ASSERT_EQUAL_MESSAGE(LABEL_DATA_FLOAT, l.type, "Label type does not match");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(1, l.format.float_fmt.precision, "Float precision does not match");
}

void check_label_float_2_decimals() {
    struct RasterLabel l;
    raster_api_create_label(&l,
                            (union RasterLabelData){ .float_val = 23.456f },
                            LABEL_DATA_FLOAT,
                            (union RasterLabelFormat){ .float_fmt = raster_api_float_format(2) },
                            (struct RasterCoords){ 10, 20 },
                            0,
                            25,
                            FONT_ALIGN_RIGHT,
                            (struct Color){ .argb = 0xFFFF0000 });

    TEST_ASSERT_EQUAL_FLOAT_MESSAGE(23.456f, l.data.float_val, "Float value does not match");
    TEST_ASSERT_EQUAL_MESSAGE(LABEL_DATA_FLOAT, l.type, "Label type does not match");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(2, l.format.float_fmt.precision, "Float precision does not match");
}

void check_label_float_3_decimals() {
    struct RasterLabel l;
    raster_api_create_label(&l,
                            (union RasterLabelData){ .float_val = 98.765f },
                            LABEL_DATA_FLOAT,
                            (union RasterLabelFormat){ .float_fmt = raster_api_float_format(3) },
                            (struct RasterCoords){ 30, 40 },
                            0,
                            30,
                            FONT_ALIGN_CENTER,
                            (struct Color){ .argb = 0xFF0000FF });

    TEST_ASSERT_EQUAL_FLOAT_MESSAGE(98.765f, l.data.float_val, "Float value does not match");
    TEST_ASSERT_EQUAL_MESSAGE(LABEL_DATA_FLOAT, l.type, "Label type does not match");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(3, l.format.float_fmt.precision, "Float precision does not match");
}

void check_label_string_type() {
    struct RasterLabel l;
    const char *test_str = "Speed";
    raster_api_create_label(&l,
                            (union RasterLabelData){ .text = (char *)test_str },
                            LABEL_DATA_STRING,
                            (union RasterLabelFormat){ .string_fmt = raster_api_string_format(0) },
                            (struct RasterCoords){ 5, 5 },
                            0,
                            40,
                            FONT_ALIGN_CENTER,
                            (struct Color){ .argb = 0xFFFFFFFF });

    TEST_ASSERT_EQUAL_STRING_MESSAGE(test_str, l.data.text, "String value does not match");
    TEST_ASSERT_EQUAL_MESSAGE(LABEL_DATA_STRING, l.type, "Label type does not match");
}

void check_get_box_with_multiple_boxes() {
    struct RasterBox boxes[] = {
        { true, 0x1, { 10, 10, 100, 100 }, { .argb = 0xFF000000 }, NULL },
        { true, 0x2, { 120, 10, 100, 100 }, { .argb = 0xFF111111 }, NULL },
        { true, 0x3, { 10, 120, 100, 100 }, { .argb = 0xFF222222 }, NULL },
        { true, 0x4, { 120, 120, 100, 100 }, { .argb = 0xFF333333 }, NULL }
    };

    struct RasterBox *b2 = raster_api_get_box(boxes, 4, 0x2);
    TEST_ASSERT_NOT_NULL_MESSAGE(b2, "Box with ID 0x2 should be found");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(0x2, b2->id, "Box ID does not match");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(120, b2->rect.x, "Box X coordinate does not match");

    struct RasterBox *b4 = raster_api_get_box(boxes, 4, 0x4);
    TEST_ASSERT_NOT_NULL_MESSAGE(b4, "Box with ID 0x4 should be found");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(0x4, b4->id, "Box ID does not match");
}

void check_get_box_first_element() {
    struct RasterBox boxes[] = {
        { true, 0x10, { 0, 0, 50, 50 }, { .argb = 0xFF000000 }, NULL },
        { true, 0x20, { 50, 0, 50, 50 }, { .argb = 0xFF000000 }, NULL }
    };

    struct RasterBox *b = raster_api_get_box(boxes, 2, 0x10);
    TEST_ASSERT_NOT_NULL_MESSAGE(b, "Box with ID 0x10 should be found");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(0x10, b->id, "Box ID does not match");
}

void check_get_box_last_element() {
    struct RasterBox boxes[] = {
        { true, 0x10, { 0, 0, 50, 50 }, { .argb = 0xFF000000 }, NULL },
        { true, 0x20, { 50, 0, 50, 50 }, { .argb = 0xFF000000 }, NULL },
        { true, 0x30, { 100, 0, 50, 50 }, { .argb = 0xFF000000 }, NULL }
    };

    struct RasterBox *b = raster_api_get_box(boxes, 3, 0x30);
    TEST_ASSERT_NOT_NULL_MESSAGE(b, "Box with ID 0x30 should be found");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(0x30, b->id, "Box ID does not match");
}

void check_set_label_data_float() {
    struct RasterLabel l;
    raster_api_create_label(&l,
                            (union RasterLabelData){ .float_val = 1.0f },
                            LABEL_DATA_FLOAT,
                            (union RasterLabelFormat){ .float_fmt = raster_api_float_format(2) },
                            (struct RasterCoords){ 0, 0 },
                            0,
                            10,
                            FONT_ALIGN_CENTER,
                            (struct Color){ .argb = 0xFFFFFFFF });

    struct RasterBox box = { true, 0x1, { 0, 0, 100, 100 }, { .argb = 0xFF000000 }, &l };

    raster_api_set_label_data(&box, (union RasterLabelData){ .float_val = 99.99f });

    TEST_ASSERT_EQUAL_FLOAT_MESSAGE(99.99f, box.label->data.float_val, "Float value was not updated correctly");
}

void check_set_label_data_string() {
    struct RasterLabel l;
    raster_api_create_label(&l,
                            (union RasterLabelData){ .text = "Old" },
                            LABEL_DATA_STRING,
                            (union RasterLabelFormat){ .string_fmt = raster_api_string_format(0) },
                            (struct RasterCoords){ 0, 0 },
                            0,
                            10,
                            FONT_ALIGN_CENTER,
                            (struct Color){ .argb = 0xFFFFFFFF });

    struct RasterBox box = { true, 0x1, { 0, 0, 100, 100 }, { .argb = 0xFF000000 }, &l };

    const char *new_text = "New";
    raster_api_set_label_data(&box, (union RasterLabelData){ .text = (char *)new_text });

    TEST_ASSERT_EQUAL_STRING_MESSAGE(new_text, box.label->data.text, "String value was not updated correctly");
}

void check_set_label_data_null_box() {
    // This should not crash
    raster_api_set_label_data(NULL, (union RasterLabelData){ .int_val = 100 });
    TEST_ASSERT_TRUE(true); // If we reach here, it didn't crash
}

void check_box_with_label_properties() {
    struct RasterLabel l;
    raster_api_create_label(&l,
                            (union RasterLabelData){ .int_val = 42 },
                            LABEL_DATA_INT,
                            (union RasterLabelFormat){ .int_fmt = raster_api_int_format(false) },
                            (struct RasterCoords){ 25, 30 },
                            0,
                            50,
                            FONT_ALIGN_LEFT,
                            (struct Color){ .argb = 0xFFABCDEF });

    TEST_ASSERT_EQUAL_UINT16_MESSAGE(25, l.pos.x, "Label X position does not match");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(30, l.pos.y, "Label Y position does not match");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(50, l.size, "Label size does not match");
    TEST_ASSERT_EQUAL_MESSAGE(FONT_ALIGN_LEFT, l.align, "Label alignment does not match");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0xFFABCDEF, l.color.argb, "Label color does not match");
}
