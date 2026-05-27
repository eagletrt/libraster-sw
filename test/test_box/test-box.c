#include "fff.h"
#include "box-api.h"
#include "label-api.h"
#include "test-font.h"
#include "unity.h"

#include <stddef.h>

DEFINE_FFF_GLOBALS;
FAKE_VALUE_FUNC(enum RasterReturnCode, fake_draw, uint16_t, uint16_t, uint16_t, uint16_t, struct Color);

void setUp(void) {
    RESET_FAKE(fake_draw);
    FFF_RESET_HISTORY();
    fake_draw_fake.return_val = RASTER_RC_OK;
}

/*!
 * \defgroup box_api_init Tests for box_api_init()
 * \{
 */

void test_box_api_init_sets_fields_and_marks_updated(void) {
    struct Box box;
    enum RasterReturnCode rc = box_api_init(&box, 0x42, (struct BoxRectangle){ 1, 2, 3, 4 }, (struct Color){ .argb = 0xFF00FF00 }, NULL);
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_OK, rc, "box_api_init should return RASTER_RC_OK on success");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(0x42, box.id, "box_api_init should set the box ID");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(1, box.rect.x, "box_api_init should set the box X coordinate");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(2, box.rect.y, "box_api_init should set the box Y coordinate");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(3, box.rect.width, "box_api_init should set the box width");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(4, box.rect.height, "box_api_init should set the box height");
    TEST_ASSERT_EQUAL_HEX32_MESSAGE(0xFF00FF00, box.color.argb, "box_api_init should set the box color");
    TEST_ASSERT_NULL_MESSAGE(box.label, "box_api_init should initialize the label pointer to NULL");
    TEST_ASSERT_TRUE_MESSAGE(box.updated, "box_api_init should mark the box as updated");
}

void test_box_api_init_null_box(void) {
    enum RasterReturnCode rc = box_api_init(NULL, 0, (struct BoxRectangle){ 0, 0, 0, 0 }, (struct Color){ 0 }, NULL);
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, rc);
}

/*! \} */

/*!
 * \defgroup box_api_set_position Tests for box_api_set_position()
 * \{
 */

void test_box_api_set_position_updates_xy_and_marks_updated(void) {
    struct Box box = { false, 0, { 10, 20, 5, 5 }, { 0 }, NULL };
    enum RasterReturnCode rc = box_api_set_position(&box, 100, 200);
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_OK, rc, "box_api_set_position should return RASTER_RC_OK on success");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(100, box.rect.x, "box_api_set_position should update the box X coordinate");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(200, box.rect.y, "box_api_set_position should update the box Y coordinate");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(5, box.rect.width, "Width must not change when repositioning");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(5, box.rect.height, "Height must not change when repositioning");
    TEST_ASSERT_TRUE(box.updated);
}

void test_box_api_set_position_preserves_label(void) {
    struct Label label;
    label_api_init(&label, "X", 0, 0, &test_font, 20, FONT_ALIGN_LEFT, (struct Color){ .argb = 0xFFFFFFFF });
    struct Box box = { false, 0, { 0, 0, 1, 1 }, { 0 }, &label };
    box_api_set_position(&box, 5, 6);
    TEST_ASSERT_EQUAL_PTR_MESSAGE(&label, box.label, "Repositioning must not alter the label content");
}

void test_box_api_set_position_null_box(void) {
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, box_api_set_position(NULL, 0, 0));
}

/*! \} */

/*!
 * \defgroup box_api_set_label Tests for box_api_set_label() and box_api_clear_label()
 * \{
 */

void test_box_api_set_label_attaches_and_marks_updated(void) {
    struct Box box = { false, 0, { 0, 0, 1, 1 }, { 0 }, NULL };
    struct Label label;
    label_api_init(&label, "X", 0, 0, &test_font, 20, FONT_ALIGN_LEFT, (struct Color){ .argb = 0xFFFFFFFF });
    enum RasterReturnCode rc = box_api_set_label(&box, &label);
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_OK, rc, "box_api_set_label should return RASTER_RC_OK on success");
    TEST_ASSERT_EQUAL_PTR_MESSAGE(&label, box.label, "box_api_set_label should attach the label to the box");
    TEST_ASSERT_TRUE_MESSAGE(box.updated, "box_api_set_label should mark the box as updated");
}

void test_box_api_clear_label_detaches_and_marks_updated(void) {
    struct Label label;
    label_api_init(&label, "X", 0, 0, &test_font, 20, FONT_ALIGN_LEFT, (struct Color){ .argb = 0xFFFFFFFF });
    struct Box box = { false, 0, { 0, 0, 1, 1 }, { 0 }, &label };
    enum RasterReturnCode rc = box_api_clear_label(&box);
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_OK, rc, "box_api_clear_label should return RASTER_RC_OK on success");
    TEST_ASSERT_NULL_MESSAGE(box.label, "box_api_clear_label should detach the label from the box");
    TEST_ASSERT_TRUE_MESSAGE(box.updated, "box_api_clear_label should mark the box as updated");
}

void test_box_api_set_label_null_box(void) {
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, box_api_set_label(NULL, NULL));
}

void test_box_api_clear_label_null_box(void) {
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, box_api_clear_label(NULL));
}

/*! \} */

/*!
 * \defgroup box_api_draw Tests for box_api_draw()
 * \{
 */

