/**
 * @file test-raster.c
 * @brief Test suite for raster-api.c
 *
 * @author Alessandro Bridi [ale.bridi15@gmail.com]
 * @date 2025-03-23
 */

#include "raster-api.h"
#include "test-font.h"
#include "unity.h"
#include "fff.h"
#include <stddef.h>

DEFINE_FFF_GLOBALS;
FAKE_VALUE_FUNC(enum RasterReturnCode, fake_draw, uint16_t, uint16_t, uint16_t, uint16_t, struct Color);

struct RasterHandler handler;
struct RasterBox box[2] = {
    { true, 0x1, { 1, 1, 1, 1 }, { .argb = 0xFF000000 }, NULL },
    { true, 0x2, { 1, 1, 1, 1 }, { .argb = 0xFF000000 }, NULL },
};

void setUp(void) {
    RESET_FAKE(fake_draw);
    FFF_RESET_HISTORY();
    raster_api_init(&handler, box, 2, fake_draw, NULL);
}

/*!
 * \defgroup raster_api_init Tests for raster_api_init()
 * \{
 */

void test_raster_api_init_successfull(void) {
    enum RasterReturnCode rc = raster_api_init(&handler, box, 2, fake_draw, NULL);
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_OK, rc, "Expected RASTER_RC_OK");
    TEST_ASSERT_EQUAL_PTR_MESSAGE(box, handler.interface, "Expected handler.interface to point to box");
    TEST_ASSERT_EQUAL_MESSAGE(2, handler.size, "Expected handler.size to be 2");
    TEST_ASSERT_EQUAL_PTR_MESSAGE(fake_draw, handler.draw, "Expected handler.draw to point to fake_draw");
    TEST_ASSERT_NULL_MESSAGE(handler.clear, "Expected handler.clear to be NULL");
}

void test_raster_api_init_null_handler(void) {
    enum RasterReturnCode rc = raster_api_init(NULL, box, 2, fake_draw, NULL);
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, rc);
}

void test_raster_api_init_null_interface(void) {
    enum RasterReturnCode rc = raster_api_init(&handler, NULL, 2, fake_draw, NULL);
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, rc);
}

void test_raster_api_init_zero_size(void) {
    enum RasterReturnCode rc = raster_api_init(&handler, box, 0, fake_draw, NULL);
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, rc);
}

void test_raster_api_init_null_draw(void) {
    enum RasterReturnCode rc = raster_api_init(&handler, box, 2, NULL, NULL);
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, rc);
}

/*! \} */

/*!
 * \defgroup raster_api_set_interface Tests for raster_api_set_interface()
 * \{
 */

void test_raster_api_set_interface_successfull(void) {
    struct RasterBox new_box[1] = {
        { true, 0x3, { 1, 1, 1, 1 }, { .argb = 0xFF000000 }, NULL },
    };
    enum RasterReturnCode rc = raster_api_set_interface(&handler, new_box, 1);
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_OK, rc, "Expected RASTER_RC_OK");
    TEST_ASSERT_EQUAL_PTR_MESSAGE(new_box, handler.interface, "Expected handler.interface to point to new_box");
    TEST_ASSERT_EQUAL_MESSAGE(1, handler.size, "Expected handler.size to be 1");
}

void test_raster_api_set_interface_null_handler(void) {
    struct RasterBox new_box[1] = {
        { true, 0x3, { 1, 1, 1, 1 }, { .argb = 0xFF000000 }, NULL },
    };
    enum RasterReturnCode rc = raster_api_set_interface(NULL, new_box, 1);
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, rc);
}

void test_raster_api_set_interface_null_interface(void) {
    enum RasterReturnCode rc = raster_api_set_interface(&handler, NULL, 1);
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, rc);
}

/*! \} */

/*!
 * \defgroup raster_api_render Tests for raster_api_render()
 * \{
 */

void test_raster_api_render_successfull(void) {
    fake_draw_fake.return_val = RASTER_RC_OK;
    enum RasterReturnCode rc = raster_api_render(&handler);
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_OK, rc, "Expected RASTER_RC_OK");
    TEST_ASSERT_EQUAL_MESSAGE(2, fake_draw_fake.call_count, "Expected draw to be called twice");
}

void test_raster_api_render_null_handler(void) {
    enum RasterReturnCode rc = raster_api_render(NULL);
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, rc);
}

void test_raster_api_render_null_draw(void) {
    handler.draw = NULL;
    enum RasterReturnCode rc = raster_api_render(&handler);
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, rc);
}

void test_raster_api_render_null_interface(void) {
    handler.interface = NULL;
    enum RasterReturnCode rc = raster_api_render(&handler);
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, rc);
}

/*! \} */

/*!
 * \defgroup raster_api_get_box Tests for raster_api_get_box()
 * \{
 */

void test_raster_api_get_box_found(void) {
    struct RasterBox *result = raster_api_get_box(box, 2, 0x2);
    TEST_ASSERT_NOT_NULL_MESSAGE(result, "Expected to find box with id 0x2");
    TEST_ASSERT_EQUAL_MESSAGE(0x2, result->id, "Expected found box to have id 0x2");
}

void test_raster_api_get_box_not_found(void) {
    struct RasterBox *result = raster_api_get_box(box, 2, 0x3);
    TEST_ASSERT_NULL(result);
}

void test_raster_api_get_box_null_boxes(void) {
    struct RasterBox *result = raster_api_get_box(NULL, 2, 0x1);
    TEST_ASSERT_NULL(result);
}

/*! \} */

/*!
 * \defgroup raster_api_create_label Tests for raster_api_create_label()
 * \{
 */

