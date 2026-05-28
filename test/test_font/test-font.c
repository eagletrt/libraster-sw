#include "fff.h"
#include "font-api.h"
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
 * \defgroup color Tests for the Color union accessors
 * \{
 */

void test_color_components_combined(void) {
    struct Color color = { .argb = 0xAABBCCDD };
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0xAA, color.a, "Alpha byte should be the highest byte");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0xBB, color.r, "Red byte should be the second highest byte");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0xCC, color.g, "Green byte should be the third highest byte");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0xDD, color.b, "Blue byte should be the lowest byte");
}

void test_color_individual_assignment(void) {
    struct Color color;
    color.a = 0x12;
    color.r = 0x34;
    color.g = 0x56;
    color.b = 0x78;
    TEST_ASSERT_EQUAL_HEX32(0x12345678, color.argb);
}

/*! \} */

/*!
 * \defgroup font_api_find_glyph Tests for font_api_find_glyph()
 * \{
 */

void test_font_api_find_glyph_present(void) {
    const struct FontGlyph *g = font_api_find_glyph(&test_font, 'A');
    TEST_ASSERT_NOT_NULL_MESSAGE(g, "Expected to find glyph 'A'");
    TEST_ASSERT_EQUAL_INT_MESSAGE('A', g->character, "Wrong character returned");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(10, g->width, "Wrong glyph width");
}

void test_font_api_find_glyph_first(void) {
    const struct FontGlyph *g = font_api_find_glyph(&test_font, ' ');
    TEST_ASSERT_NOT_NULL_MESSAGE(g, "Expected to find glyph ' '");
    TEST_ASSERT_EQUAL_INT_MESSAGE(' ', g->character, "Wrong character returned");
}

void test_font_api_find_glyph_last(void) {
    const struct FontGlyph *g = font_api_find_glyph(&test_font, 't');
    TEST_ASSERT_NOT_NULL_MESSAGE(g, "Expected to find glyph 't'");
    TEST_ASSERT_EQUAL_INT_MESSAGE('t', g->character, "Wrong character returned");
}

void test_font_api_find_glyph_missing(void) {
    TEST_ASSERT_NULL(font_api_find_glyph(&test_font, 'Z'));
}

void test_font_api_find_glyph_null_font(void) {
    TEST_ASSERT_NULL(font_api_find_glyph(NULL, 'A'));
}

void test_font_api_find_glyph_null_callback(void) {
    struct Font broken = test_font;
    broken.find_glyph = NULL;
    TEST_ASSERT_NULL(font_api_find_glyph(&broken, 'A'));
}

/*! \} */

/*!
 * \defgroup font_api_length Tests for font_api_length()
 * \{
 */

// "Test" = T(8) + e(9) + s(7) + t(6) = 30 at native size.
void test_font_api_length_native_size(void) {
    uint16_t len = font_api_length("Test", test_font.base_size, &test_font);
    TEST_ASSERT_EQUAL_UINT16(30, len);
}

void test_font_api_length_empty_string(void) {
    TEST_ASSERT_EQUAL_UINT16(0, font_api_length("", test_font.base_size, &test_font));
}

void test_font_api_length_null_text(void) {
    TEST_ASSERT_EQUAL_UINT16(0, font_api_length(NULL, test_font.base_size, &test_font));
}

void test_font_api_length_null_font(void) {
    TEST_ASSERT_EQUAL_UINT16(0, font_api_length("ABC", 20, NULL));
}

void test_font_api_length_zero_base_size(void) {
    struct Font broken = test_font;
    broken.base_size = 0;
    TEST_ASSERT_EQUAL_UINT16(0, font_api_length("ABC", 20, &broken));
}

void test_font_api_length_single_char(void) {
    // 'A' width 10 at native size 20.
    TEST_ASSERT_EQUAL_UINT16(10, font_api_length("A", test_font.base_size, &test_font));
}

/*! \} */

/*!
 * \defgroup font_api_draw Tests for font_api_draw()
 * \{
 */

