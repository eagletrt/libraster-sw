/**
 * @file test-raster.c
 * @brief Test suite for raster-api.c
 *
 * @author Alessandro Bridi [ale.bridi15@gmail.com]
 * @date 2025-03-23
 */

#include "fff.h"
#include "raster-api.h"
#include "test-font.h"
#include "eagletrt.h"
#include "unity.h"

#include <stddef.h>

DEFINE_FFF_GLOBALS;
FAKE_VALUE_FUNC(enum RasterReturnCode, fake_draw, uint16_t, uint16_t, uint16_t, uint16_t, struct Color);
FAKE_VALUE_FUNC(enum RasterReturnCode, fake_clear);

EAGLETRT_STATIC struct RasterHandler handler;
EAGLETRT_STATIC struct RasterBox boxes[2] = {
    { true, 0x1, { 1, 1, 1, 1 }, { .argb = 0xFF000000 }, NULL },
    { true, 0x2, { 1, 1, 1, 1 }, { .argb = 0xFF000000 }, NULL },
};

void setUp(void) {
    RESET_FAKE(fake_draw);
    RESET_FAKE(fake_clear);
    FFF_RESET_HISTORY();
    fake_draw_fake.return_val = RASTER_RC_OK;
    fake_clear_fake.return_val = RASTER_RC_OK;
    boxes[0].updated = true;
    boxes[1].updated = true;
    boxes[0].label = NULL;
    boxes[1].label = NULL;
    raster_api_init(&handler, boxes, 2, fake_draw, NULL);
}

/*!
 * \defgroup raster_api_init Tests for raster_api_init()
 * \{
 */

void test_raster_api_init_successful_partial(void) {
    enum RasterReturnCode rc = raster_api_init(&handler, boxes, 2, fake_draw, NULL);
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_OK, rc, "Expected RASTER_RC_OK");
    TEST_ASSERT_EQUAL_PTR_MESSAGE(boxes, handler.interface, "Expected handler.interface to point to boxes");
    TEST_ASSERT_EQUAL_MESSAGE(2, handler.size, "Expected handler.size to be 2");
    TEST_ASSERT_EQUAL_PTR_MESSAGE(fake_draw, handler.draw, "Expected handler.draw to be fake_draw");
    TEST_ASSERT_NULL_MESSAGE(handler.clear, "Expected handler.clear to be NULL (partial mode)");
}

void test_raster_api_init_successful_full_redraw(void) {
    enum RasterReturnCode rc = raster_api_init(&handler, boxes, 2, fake_draw, fake_clear);
    TEST_ASSERT_EQUAL(RASTER_RC_OK, rc);
    TEST_ASSERT_EQUAL_PTR_MESSAGE(fake_clear, handler.clear, "Expected handler.clear to be fake_clear (full-redraw mode)");
}

void test_raster_api_init_null_handler(void) {
    enum RasterReturnCode rc = raster_api_init(NULL, boxes, 2, fake_draw, NULL);
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, rc);
}

void test_raster_api_init_null_interface(void) {
    enum RasterReturnCode rc = raster_api_init(&handler, NULL, 2, fake_draw, NULL);
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, rc);
}

void test_raster_api_init_zero_size(void) {
    enum RasterReturnCode rc = raster_api_init(&handler, boxes, 0, fake_draw, NULL);
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, rc);
}

void test_raster_api_init_null_draw(void) {
    enum RasterReturnCode rc = raster_api_init(&handler, boxes, 2, NULL, NULL);
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, rc);
}

/*! \} */

/*!
 * \defgroup raster_api_set_interface Tests for raster_api_set_interface()
 * \{
 */

void test_raster_api_set_interface_successful(void) {
    struct RasterBox new_box[1] = {
        { true, 0x3, { 1, 1, 1, 1 }, { .argb = 0xFF000000 }, NULL },
    };
    enum RasterReturnCode rc = raster_api_set_interface(&handler, new_box, 1);
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_OK, rc, "Expected RASTER_RC_OK");
    TEST_ASSERT_EQUAL_PTR_MESSAGE(new_box, handler.interface, "Expected handler.interface to point to new_box");
    TEST_ASSERT_EQUAL_MESSAGE(1, handler.size, "Expected handler.size to be 1");
}

