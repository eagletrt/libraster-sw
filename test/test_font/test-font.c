#include "fff.h"
#include "font-api.h"
#include "test-font.h"
#include "unity.h"

#include <stddef.h>

DEFINE_FFF_GLOBALS;
FAKE_VALUE_FUNC(enum RasterReturnCode, fake_draw, uint16_t, uint16_t, uint16_t, uint16_t, struct Color);

uint32_t prv_q16_multiply(uint32_t value, uint32_t multiplier_q16);

enum RasterReturnCode prv_emit_run(uint8_t alpha, uint16_t count, uint16_t glyph_width, uint32_t multiplier_q16, uint16_t origin_x, uint16_t origin_y, uint32_t base_argb, int16_t *current_x, int16_t *current_y, raster_draw_rectangle_callback draw);

enum RasterReturnCode prv_render_glyph(const struct FontGlyph *glyph, const struct Font *font, uint16_t origin_x, uint16_t origin_y, uint32_t multiplier_q16, struct Color color, raster_draw_rectangle_callback draw);

#define Q16_ONE (0x10000u)
#define Q16_HALF (0x8000u)

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
    TEST_ASSERT_EQUAL_HEX8_MESSAGE(0xFF, got.a, "Alpha should be the glyph coverage");
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

/*!
 * \defgroup prv_q16_multiply Tests for prv_q16_multiply()
 * \{
 */

void test_prv_q16_multiply_identity(void) {
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(123, prv_q16_multiply(123, Q16_ONE), "Multiplying by Q16 one must return the value unchanged");
}

void test_prv_q16_multiply_zero_value(void) {
    TEST_ASSERT_EQUAL_UINT32(0, prv_q16_multiply(0, Q16_ONE));
}

void test_prv_q16_multiply_zero_multiplier(void) {
    TEST_ASSERT_EQUAL_UINT32(0, prv_q16_multiply(999, 0));
}

void test_prv_q16_multiply_half_rounds_up(void) {
    TEST_ASSERT_EQUAL_UINT32(5, prv_q16_multiply(10, Q16_HALF));
}

void test_prv_q16_multiply_rounds_to_nearest_up(void) {
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, prv_q16_multiply(3, Q16_HALF), "1.5 must round up to 2");
}

void test_prv_q16_multiply_rounds_to_nearest_down(void) {
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, prv_q16_multiply(1, Q16_ONE / 4u), "0.25 must round down to 0");
}

void test_prv_q16_multiply_doubles(void) {
    TEST_ASSERT_EQUAL_UINT32(84, prv_q16_multiply(42, Q16_ONE * 2u));
}

/*! \} */

/*!
 * \defgroup prv_emit_run Tests for prv_emit_run()
 * \{
 */

void test_prv_emit_run_low_alpha_skips_draw_and_advances_cursor(void) {
    int16_t cx = 0;
    int16_t cy = 0;
    enum RasterReturnCode rc = prv_emit_run(0x00, 5, 10, Q16_ONE, 0, 0, 0, &cx, &cy, fake_draw);
    TEST_ASSERT_EQUAL(RASTER_RC_OK, rc);
    TEST_ASSERT_EQUAL_UINT_MESSAGE(0, fake_draw_fake.call_count, "Sub-threshold alpha must skip drawing");
    TEST_ASSERT_EQUAL_INT16_MESSAGE(5, cx, "Cursor X must advance by count when no wrap occurs");
    TEST_ASSERT_EQUAL_INT16_MESSAGE(0, cy, "Cursor Y must stay on the same row");
}

void test_prv_emit_run_low_alpha_wraps_cursor_across_rows(void) {
    int16_t cx = 8;
    int16_t cy = 1;
    enum RasterReturnCode rc = prv_emit_run(0x00, 5, 10, Q16_ONE, 0, 0, 0, &cx, &cy, fake_draw);
    TEST_ASSERT_EQUAL(RASTER_RC_OK, rc);
    TEST_ASSERT_EQUAL_INT16(3, cx);
    TEST_ASSERT_EQUAL_INT16(2, cy);
}

void test_prv_emit_run_visible_single_row_no_wrap(void) {
    int16_t cx = 0;
    int16_t cy = 0;
    enum RasterReturnCode rc = prv_emit_run(0xF0, 4, 10, Q16_ONE, 100, 200, 0x00112233u, &cx, &cy, fake_draw);
    TEST_ASSERT_EQUAL(RASTER_RC_OK, rc);
    TEST_ASSERT_EQUAL_UINT_MESSAGE(1, fake_draw_fake.call_count, "A visible run that fits in one row must produce exactly one rectangle");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(100, fake_draw_fake.arg0_history[0], "Rectangle X must be origin_x + 0");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(200, fake_draw_fake.arg1_history[0], "Rectangle Y must be origin_y + 0");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(4, fake_draw_fake.arg2_history[0], "Width must equal the take count");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(1, fake_draw_fake.arg3_history[0], "Height must be 1 at unit scale");
    TEST_ASSERT_EQUAL_HEX32_MESSAGE(0xF0112233u, fake_draw_fake.arg4_history[0].argb, "Alpha must be the run alpha OR'd into base_argb");
    TEST_ASSERT_EQUAL_INT16(4, cx);
    TEST_ASSERT_EQUAL_INT16(0, cy);
}

