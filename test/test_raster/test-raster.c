#include "fff.h"
#include "label-api.h"
#include "raster-api.h"
#include "test-font.h"
#include "eagletrt.h"
#include "unity.h"

#include <stddef.h>

DEFINE_FFF_GLOBALS;
FAKE_VALUE_FUNC(enum RasterReturnCode, fake_draw, uint16_t, uint16_t, uint16_t, uint16_t, struct Color);
FAKE_VALUE_FUNC(enum RasterReturnCode, fake_clear);

EAGLETRT_STATIC struct RasterHandler partial_handler;
EAGLETRT_STATIC struct RasterHandler clear_handler;
EAGLETRT_STATIC struct Box boxes[2] = {
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
    raster_api_init(&partial_handler, boxes, 2, fake_draw, NULL);
    raster_api_init(&clear_handler, boxes, 2, fake_draw, fake_clear);
}

/*!
 * \defgroup raster_api_init Tests for raster_api_init()
 * \{
 */

void test_raster_api_init_successful_partial(void) {
    enum RasterReturnCode rc = raster_api_init(&partial_handler, boxes, 2, fake_draw, NULL);
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_OK, rc, "Expected RASTER_RC_OK");
    TEST_ASSERT_EQUAL_PTR_MESSAGE(boxes, partial_handler.interface, "Expected handler.interface to point to boxes");
    TEST_ASSERT_EQUAL_MESSAGE(2, partial_handler.box_count, "Expected handler.size to be 2");
    TEST_ASSERT_EQUAL_PTR_MESSAGE(fake_draw, partial_handler.draw, "Expected handler.draw to be fake_draw");
    TEST_ASSERT_NULL_MESSAGE(partial_handler.clear, "Expected handler.clear to be NULL (partial mode)");
}

void test_raster_api_init_successful_full_redraw(void) {
    enum RasterReturnCode rc = raster_api_init(&clear_handler, boxes, 2, fake_draw, fake_clear);
    TEST_ASSERT_EQUAL(RASTER_RC_OK, rc);
    TEST_ASSERT_EQUAL_PTR_MESSAGE(fake_clear, clear_handler.clear, "Expected handler.clear to be fake_clear (full-redraw mode)");
}

void test_raster_api_init_null_handler(void) {
    enum RasterReturnCode rc = raster_api_init(NULL, boxes, 2, fake_draw, NULL);
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, rc);
}

void test_raster_api_init_null_interface(void) {
    enum RasterReturnCode rc = raster_api_init(&partial_handler, NULL, 2, fake_draw, NULL);
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, rc);
}

void test_raster_api_init_zero_size(void) {
    enum RasterReturnCode rc = raster_api_init(&partial_handler, boxes, 0, fake_draw, NULL);
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, rc);
}

void test_raster_api_init_null_draw(void) {
    enum RasterReturnCode rc = raster_api_init(&partial_handler, boxes, 2, NULL, NULL);
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, rc);
}

/*! \} */

/*!
 * \defgroup raster_api_set_interface Tests for raster_api_set_interface()
 * \{
 */

void test_raster_api_set_interface_successful(void) {
    struct Box new_box[1] = {
        { true, 0x3, { 1, 1, 1, 1 }, { .argb = 0xFF000000 }, NULL },
    };
    enum RasterReturnCode rc = raster_api_set_interface(&partial_handler, new_box, 1);
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_OK, rc, "Expected RASTER_RC_OK");
    TEST_ASSERT_EQUAL_PTR_MESSAGE(new_box, partial_handler.interface, "Expected handler.interface to point to new_box");
    TEST_ASSERT_EQUAL_MESSAGE(1, partial_handler.box_count, "Expected handler.size to be 1");
}

void test_raster_api_set_interface_null_handler(void) {
    struct Box new_box[1] = { 0 };
    enum RasterReturnCode rc = raster_api_set_interface(NULL, new_box, 1);
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, rc);
}

void test_raster_api_set_interface_null_interface(void) {
    enum RasterReturnCode rc = raster_api_set_interface(&partial_handler, NULL, 1);
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, rc);
}

void test_raster_api_set_interface_zero_size(void) {
    struct Box new_box[1] = { 0 };
    enum RasterReturnCode rc = raster_api_set_interface(&partial_handler, new_box, 0);
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
    enum RasterReturnCode rc = raster_api_render(&partial_handler);
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_OK, rc, "Expected RASTER_RC_OK");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(1, fake_draw_fake.call_count, "Only the updated box should be redrawn");
}

void test_raster_api_render_partial_clears_updated_flag(void) {
    boxes[0].updated = true;
    boxes[1].updated = true;
    raster_api_render(&partial_handler);
    TEST_ASSERT_FALSE_MESSAGE(boxes[0].updated, "updated should be cleared after redraw");
    TEST_ASSERT_FALSE_MESSAGE(boxes[1].updated, "updated should be cleared after redraw");
}