void test_box_api_draw_emits_background(void) {
    struct Box box = { true, 0, { 5, 6, 7, 8 }, { .argb = 0x11223344 }, NULL };
    enum RasterReturnCode rc = box_api_draw(&box, fake_draw);
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_OK, rc, "box_api_draw should return RASTER_RC_OK on success");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(1, fake_draw_fake.call_count, "box_api_draw should emit exactly one draw call for the background");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(5, fake_draw_fake.arg0_history[0], "box_api_draw should emit the correct X coordinate for the background");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(6, fake_draw_fake.arg1_history[0], "box_api_draw should emit the correct Y coordinate for the background");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(7, fake_draw_fake.arg2_history[0], "box_api_draw should emit the correct width for the background");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(8, fake_draw_fake.arg3_history[0], "box_api_draw should emit the correct height for the background");
    TEST_ASSERT_EQUAL_HEX32_MESSAGE(0x11223344, fake_draw_fake.arg4_history[0].argb, "box_api_draw should emit the correct color for the background");
}

void test_box_api_draw_renders_label_at_offset(void) {
    struct Label label;
    label_api_init(&label, "A", 5, 6, &test_font, test_font.base_size, FONT_ALIGN_LEFT, (struct Color){ .argb = 0xFFFFFFFF });
    struct Box box = { true, 0, { 10, 20, 50, 50 }, { .argb = 0xFF000000 }, &label };
    enum RasterReturnCode rc = box_api_draw(&box, fake_draw);
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_OK, rc, "box_api_draw should return RASTER_RC_OK on success");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(2, fake_draw_fake.call_count, "Background + one glyph");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(15, fake_draw_fake.arg0_history[1], "box_api_draw should render the label at the correct X coordinate (box X + label offset X)");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(26, fake_draw_fake.arg1_history[1], "box_api_draw should render the label at the correct Y coordinate (box Y + label offset Y)");
}

void test_box_api_draw_does_not_clear_updated_flag(void) {
    struct Box box = { true, 0, { 0, 0, 1, 1 }, { 0 }, NULL };
    box_api_draw(&box, fake_draw);
    TEST_ASSERT_TRUE_MESSAGE(box.updated, "box_api_draw is a pure render and must not clear the flag");
}

void test_box_api_draw_null_box(void) {
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, box_api_draw(NULL, fake_draw));
}

void test_box_api_draw_null_callback(void) {
    struct Box box = { true, 0, { 0, 0, 1, 1 }, { 0 }, NULL };
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, box_api_draw(&box, NULL));
}

void test_box_api_draw_propagates_callback_error(void) {
    fake_draw_fake.return_val = RASTER_RC_ERROR;
    struct Box box = { true, 0, { 0, 0, 1, 1 }, { 0 }, NULL };
    TEST_ASSERT_EQUAL(RASTER_RC_ERROR, box_api_draw(&box, fake_draw));
}

/*! \} */

/*!
 * \defgroup box_api_find Tests for box_api_find()
 * \{
 */

void test_box_api_find(void) {
    struct Box boxes[3] = {
        { false, 0x1, { 0 }, { 0 }, NULL },
        { false, 0x2, { 0 }, { 0 }, NULL },
        { false, 0x3, { 0 }, { 0 }, NULL },
    };
    struct Box *got = box_api_find(boxes, 3, 0x2);
    TEST_ASSERT_EQUAL_PTR(&boxes[1], got);
}

void test_box_api_find_not_found(void) {
    struct Box boxes[1] = { { false, 0x1, { 0 }, { 0 }, NULL } };
    TEST_ASSERT_NULL(box_api_find(boxes, 1, 0xBEEF));
}

void test_box_api_find_null_array(void) {
    TEST_ASSERT_NULL(box_api_find(NULL, 1, 0x1));
}

void test_box_api_find_zero_size(void) {
    struct Box boxes[1] = { { false, 0x1, { 0 }, { 0 }, NULL } };
    TEST_ASSERT_NULL(box_api_find(boxes, 0, 0x1));
}

/*! \} */

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_box_api_init_sets_fields_and_marks_updated);
    RUN_TEST(test_box_api_init_null_box);

    RUN_TEST(test_box_api_set_position_updates_xy_and_marks_updated);
    RUN_TEST(test_box_api_set_position_preserves_label);
    RUN_TEST(test_box_api_set_position_null_box);

    RUN_TEST(test_box_api_set_label_attaches_and_marks_updated);
    RUN_TEST(test_box_api_clear_label_detaches_and_marks_updated);
    RUN_TEST(test_box_api_set_label_null_box);
    RUN_TEST(test_box_api_clear_label_null_box);

    RUN_TEST(test_box_api_draw_emits_background);
    RUN_TEST(test_box_api_draw_renders_label_at_offset);
    RUN_TEST(test_box_api_draw_does_not_clear_updated_flag);
    RUN_TEST(test_box_api_draw_null_box);
    RUN_TEST(test_box_api_draw_null_callback);
    RUN_TEST(test_box_api_draw_propagates_callback_error);

    RUN_TEST(test_box_api_find);
    RUN_TEST(test_box_api_find_not_found);
    RUN_TEST(test_box_api_find_null_array);
    RUN_TEST(test_box_api_find_zero_size);

    return UNITY_END();
}