void test_prv_emit_run_visible_wraps_to_next_row(void) {
    int16_t cx = 8;
    int16_t cy = 0;
    enum RasterReturnCode rc = prv_emit_run(0xF0, 5, 10, Q16_ONE, 0, 0, 0, &cx, &cy, fake_draw);
    TEST_ASSERT_EQUAL(RASTER_RC_OK, rc);
    TEST_ASSERT_EQUAL_UINT_MESSAGE(2, fake_draw_fake.call_count, "Wrapping must split into two draw calls");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(8, fake_draw_fake.arg0_history[0], "First slice starts at the original cx");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(0, fake_draw_fake.arg1_history[0], "First slice on row 0");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(2, fake_draw_fake.arg2_history[0], "First slice consumes the rest of the row");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(0, fake_draw_fake.arg0_history[1], "Second slice starts at the row beginning");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(1, fake_draw_fake.arg1_history[1], "Second slice on row 1");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(3, fake_draw_fake.arg2_history[1], "Second slice carries the remainder");
    TEST_ASSERT_EQUAL_INT16(3, cx);
    TEST_ASSERT_EQUAL_INT16(1, cy);
}

void test_prv_emit_run_scaled_geometry(void) {
    int16_t cx = 0;
    int16_t cy = 0;
    enum RasterReturnCode rc = prv_emit_run(0xF0, 4, 10, Q16_ONE * 2u, 50, 60, 0, &cx, &cy, fake_draw);
    TEST_ASSERT_EQUAL(RASTER_RC_OK, rc);
    TEST_ASSERT_EQUAL_UINT_MESSAGE(1, fake_draw_fake.call_count, "Scaling must not split the run");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(50, fake_draw_fake.arg0_history[0], "Origin X applies unchanged at scaled cx=0");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(60, fake_draw_fake.arg1_history[0], "Origin Y applies unchanged at scaled cy=0");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(8, fake_draw_fake.arg2_history[0], "Width must double under 2x scale");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(2, fake_draw_fake.arg3_history[0], "Height must double under 2x scale");
}

void test_prv_emit_run_propagates_draw_error(void) {
    fake_draw_fake.return_val = RASTER_RC_ERROR;
    int16_t cx = 0;
    int16_t cy = 0;
    enum RasterReturnCode rc = prv_emit_run(0xF0, 2, 10, Q16_ONE, 0, 0, 0, &cx, &cy, fake_draw);
    TEST_ASSERT_EQUAL(RASTER_RC_ERROR, rc);
}

/*! \} */

/*!
 * \defgroup prv_render_glyph Tests for prv_render_glyph()
 * \{
 */

void test_prv_render_glyph_zero_width_is_noop(void) {
    struct FontGlyph empty = { 'X', 0, 3, 0, 20 };
    enum RasterReturnCode rc = prv_render_glyph(&empty, &test_font, 0, 0, Q16_ONE, (struct Color){ .argb = 0xFFFFFFFF }, fake_draw);
    TEST_ASSERT_EQUAL(RASTER_RC_OK, rc);
    TEST_ASSERT_EQUAL_UINT(0, fake_draw_fake.call_count);
}

void test_prv_render_glyph_zero_height_is_noop(void) {
    struct FontGlyph empty = { 'X', 0, 3, 10, 0 };
    enum RasterReturnCode rc = prv_render_glyph(&empty, &test_font, 0, 0, Q16_ONE, (struct Color){ .argb = 0xFFFFFFFF }, fake_draw);
    TEST_ASSERT_EQUAL(RASTER_RC_OK, rc);
    TEST_ASSERT_EQUAL_UINT(0, fake_draw_fake.call_count);
}

void test_prv_render_glyph_decodes_single_pixel_rle(void) {
    enum RasterReturnCode rc = prv_render_glyph(&test_glyphs[1], &test_font, 7, 9, Q16_ONE, (struct Color){ .argb = 0xFFAABBCC }, fake_draw);
    TEST_ASSERT_EQUAL(RASTER_RC_OK, rc);
    TEST_ASSERT_EQUAL_UINT_MESSAGE(1, fake_draw_fake.call_count, "The test glyph encodes a single opaque pixel");
    TEST_ASSERT_EQUAL_UINT16(7, fake_draw_fake.arg0_history[0]);
    TEST_ASSERT_EQUAL_UINT16(9, fake_draw_fake.arg1_history[0]);
}

void test_prv_render_glyph_propagates_emit_error(void) {
    fake_draw_fake.return_val = RASTER_RC_ERROR;
    enum RasterReturnCode rc = prv_render_glyph(&test_glyphs[1], &test_font, 0, 0, Q16_ONE, (struct Color){ .argb = 0xFFFFFFFF }, fake_draw);
    TEST_ASSERT_EQUAL(RASTER_RC_ERROR, rc);
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

    RUN_TEST(test_prv_q16_multiply_identity);
    RUN_TEST(test_prv_q16_multiply_zero_value);
    RUN_TEST(test_prv_q16_multiply_zero_multiplier);
    RUN_TEST(test_prv_q16_multiply_half_rounds_up);
    RUN_TEST(test_prv_q16_multiply_rounds_to_nearest_up);
    RUN_TEST(test_prv_q16_multiply_rounds_to_nearest_down);
    RUN_TEST(test_prv_q16_multiply_doubles);

    RUN_TEST(test_prv_emit_run_low_alpha_skips_draw_and_advances_cursor);
    RUN_TEST(test_prv_emit_run_low_alpha_wraps_cursor_across_rows);
    RUN_TEST(test_prv_emit_run_visible_single_row_no_wrap);
    RUN_TEST(test_prv_emit_run_visible_wraps_to_next_row);
    RUN_TEST(test_prv_emit_run_scaled_geometry);
    RUN_TEST(test_prv_emit_run_propagates_draw_error);

    RUN_TEST(test_prv_render_glyph_zero_width_is_noop);
    RUN_TEST(test_prv_render_glyph_zero_height_is_noop);
    RUN_TEST(test_prv_render_glyph_decodes_single_pixel_rle);
    RUN_TEST(test_prv_render_glyph_propagates_emit_error);

    return UNITY_END();
}