void test_raster_api_set_interface_null_handler(void) {
    struct RasterBox new_box[1] = { 0 };
    enum RasterReturnCode rc = raster_api_set_interface(NULL, new_box, 1);
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, rc);
}

void test_raster_api_set_interface_null_interface(void) {
    enum RasterReturnCode rc = raster_api_set_interface(&handler, NULL, 1);
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, rc);
}

void test_raster_api_set_interface_zero_size(void) {
    struct RasterBox new_box[1] = { 0 };
    enum RasterReturnCode rc = raster_api_set_interface(&handler, new_box, 0);
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, rc);
}

/*! \} */

/*!
 * \defgroup raster_api_render Tests for raster_api_render()
 * \{
 */

void test_raster_api_render_partial_draws_updated_only(void) {
    boxes[0].updated = true;
    boxes[1].updated = false;
    enum RasterReturnCode rc = raster_api_render(&handler);
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_OK, rc, "Expected RASTER_RC_OK");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(1, fake_draw_fake.call_count, "Only the updated box should be redrawn");
}

void test_raster_api_render_partial_clears_updated_flag(void) {
    boxes[0].updated = true;
    boxes[1].updated = true;
    raster_api_render(&handler);
    TEST_ASSERT_FALSE_MESSAGE(boxes[0].updated, "updated should be cleared after redraw");
    TEST_ASSERT_FALSE_MESSAGE(boxes[1].updated, "updated should be cleared after redraw");
}

void test_raster_api_render_partial_skips_when_no_updates(void) {
    boxes[0].updated = false;
    boxes[1].updated = false;
    raster_api_render(&handler);
    TEST_ASSERT_EQUAL_UINT(0, fake_draw_fake.call_count);
}

void test_raster_api_render_full_redraw_calls_clear_and_all_boxes(void) {
    raster_api_init(&handler, boxes, 2, fake_draw, fake_clear);
    boxes[0].updated = false;
    boxes[1].updated = false;
    enum RasterReturnCode rc = raster_api_render(&handler);
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_OK, rc, "Expected RASTER_RC_OK");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(1, fake_clear_fake.call_count, "Clear should run once");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(2, fake_draw_fake.call_count, "Both boxes should be drawn even when not flagged");
}

void test_raster_api_render_full_redraw_keeps_updated_flag(void) {
    raster_api_init(&handler, boxes, 2, fake_draw, fake_clear);
    boxes[0].updated = true;
    boxes[1].updated = true;
    raster_api_render(&handler);
    TEST_ASSERT_TRUE_MESSAGE(boxes[0].updated, "Full-redraw mode should not touch the updated flag");
    TEST_ASSERT_TRUE_MESSAGE(boxes[1].updated, "Full-redraw mode should not touch the updated flag");
}

void test_raster_api_render_propagates_clear_error(void) {
    raster_api_init(&handler, boxes, 2, fake_draw, fake_clear);
    fake_clear_fake.return_val = RASTER_RC_ERROR;
    enum RasterReturnCode rc = raster_api_render(&handler);
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_ERROR, rc, "Expected RASTER_RC_ERROR when clear fails");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(0, fake_draw_fake.call_count, "Draw should not run when clear fails");
}