void test_raster_api_render_partial_skips_when_no_updates(void) {
    boxes[0].updated = false;
    boxes[1].updated = false;
    raster_api_render(&partial_handler);
    TEST_ASSERT_EQUAL_UINT(0, fake_draw_fake.call_count);
}

void test_raster_api_render_full_redraw_calls_clear_and_all_boxes(void) {
    boxes[0].updated = false;
    boxes[1].updated = false;
    enum RasterReturnCode rc = raster_api_render(&clear_handler);
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_OK, rc, "Expected RASTER_RC_OK");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(1, fake_clear_fake.call_count, "Clear should run once");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(2, fake_draw_fake.call_count, "Both boxes should be drawn even when not flagged");
}

void test_raster_api_render_full_redraw_keeps_updated_flag(void) {
    boxes[0].updated = true;
    boxes[1].updated = true;
    raster_api_render(&clear_handler);
    TEST_ASSERT_TRUE_MESSAGE(boxes[0].updated, "Full-redraw mode should not touch the updated flag");
    TEST_ASSERT_TRUE_MESSAGE(boxes[1].updated, "Full-redraw mode should not touch the updated flag");
}

void test_raster_api_render_propagates_clear_error(void) {
    fake_clear_fake.return_val = RASTER_RC_ERROR;
    enum RasterReturnCode rc = raster_api_render(&clear_handler);
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_ERROR, rc, "Expected RASTER_RC_ERROR when clear fails");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(0, fake_draw_fake.call_count, "Draw should not run when clear fails");
}

void test_raster_api_render_propagates_draw_error(void) {
    fake_draw_fake.return_val = RASTER_RC_ERROR;
    enum RasterReturnCode rc = raster_api_render(&partial_handler);
    TEST_ASSERT_EQUAL(RASTER_RC_ERROR, rc);
}

void test_raster_api_render_null_handler(void) {
    enum RasterReturnCode rc = raster_api_render(NULL);
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, rc);
}

void test_raster_api_render_null_draw(void) {
    partial_handler.draw = NULL;
    enum RasterReturnCode rc = raster_api_render(&partial_handler);
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, rc);
}

void test_raster_api_render_null_interface(void) {
    partial_handler.interface = NULL;
    enum RasterReturnCode rc = raster_api_render(&partial_handler);
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, rc);
}

void test_raster_api_render_passes_box_geometry(void) {
    boxes[0] = (struct Box){ .updated = true, .id = 0x1, .rect = { 5, 6, 7, 8 }, .color = { .argb = 0x12345678 }, .label = NULL };
    boxes[1].updated = false;
    raster_api_render(&partial_handler);
    TEST_ASSERT_EQUAL_UINT_MESSAGE(1, fake_draw_fake.call_count, "Only the updated box should be redrawn");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(5, fake_draw_fake.arg0_history[0], "Expected draw to receive box.x as arg0");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(6, fake_draw_fake.arg1_history[0], "Expected draw to receive box.y as arg1");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(7, fake_draw_fake.arg2_history[0], "Expected draw to receive box.width as arg2");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(8, fake_draw_fake.arg3_history[0], "Expected draw to receive box.height as arg3");
    TEST_ASSERT_EQUAL_HEX32_MESSAGE(0x12345678, fake_draw_fake.arg4_history[0].argb, "Expected draw to receive box.color as arg4");
}

void test_raster_api_render_with_label_draws_background_then_glyph(void) {
    struct Label label;
    label_api_init(&label, "A", 5, 6, &test_font, test_font.base_size, FONT_ALIGN_LEFT, (struct Color){ .argb = 0xFF112233 });
    boxes[0] = (struct Box){ .updated = true, .id = 0x1, .rect = { 10, 20, 100, 30 }, .color = { .argb = 0xFF000000 }, .label = &label };
    boxes[1].updated = false;

    enum RasterReturnCode rc = raster_api_render(&partial_handler);
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_OK, rc, "Expected RASTER_RC_OK");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(2, fake_draw_fake.call_count, "One call for box bg, one for glyph 'A'");

    TEST_ASSERT_EQUAL_UINT16_MESSAGE(10, fake_draw_fake.arg0_history[0], "Expected first draw call to receive box.x as arg0");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(20, fake_draw_fake.arg1_history[0], "Expected first draw call to receive box.y as arg1");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(100, fake_draw_fake.arg2_history[0], "Expected first draw call to receive box.width as arg2");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(30, fake_draw_fake.arg3_history[0], "Expected first draw call to receive box.height as arg3");
    TEST_ASSERT_EQUAL_HEX32_MESSAGE(0xFF000000, fake_draw_fake.arg4_history[0].argb, "Expected first draw call to receive box.color as arg4");

    // second call: glyph at (box.x + label.offset_x, box.y + label.offset_y).
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(15, fake_draw_fake.arg0_history[1], "Expected second draw call to receive box.x + label.offset_x as arg0");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(26, fake_draw_fake.arg1_history[1], "Expected second draw call to receive box.y + label.offset_y as arg1");
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
    RUN_TEST(test_raster_api_render_with_label_draws_background_then_glyph);

    return UNITY_END();
}
