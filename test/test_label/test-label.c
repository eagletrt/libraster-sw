#include "fff.h"
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
 * \defgroup label_api_init Tests for label_api_init()
 * \{
 */

void test_label_api_init_sets_fields(void) {
    struct Label label;
    enum RasterReturnCode rc = label_api_init(&label, "hi", -3, 7, &test_font, 16, FONT_ALIGN_CENTER, (struct Color){ .argb = 0xFFAABBCC });
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_OK, rc, "Initialization should succeed");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("hi", label.text, "Text should be set correctly");
    TEST_ASSERT_EQUAL_INT16_MESSAGE(-3, label.offset_x, "Offset X should be set correctly");
    TEST_ASSERT_EQUAL_INT16_MESSAGE(7, label.offset_y, "Offset Y should be set correctly");
    TEST_ASSERT_EQUAL_PTR_MESSAGE(&test_font, label.font, "Font pointer should be set correctly");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(16, label.size, "Font size should be set correctly");
    TEST_ASSERT_EQUAL_MESSAGE(FONT_ALIGN_CENTER, label.alignment, "Font alignment should be set correctly");
    TEST_ASSERT_EQUAL_HEX32_MESSAGE(0xFFAABBCC, label.color.argb, "Color should be set correctly");
}

void test_label_api_init_null_label(void) {
    enum RasterReturnCode rc = label_api_init(NULL, "x", 0, 0, &test_font, 20, FONT_ALIGN_LEFT, (struct Color){ 0 });
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, rc);
}

void test_label_api_init_null_font(void) {
    struct Label label;
    enum RasterReturnCode rc = label_api_init(&label, "x", 0, 0, NULL, 20, FONT_ALIGN_LEFT, (struct Color){ 0 });
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, rc);
}

/*! \} */

/*!
 * \defgroup label_api_set_text Tests for label_api_set_text()
 * \{
 */

void test_label_api_set_text_updates_pointer(void) {
    struct Label label;
    label_api_init(&label, "old", 0, 0, &test_font, 20, FONT_ALIGN_LEFT, (struct Color){ 0 });
    char *new_text = "new";
    enum RasterReturnCode rc = label_api_set_text(&label, new_text);
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_OK, rc, "Setting text should succeed");
    TEST_ASSERT_EQUAL_PTR_MESSAGE(new_text, label.text, "Text pointer should be updated to the new value");
}

void test_label_api_set_text_accepts_null(void) {
    struct Label label;
    label_api_init(&label, "old", 0, 0, &test_font, 20, FONT_ALIGN_LEFT, (struct Color){ 0 });
    enum RasterReturnCode rc = label_api_set_text(&label, NULL);
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_OK, rc, "Setting text to NULL should succeed");
    TEST_ASSERT_NULL_MESSAGE(label.text, "Text pointer should be set to NULL when given NULL");
}

void test_label_api_set_text_null_label(void) {
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, label_api_set_text(NULL, "x"));
}

/*! \} */

/*!
 * \defgroup label_api_draw Tests for label_api_draw()
 * \{
 */

void test_label_api_draw_renders_at_explicit_position(void) {
    struct Label label;
    label_api_init(&label, "A", 999, 999, &test_font, test_font.base_size, FONT_ALIGN_LEFT, (struct Color){ .argb = 0xFFFFFFFF });
    enum RasterReturnCode rc = label_api_draw(&label, 30, 40, fake_draw);
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_OK, rc, "Drawing should succeed");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(1, fake_draw_fake.call_count, "Should render the single glyph");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(30, fake_draw_fake.arg0_history[0], "Standalone draw must ignore offsets and anchor at the given X");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(40, fake_draw_fake.arg1_history[0], "Standalone draw must ignore offsets and anchor at the given Y");
}

void test_label_api_draw_null_text_is_noop(void) {
    struct Label label;
    label_api_init(&label, NULL, 0, 0, &test_font, 20, FONT_ALIGN_LEFT, (struct Color){ 0 });
    enum RasterReturnCode rc = label_api_draw(&label, 0, 0, fake_draw);
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_OK, rc, "Drawing should succeed even with NULL text");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(0, fake_draw_fake.call_count, "No draw calls should be made when text is NULL");
}

void test_label_api_draw_null_label(void) {
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, label_api_draw(NULL, 0, 0, fake_draw));
}

void test_label_api_draw_null_callback(void) {
    struct Label label;
    label_api_init(&label, "A", 0, 0, &test_font, 20, FONT_ALIGN_LEFT, (struct Color){ 0 });
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, label_api_draw(&label, 0, 0, NULL));
}

/*! \} */

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_label_api_init_sets_fields);
    RUN_TEST(test_label_api_init_null_label);
    RUN_TEST(test_label_api_init_null_font);

    RUN_TEST(test_label_api_set_text_updates_pointer);
    RUN_TEST(test_label_api_set_text_accepts_null);
    RUN_TEST(test_label_api_set_text_null_label);

    RUN_TEST(test_label_api_draw_renders_at_explicit_position);
    RUN_TEST(test_label_api_draw_null_text_is_noop);
    RUN_TEST(test_label_api_draw_null_label);
    RUN_TEST(test_label_api_draw_null_callback);

    return UNITY_END();
}