void test_raster_api_create_label_successfull(void) {
    struct RasterLabel label;
    enum RasterReturnCode rc = raster_api_create_label(&label, (union RasterLabelData){ .int_val = 42 }, RASTER_LABEL_DATA_INT, (union RasterLabelFormat){ .int_fmt = { .is_unsigned = false } }, (struct RasterCoords){ 0, 0 }, &test_font, 12, FONT_ALIGN_LEFT, (struct Color){ .argb = 0xFFFFFFFF });

    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_OK, rc, "Expected RASTER_RC_OK");
    TEST_ASSERT_EQUAL_MESSAGE(42, label.data.int_val, "Expected label data to be 42");
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_LABEL_DATA_INT, label.type, "Expected label type to be RASTER_LABEL_DATA_INT");
    TEST_ASSERT_TRUE_MESSAGE(label.format.int_fmt.is_unsigned, "Expected label format to be unsigned");
    TEST_ASSERT_EQUAL_MESSAGE(0, label.pos.x, "Expected label position x to be 0");
    TEST_ASSERT_EQUAL_MESSAGE(0, label.pos.y, "Expected label position y to be 0");
    TEST_ASSERT_EQUAL_PTR_MESSAGE(&test_font, label.font, "Expected label font to point to test_font");
    TEST_ASSERT_EQUAL_MESSAGE(12, label.size, "Expected label size to be 12");
    TEST_ASSERT_EQUAL_MESSAGE(FONT_ALIGN_LEFT, label.align, "Expected label alignment to be FONT_ALIGN_LEFT");
    TEST_ASSERT_EQUAL_HEX32_MESSAGE(0xFFFFFFFF, label.color.argb, "Expected label color to be 0xFFFFFFFF");
}

void test_raster_api_create_label_null_label(void) {
    enum RasterReturnCode rc = raster_api_create_label(NULL, (union RasterLabelData){ .int_val = 42 }, RASTER_LABEL_DATA_INT, (union RasterLabelFormat){ .int_fmt = { .is_unsigned = false } }, (struct RasterCoords){ 0, 0 }, NULL, 12, FONT_ALIGN_LEFT, (struct Color){ .argb = 0xFFFFFFFF });
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, rc);
}

void test_raster_api_create_label_null_font(void) {
    struct RasterLabel label;
    enum RasterReturnCode rc = raster_api_create_label(&label, (union RasterLabelData){ .int_val = 42 }, RASTER_LABEL_DATA_INT, (union RasterLabelFormat){ .int_fmt = { .is_unsigned = false } }, (struct RasterCoords){ 0, 0 }, NULL, 12, FONT_ALIGN_LEFT, (struct Color){ .argb = 0xFFFFFFFF });
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, rc);
}

/*! \} */

/*!
 * \defgroup raster_api_set_label_data Tests for raster_api_set_label_data()
 * \{
 */

void test_raster_api_set_label_data_successfull(void) {
    struct RasterLabel label;
    raster_api_create_label(&label, (union RasterLabelData){ .int_val = 42 }, RASTER_LABEL_DATA_INT, (union RasterLabelFormat){ .int_fmt = { .is_unsigned = false } }, (struct RasterCoords){ 0, 0 }, &test_font, 12, FONT_ALIGN_LEFT, (struct Color){ .argb = 0xFFFFFFFF });
    struct RasterBox box = { true, 0x1, { 1, 1, 1, 1 }, { .argb = 0xFF000000 }, &label };

    enum RasterReturnCode rc = raster_api_set_label_data(&box, (union RasterLabelData){ .int_val = 84 });
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_OK, rc, "Expected RASTER_RC_OK");
    TEST_ASSERT_EQUAL_MESSAGE(84, label.data.int_val, "Expected label data to be updated to 84");
    TEST_ASSERT_TRUE_MESSAGE(box.updated, "Expected box to be marked as updated");
}

void test_raster_api_set_label_data_null_box(void) {
    enum RasterReturnCode rc = raster_api_set_label_data(NULL, (union RasterLabelData){ .int_val = 84 });
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, rc);
}

void test_raster_api_set_label_data_null_label(void) {
    struct RasterBox box = { true, 0x1, { 1, 1, 1, 1 }, { .argb = 0xFF000000 }, NULL };
    enum RasterReturnCode rc = raster_api_set_label_data(&box, (union RasterLabelData){ .int_val = 84 });
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, rc);
}

/*! \} */

// TODO: add tests under here

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_raster_api_init_successfull);
    RUN_TEST(test_raster_api_init_null_handler);
    RUN_TEST(test_raster_api_init_null_interface);
    RUN_TEST(test_raster_api_init_zero_size);
    RUN_TEST(test_raster_api_init_null_draw);

    RUN_TEST(test_raster_api_set_interface_successfull);
    RUN_TEST(test_raster_api_set_interface_null_handler);
    RUN_TEST(test_raster_api_set_interface_null_interface);

    RUN_TEST(test_raster_api_render_successfull);
    RUN_TEST(test_raster_api_render_null_handler);
    RUN_TEST(test_raster_api_render_null_draw);
    RUN_TEST(test_raster_api_render_null_interface);

    RUN_TEST(test_raster_api_get_box_found);
    RUN_TEST(test_raster_api_get_box_not_found);
    RUN_TEST(test_raster_api_get_box_null_boxes);

    RUN_TEST(test_raster_api_create_label_successfull);
    RUN_TEST(test_raster_api_create_label_null_label);
    RUN_TEST(test_raster_api_create_label_null_font);

    RUN_TEST(test_raster_api_set_label_data_successfull);
    RUN_TEST(test_raster_api_set_label_data_null_box);
    RUN_TEST(test_raster_api_set_label_data_null_label);

    return UNITY_END();
}