void test_font_api_draw_null_font(void) {
    enum RasterReturnCode rc = font_api_draw(0, 0, FONT_ALIGN_LEFT, NULL, "A", (struct Color){ .argb = 0xFFFFFFFF }, 20, fake_draw);
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_NULL_POINTER, rc, "Expected null pointer error when font is null");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(0, fake_draw_fake.call_count, "Draw callback should not be called when font is null");
}

void test_font_api_draw_null_text(void) {
    enum RasterReturnCode rc = font_api_draw(0, 0, FONT_ALIGN_LEFT, &test_font, NULL, (struct Color){ .argb = 0xFFFFFFFF }, 20, fake_draw);
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_NULL_POINTER, rc, "Expected null pointer error when text is null");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(0, fake_draw_fake.call_count, "Draw callback should not be called when text is null");
}

void test_font_api_draw_null_callback(void) {
    enum RasterReturnCode rc = font_api_draw(0, 0, FONT_ALIGN_LEFT, &test_font, "A", (struct Color){ .argb = 0xFFFFFFFF }, 20, NULL);
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, rc);
}

void test_font_api_draw_empty_text(void) {
    enum RasterReturnCode rc = font_api_draw(0, 0, FONT_ALIGN_LEFT, &test_font, "", (struct Color){ .argb = 0xFFFFFFFF }, 20, fake_draw);
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_OK, rc, "Expected OK return code when drawing empty text");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(0, fake_draw_fake.call_count, "Draw callback should not be called when text is empty");
}

void test_font_api_draw_left_alignment_origin(void) {
    enum RasterReturnCode rc = font_api_draw(5, 10, FONT_ALIGN_LEFT, &test_font, "A", (struct Color){ .argb = 0xFFFFFFFF }, 20, fake_draw);
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_OK, rc, "Expected OK return code when drawing with valid parameters");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(1, fake_draw_fake.call_count, "Expected draw callback to be called once for one character");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(5, fake_draw_fake.arg0_history[0], "Expected first rectangle at x=5");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(10, fake_draw_fake.arg1_history[0], "Expected first rectangle at y=10");
}

void test_font_api_draw_center_alignment_offset(void) {
    // "AB" length = 22 at native; center alignment subtracts len/2 = 11.
    enum RasterReturnCode rc = font_api_draw(100, 0, FONT_ALIGN_CENTER, &test_font, "AB", (struct Color){ .argb = 0xFFFFFFFF }, 20, fake_draw);
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_OK, rc, "Expected OK return code when drawing with valid parameters");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(2, fake_draw_fake.call_count, "Expected draw callback to be called twice for two characters");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(100 - 11, fake_draw_fake.arg0_history[0], "First glyph X should be x - len/2");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(100 - 11 + 10, fake_draw_fake.arg0_history[1], "Second glyph X should advance by glyph A width");
}

void test_font_api_draw_right_alignment_offset(void) {
    enum RasterReturnCode rc = font_api_draw(50, 0, FONT_ALIGN_RIGHT, &test_font, "AB", (struct Color){ .argb = 0xFFFFFFFF }, 20, fake_draw);
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_OK, rc, "Expected OK return code when drawing with valid parameters");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(2, fake_draw_fake.call_count, "Expected draw callback to be called twice for two characters");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(50 - 22, fake_draw_fake.arg0_history[0], "First glyph X should be x - len");
}

void test_font_api_draw_skips_unknown_chars(void) {
    enum RasterReturnCode rc = font_api_draw(0, 0, FONT_ALIGN_LEFT, &test_font, "ZA", (struct Color){ .argb = 0xFFFFFFFF }, 20, fake_draw);
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_OK, rc, "Expected OK return code when drawing with valid parameters");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(1, fake_draw_fake.call_count, "Expected draw callback to be called once for one known character");
}