void test_raster_api_render_propagates_draw_error(void) {
    fake_draw_fake.return_val = RASTER_RC_ERROR;
    enum RasterReturnCode rc = raster_api_render(&handler);
    TEST_ASSERT_EQUAL(RASTER_RC_ERROR, rc);
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

void test_raster_api_render_passes_box_geometry(void) {
    boxes[0] = (struct RasterBox){ .updated = true, .id = 0x1, .rect = { 5, 6, 7, 8 }, .color = { .argb = 0x12345678 }, .label = NULL };
    boxes[1].updated = false;
    raster_api_render(&handler);
    TEST_ASSERT_EQUAL_UINT_MESSAGE(1, fake_draw_fake.call_count, "Only the updated box should be redrawn");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(5, fake_draw_fake.arg0_history[0], "Expected draw to receive box.x as arg0");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(6, fake_draw_fake.arg1_history[0], "Expected draw to receive box.y as arg1");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(7, fake_draw_fake.arg2_history[0], "Expected draw to receive box.width as arg2");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(8, fake_draw_fake.arg3_history[0], "Expected draw to receive box.height as arg3");
    TEST_ASSERT_EQUAL_HEX32_MESSAGE(0x12345678, fake_draw_fake.arg4_history[0].argb, "Expected draw to receive box.color as arg4");
}

/*! \} */

/*!
 * \defgroup raster_api_get_box Tests for raster_api_get_box()
 * \{
 */

void test_raster_api_get_box_first(void) {
    struct RasterBox *result = raster_api_get_box(boxes, 2, 0x1);
    TEST_ASSERT_NOT_NULL_MESSAGE(result, "Expected to find box with id 0x1");
    TEST_ASSERT_EQUAL_PTR_MESSAGE(&boxes[0], result, "Expected to find the first box");
}

void test_raster_api_get_box_last(void) {
    struct RasterBox *result = raster_api_get_box(boxes, 2, 0x2);
    TEST_ASSERT_NOT_NULL_MESSAGE(result, "Expected to find box with id 0x2");
    TEST_ASSERT_EQUAL_PTR_MESSAGE(&boxes[1], result, "Expected to find the second box");
}

void test_raster_api_get_box_not_found(void) {
    TEST_ASSERT_NULL(raster_api_get_box(boxes, 2, 0xBEEF));
}

void test_raster_api_get_box_null_boxes(void) {
    TEST_ASSERT_NULL(raster_api_get_box(NULL, 2, 0x1));
}

void test_raster_api_get_box_zero_size(void) {
    TEST_ASSERT_NULL(raster_api_get_box(boxes, 0, 0x1));
}

/*! \} */

/*!
 * \defgroup raster_api_create_label Tests for raster_api_create_label()
 * \{
 */

void test_raster_api_create_label_successful(void) {
    struct RasterLabel label;
    enum RasterReturnCode rc = raster_api_create_label(
        &label,
        (union RasterLabelData){ .integer = { .value = 42, .is_unsigned = true } },
        RASTER_LABEL_DATA_INT,
        (struct RasterCoords){ 3, 4 },
        &test_font,
        12,
        FONT_ALIGN_LEFT,
        (struct Color){ .argb = 0xFFFFFFFF });
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_OK, rc, "Expected RASTER_RC_OK");
    TEST_ASSERT_EQUAL_INT32_MESSAGE(42, label.data.integer.value, "Expected label.data.integer.value to be set to 42");
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_LABEL_DATA_INT, label.type, "Expected label.type to be RASTER_LABEL_DATA_INT");
    TEST_ASSERT_TRUE_MESSAGE(label.data.integer.is_unsigned, "Expected label.data.integer.is_unsigned to be true");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(3, label.pos.x, "Expected label.pos.x to be set to 3");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(4, label.pos.y, "Expected label.pos.y to be set to 4");
    TEST_ASSERT_EQUAL_PTR_MESSAGE(&test_font, label.font, "Expected label.font to point to test_font");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(12, label.size, "Expected label.size to be set to 12");
    TEST_ASSERT_EQUAL_MESSAGE(FONT_ALIGN_LEFT, label.align, "Expected label.align to be FONT_ALIGN_LEFT");
    TEST_ASSERT_EQUAL_HEX32_MESSAGE(0xFFFFFFFF, label.color.argb, "Expected label.color to be set to 0xFFFFFFFF");
}

void test_raster_api_create_label_signed_int_format(void) {
    struct RasterLabel label;
    raster_api_create_label(&label,
                            (union RasterLabelData){ .integer = { .value = -1, .is_unsigned = false } },
                            RASTER_LABEL_DATA_INT,
                            (struct RasterCoords){ 0, 0 },
                            &test_font,
                            10,
                            FONT_ALIGN_LEFT,
                            (struct Color){ .argb = 0xFFFFFFFF });
    TEST_ASSERT_FALSE_MESSAGE(label.data.integer.is_unsigned, "Expected label.data.integer.is_unsigned to be false for signed int format");
    TEST_ASSERT_EQUAL_INT32_MESSAGE(-1, label.data.integer.value, "Expected label.data.integer.value to be set to -1 for signed int format");
}

void test_raster_api_create_label_null_label(void) {
    enum RasterReturnCode rc = raster_api_create_label(NULL,
                                                       (union RasterLabelData){ .integer = { .value = 0, .is_unsigned = false } },
                                                       RASTER_LABEL_DATA_INT,
                                                       (struct RasterCoords){ 0, 0 },
                                                       &test_font,
                                                       12,
                                                       FONT_ALIGN_LEFT,
                                                       (struct Color){ .argb = 0xFFFFFFFF });
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, rc);
}

void test_raster_api_create_label_null_font(void) {
    struct RasterLabel label;
    enum RasterReturnCode rc = raster_api_create_label(&label,
                                                       (union RasterLabelData){ .integer = { .value = 0, .is_unsigned = false } },
                                                       RASTER_LABEL_DATA_INT,
                                                       (struct RasterCoords){ 0, 0 },
                                                       NULL,
                                                       12,
                                                       FONT_ALIGN_LEFT,
                                                       (struct Color){ .argb = 0xFFFFFFFF });
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, rc);
}

/*! \} */

/*!
 * \defgroup raster_api_set_label_data Tests for raster_api_set_label_data()
 * \{
 */

EAGLETRT_STATIC struct RasterLabel make_int_label(void) {
    struct RasterLabel label;
    raster_api_create_label(&label,
                            (union RasterLabelData){ .integer = { .value = 0, .is_unsigned = false } },
                            RASTER_LABEL_DATA_INT,
                            (struct RasterCoords){ 0, 0 },
                            &test_font,
                            12,
                            FONT_ALIGN_LEFT,
                            (struct Color){ .argb = 0xFFFFFFFF });
    return label;
}

void test_raster_api_set_label_string_updates_value(void) {
    struct RasterLabel label = make_int_label();
    label.type = RASTER_LABEL_DATA_STRING;
    struct RasterBox box = { false, 0x1, { 0, 0, 1, 1 }, { 0 }, &label };
    char *new_text = "hi";
    enum RasterReturnCode rc = raster_api_set_label_string(&box, new_text);
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_OK, rc, "Expected RASTER_RC_OK");
    TEST_ASSERT_EQUAL_PTR_MESSAGE(new_text, label.data.string.value, "Expected label.data.string.value to be updated to new_text");
    TEST_ASSERT_TRUE_MESSAGE(box.updated, "Setting label text must mark the box as updated");
}

void test_raster_api_set_label_string_null_text(void) {
    struct RasterLabel label = make_int_label();
    label.type = RASTER_LABEL_DATA_STRING;
    struct RasterBox box = { false, 0x1, { 0, 0, 1, 1 }, { 0 }, &label };
    enum RasterReturnCode rc = raster_api_set_label_string(&box, NULL);
    TEST_ASSERT_NULL_MESSAGE(label.data.string.value, "Expected label.data.string.value to be NULL when setting NULL text");
    TEST_ASSERT_EQUAL_MESSAGE(0, label.data.string.length, "Expected label.data.string.length to be 0 when setting NULL text");
    TEST_ASSERT_TRUE_MESSAGE(box.updated, "Setting label text must mark the box as updated even when setting NULL");
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_OK, rc, "Expected RASTER_RC_OK even when setting NULL text");
}

void test_raster_api_set_label_string_null_box(void) {
    enum RasterReturnCode rc = raster_api_set_label_string(NULL, "test");
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, rc);
}

void test_raster_api_set_label_int_updates_value(void) {
    struct RasterLabel label = make_int_label();
    struct RasterBox box = { false, 0x1, { 0, 0, 1, 1 }, { 0 }, &label };
    enum RasterReturnCode rc = raster_api_set_label_int(&box, -77);
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_OK, rc, "Expected RASTER_RC_OK");
    TEST_ASSERT_EQUAL_INT32_MESSAGE(-77, label.data.integer.value, "Expected label.data.integer.value to be updated to -77");
    TEST_ASSERT_TRUE_MESSAGE(box.updated, "Setting label int data must mark the box as updated");
}