void test_font_api_draw_propagates_callback_error(void) {
    fake_draw_fake.return_val = RASTER_RC_ERROR;
    enum RasterReturnCode rc = font_api_draw(0, 0, FONT_ALIGN_LEFT, &test_font, "A", (struct Color){ .argb = 0xFFFFFFFF }, 20, fake_draw);
    TEST_ASSERT_EQUAL(RASTER_RC_ERROR, rc);
}

void test_font_api_draw_carries_color_with_alpha(void) {
    enum RasterReturnCode rc = font_api_draw(0, 0, FONT_ALIGN_LEFT, &test_font, "A", (struct Color){ .argb = 0x00112233 }, 20, fake_draw);
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_OK, rc, "Expected OK return code when drawing with valid parameters");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(1, fake_draw_fake.call_count, "Expected draw callback to be called once for one character");
    struct Color got = fake_draw_fake.arg4_history[0];
    TEST_ASSERT_EQUAL_HEX8_MESSAGE(0xF0, got.a, "Alpha should be the glyph coverage");
    TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x11, got.r, "Red should come from the caller");
    TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x22, got.g, "Green should come from the caller");
    TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x33, got.b, "Blue should come from the caller");
}

void test_font_api_draw_advances_x_at_native_size(void) {
    enum RasterReturnCode rc = font_api_draw(0, 0, FONT_ALIGN_LEFT, &test_font, "BA", (struct Color){ .argb = 0xFFFFFFFF }, 20, fake_draw);
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_OK, rc, "Expected OK return code when drawing with valid parameters");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(2, fake_draw_fake.call_count, "Expected draw callback to be called twice for two characters");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(0, fake_draw_fake.arg0_history[0], "First glyph X should be at the initial X");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(12, fake_draw_fake.arg0_history[1], "Second glyph X should be first X + first glyph width (12)");
}

void test_font_api_draw_scales_size(void) {
    enum RasterReturnCode rc = font_api_draw(0, 0, FONT_ALIGN_LEFT, &test_font, "A", (struct Color){ .argb = 0xFFFFFFFF }, 40, fake_draw);
    TEST_ASSERT_EQUAL_MESSAGE(RASTER_RC_OK, rc, "Expected OK return code when drawing with valid parameters");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(1, fake_draw_fake.call_count, "Expected draw callback to be called once for one character");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(2, fake_draw_fake.arg2_history[0], "Width should be doubled");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(2, fake_draw_fake.arg3_history[0], "Height should be doubled");
}

/*! \} */

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_color_components_combined);
    RUN_TEST(test_color_individual_assignment);

    RUN_TEST(test_font_api_find_glyph_present);
    RUN_TEST(test_font_api_find_glyph_first);
    RUN_TEST(test_font_api_find_glyph_last);
    RUN_TEST(test_font_api_find_glyph_missing);
    RUN_TEST(test_font_api_find_glyph_null_font);
    RUN_TEST(test_font_api_find_glyph_null_callback);

    RUN_TEST(test_font_api_length_native_size);
    RUN_TEST(test_font_api_length_empty_string);
    RUN_TEST(test_font_api_length_null_text);
    RUN_TEST(test_font_api_length_null_font);
    RUN_TEST(test_font_api_length_zero_base_size);
    RUN_TEST(test_font_api_length_single_char);

    RUN_TEST(test_font_api_draw_null_font);
    RUN_TEST(test_font_api_draw_null_text);
    RUN_TEST(test_font_api_draw_null_callback);
    RUN_TEST(test_font_api_draw_empty_text);
    RUN_TEST(test_font_api_draw_left_alignment_origin);
    RUN_TEST(test_font_api_draw_center_alignment_offset);
    RUN_TEST(test_font_api_draw_right_alignment_offset);
    RUN_TEST(test_font_api_draw_skips_unknown_chars);
    RUN_TEST(test_font_api_draw_propagates_callback_error);
    RUN_TEST(test_font_api_draw_carries_color_with_alpha);
    RUN_TEST(test_font_api_draw_advances_x_at_native_size);
    RUN_TEST(test_font_api_draw_scales_size);

    return UNITY_END();
}