void test_raster_api_set_label_float_updates_value(void) {
    struct RasterLabel label = make_int_label();
    label.type = RASTER_LABEL_DATA_FLOAT;
    struct RasterBox box = { false, 0x1, { 0, 0, 1, 1 }, { 0 }, &label };
    enum RasterReturnCode rc = raster_api_set_label_float(&box, 3.5f);
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_OK, rc, "Expected RASTER_RC_OK");
    TEST_ASSERT_EQUAL_FLOAT_MESSAGE(3.5f, label.data.decimal.value, "Expected label.data.decimal.value to be updated to 3.5f");
    TEST_ASSERT_TRUE_MESSAGE(box.updated, "Setting label float data must mark the box as updated");
}

/*! \} */

/*!
 * \defgroup raster_api_set_label_*_format Tests for raster_api_set_label_*_format()
 * \{
 */

void test_raster_api_set_label_format_updates_format(void) {
    struct RasterLabel label = make_int_label();
    label.type = RASTER_LABEL_DATA_FLOAT;
    struct RasterBox box = { false, 0x1, { 0, 0, 1, 1 }, { 0 }, &label };
    enum RasterReturnCode rc = raster_api_set_label_int_format(&box, true);
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_OK, rc, "Expected RASTER_RC_OK");
    TEST_ASSERT_TRUE_MESSAGE(label.data.integer.is_unsigned, "Expected label.data.integer.is_unsigned to be updated to true");
    TEST_ASSERT_TRUE_MESSAGE(box.updated, "Setting label format must mark the box as updated");
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_LABEL_DATA_INT, label.type, "Expected label.type to remain RASTER_LABEL_DATA_INT");
}

void test_raster_api_set_label_format_null_box(void) {
    enum RasterReturnCode rc = raster_api_set_label_int_format(NULL, true);
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, rc);
}

void test_raster_api_set_label_format_null_label(void) {
    struct RasterBox box = { false, 0x1, { 0, 0, 1, 1 }, { 0 }, NULL };
    enum RasterReturnCode rc = raster_api_set_label_int_format(&box, true);
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, rc);
}

void test_raster_api_set_label_float_format_updates_format(void) {
    struct RasterLabel label = make_int_label();
    struct RasterBox box = { false, 0x1, { 0, 0, 1, 1 }, { 0 }, &label };
    enum RasterReturnCode rc = raster_api_set_label_float_format(&box, 2);
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_OK, rc, "Expected RASTER_RC_OK");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(2, label.data.decimal.precision, "Expected label.data.decimal.precision to be updated to 2");
    TEST_ASSERT_TRUE_MESSAGE(box.updated, "Setting label format must mark the box as updated");
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_LABEL_DATA_FLOAT, label.type, "Expected label.type to remain RASTER_LABEL_DATA_FLOAT");
}

void test_raster_api_set_label_string_format_updates_format(void) {
    struct RasterLabel label = make_int_label();
    label.type = RASTER_LABEL_DATA_STRING;
    struct RasterBox box = { false, 0x1, { 0, 0, 1, 1 }, { 0 }, &label };
    enum RasterReturnCode rc = raster_api_set_label_string_format(&box, 10);
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_OK, rc, "Expected RASTER_RC_OK");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(10, label.data.string.max_length, "Expected label.data.string.max_length to be updated to 10");
    TEST_ASSERT_TRUE_MESSAGE(box.updated, "Setting label format must mark the box as updated");
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_LABEL_DATA_STRING, label.type, "Expected label.type to remain RASTER_LABEL_DATA_STRING");
}

/*! \} */

/*!
 * \defgroup raster_api_render_label Tests that exercise label rendering through font_api_draw
 * \{
 */

void test_raster_api_render_with_label_draws_background_then_glyph(void) {
    struct RasterLabel label;
    raster_api_create_label(&label,
                            (union RasterLabelData){ .string = { .value = "A", .length = 1, .max_length = 0 } },
                            RASTER_LABEL_DATA_STRING,
                            (struct RasterCoords){ 5, 6 },
                            &test_font,
                            test_font.base_size,
                            FONT_ALIGN_LEFT,
                            (struct Color){ .argb = 0xFF112233 });
    boxes[0] = (struct RasterBox){ .updated = true, .id = 0x1, .rect = { 10, 20, 100, 30 }, .color = { .argb = 0xFF000000 }, .label = &label };
    boxes[1].updated = false;

    enum RasterReturnCode rc = raster_api_render(&handler);
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_OK, rc, "Expected RASTER_RC_OK");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(2, fake_draw_fake.call_count, "One call for box bg, one for glyph 'A'");

    // first call: box background.
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(10, fake_draw_fake.arg0_history[0], "Expected first draw call to receive box.x as arg0");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(20, fake_draw_fake.arg1_history[0], "Expected first draw call to receive box.y as arg1");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(100, fake_draw_fake.arg2_history[0], "Expected first draw call to receive box.width as arg2");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(30, fake_draw_fake.arg3_history[0], "Expected first draw call to receive box.height as arg3");
    TEST_ASSERT_EQUAL_HEX32_MESSAGE(0xFF000000, fake_draw_fake.arg4_history[0].argb, "Expected first draw call to receive box.color as arg4");

    // second call: glyph at (box.x + label.pos.x, box.y + label.pos.y).
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(15, fake_draw_fake.arg0_history[1], "Expected second draw call to receive box.x + label.pos.x as arg0");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(26, fake_draw_fake.arg1_history[1], "Expected second draw call to receive box.y + label.pos.y as arg1");
}

/*! \} */

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_raster_api_init_successful_partial);
    RUN_TEST(test_raster_api_init_successful_full_redraw);
    RUN_TEST(test_raster_api_init_null_handler);
    RUN_TEST(test_raster_api_init_null_interface);
    RUN_TEST(test_raster_api_init_zero_size);
    RUN_TEST(test_raster_api_init_null_draw);

    RUN_TEST(test_raster_api_set_interface_successful);
    RUN_TEST(test_raster_api_set_interface_null_handler);
    RUN_TEST(test_raster_api_set_interface_null_interface);
    RUN_TEST(test_raster_api_set_interface_zero_size);

    RUN_TEST(test_raster_api_render_partial_draws_updated_only);
    RUN_TEST(test_raster_api_render_partial_clears_updated_flag);
    RUN_TEST(test_raster_api_render_partial_skips_when_no_updates);
    RUN_TEST(test_raster_api_render_full_redraw_calls_clear_and_all_boxes);
    RUN_TEST(test_raster_api_render_full_redraw_keeps_updated_flag);
    RUN_TEST(test_raster_api_render_propagates_clear_error);
    RUN_TEST(test_raster_api_render_propagates_draw_error);
    RUN_TEST(test_raster_api_render_null_handler);
    RUN_TEST(test_raster_api_render_null_draw);
    RUN_TEST(test_raster_api_render_null_interface);
    RUN_TEST(test_raster_api_render_passes_box_geometry);

    RUN_TEST(test_raster_api_get_box_first);
    RUN_TEST(test_raster_api_get_box_last);
    RUN_TEST(test_raster_api_get_box_not_found);
    RUN_TEST(test_raster_api_get_box_null_boxes);
    RUN_TEST(test_raster_api_get_box_zero_size);

    RUN_TEST(test_raster_api_create_label_successful);
    RUN_TEST(test_raster_api_create_label_signed_int_format);
    RUN_TEST(test_raster_api_create_label_null_label);
    RUN_TEST(test_raster_api_create_label_null_font);

    RUN_TEST(test_raster_api_set_label_string_updates_value);
    RUN_TEST(test_raster_api_set_label_string_null_text);
    RUN_TEST(test_raster_api_set_label_string_null_box);
    RUN_TEST(test_raster_api_set_label_int_updates_value);
    RUN_TEST(test_raster_api_set_label_float_updates_value);
    RUN_TEST(test_raster_api_set_label_format_updates_format);
    RUN_TEST(test_raster_api_set_label_format_null_box);
    RUN_TEST(test_raster_api_set_label_format_null_label);

    RUN_TEST(test_raster_api_set_label_format_updates_format);
    RUN_TEST(test_raster_api_set_label_format_null_box);
    RUN_TEST(test_raster_api_set_label_format_null_label);
    RUN_TEST(test_raster_api_set_label_float_format_updates_format);
    RUN_TEST(test_raster_api_set_label_string_format_updates_format);

    RUN_TEST(test_raster_api_render_with_label_draws_background_then_glyph);

    return UNITY_END();
}
