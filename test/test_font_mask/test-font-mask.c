/*!
 * \file test-font-mask.c
 * \date 2026-10-21
 * \author Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Unit tests for mask rendering (font/label/box/raster *_masked and raster_api_set_mask).
 *
 * \details The central property: rendering a string through a coverage mask
 *     must cover exactly the pixels, with exactly the coverage, that the
 *     rectangle path covers. Both paths are rendered into a coverage map
 *     (highest coverage wins, like the mask writer) and compared, with glyphs
 *     made of several runs, partial alphas, sub-threshold runs and runs that
 *     cross glyph rows, at native and fractional scale and with a mask small
 *     enough to force banding.
 */

#include "box-api.h"
#include "font-api.h"
#include "label-api.h"
#include "raster-api.h"
#include "unity.h"

#include <stddef.h>
#include <string.h>

/*
 * Test font: three glyphs, 5 rows tall at native size.
 *   'A' 6x5, 'B' 4x5, 'C' 3x4 (shorter, runs crossing rows).
 */
static const uint8_t mask_sdf_data[] = {
    /* 'A' (offset 0, 30 px) */
    0x00, 2, 0x80, 2, 0x00, 2,
    0x40, 1, 0xFF, 4, 0x40, 1,
    0xFF, 6,
    0x10, 1, 0xC0, 4, 0x10, 1,
    0xFF, 2, 0x00, 2, 0xFF, 2,
    /* 'B' (offset 30, 20 px) */
    0xFF, 4, 0xA0, 2, 0x00, 2, 0xFF, 4, 0x60, 3, 0x20, 1, 0xFF, 4,
    /* 'C' (offset 44, 12 px) */
    0xFF, 5, 0x00, 4, 0x90, 3,
};

static const struct FontGlyph mask_glyphs[] = {
    { 0x0041U, 0, 30, 6, 5 },  /* 'A' */
    { 0x0042U, 30, 14, 4, 5 }, /* 'B' */
    { 0x0043U, 44, 6, 3, 4 },  /* 'C' */
};

static const struct FontGlyph *mask_find_glyph(uint32_t codepoint) {
    switch (codepoint) {
        case 0x0041U: return &mask_glyphs[0];
        case 0x0042U: return &mask_glyphs[1];
        case 0x0043U: return &mask_glyphs[2];
        default: return NULL;
    }
}

static const struct Font mask_font = {
    .base_size = 5,
    .sdf_data = mask_sdf_data,
    .glyphs = mask_glyphs,
    .glyph_count = 3,
    .find_glyph = mask_find_glyph,
};

#define TEST_MAP_WIDTH (160U)
#define TEST_MAP_HEIGHT (64U)
#define TEST_MASK_CAPACITY (TEST_MAP_WIDTH * TEST_MAP_HEIGHT)

static uint8_t rect_map[TEST_MAP_HEIGHT][TEST_MAP_WIDTH];
static uint8_t mask_map[TEST_MAP_HEIGHT][TEST_MAP_WIDTH];
static uint8_t mask_scratch[TEST_MASK_CAPACITY];

static unsigned rect_calls;
static unsigned mask_calls;
static struct Color last_mask_color;
static enum RasterReturnCode mask_return;

static void plot(uint8_t map[TEST_MAP_HEIGHT][TEST_MAP_WIDTH], uint32_t x, uint32_t y, uint8_t alpha) {
    if (x < TEST_MAP_WIDTH && y < TEST_MAP_HEIGHT && alpha > map[y][x]) {
        map[y][x] = alpha;
    }
}

static enum RasterReturnCode rect_sink(uint16_t x, uint16_t y, uint16_t width, uint16_t height, struct Color color) {
    rect_calls++;
    for (uint32_t row = 0U; row < height; ++row) {
        for (uint32_t col = 0U; col < width; ++col) {
            plot(rect_map, (uint32_t)x + col, (uint32_t)y + row, color.a);
        }
    }
    return RASTER_RC_OK;
}

static enum RasterReturnCode mask_sink(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *mask, struct Color color) {
    mask_calls++;
    last_mask_color = color;
    for (uint32_t row = 0U; row < height; ++row) {
        for (uint32_t col = 0U; col < width; ++col) {
            plot(mask_map, (uint32_t)x + col, (uint32_t)y + row, mask[(row * width) + col]);
        }
    }
    return mask_return;
}

void setUp(void) {
    memset(rect_map, 0, sizeof(rect_map));
    memset(mask_map, 0, sizeof(mask_map));
    memset(mask_scratch, 0xEE, sizeof(mask_scratch)); /* garbage: the renderer must clear it */
    rect_calls = 0U;
    mask_calls = 0U;
    last_mask_color.argb = 0U;
    mask_return = RASTER_RC_OK;
}

void tearDown(void) {
}

static const struct Color text_color = { .argb = 0xFF336699U };

static void assert_paths_match(uint16_t x, uint16_t y, enum FontAlignment alignment, const char *text, uint16_t pixel_size, uint32_t mask_size) {
    const struct RasterMask mask = { .buffer = mask_scratch, .size = mask_size };
    TEST_ASSERT_EQUAL(RASTER_RC_OK, font_api_draw(x, y, alignment, &mask_font, text, text_color, pixel_size, rect_sink));
    TEST_ASSERT_EQUAL(RASTER_RC_OK, font_api_draw_masked(x, y, alignment, &mask_font, text, text_color, pixel_size, &mask, mask_sink));
    TEST_ASSERT_TRUE_MESSAGE(rect_calls > 0U, "the fixture must actually draw something");
    TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(rect_map, mask_map, sizeof(rect_map), "mask path must cover exactly what the rectangle path covers");
}

/*!
 * \defgroup font_api_draw_masked Equivalence with the rectangle path
 * \{
 */

void test_masked_matches_rectangles_native_size(void) {
    assert_paths_match(10U, 3U, FONT_ALIGN_LEFT, "ABCAB", 5U, TEST_MASK_CAPACITY);
    TEST_ASSERT_EQUAL_MESSAGE(1U, mask_calls, "a mask large enough renders the whole string in one call");
}

void test_masked_matches_rectangles_fractional_scale_centered(void) {
    assert_paths_match(80U, 7U, FONT_ALIGN_CENTER, "CABBA", 13U, TEST_MASK_CAPACITY); /* x2.6 */
    TEST_ASSERT_EQUAL(1U, mask_calls);
}

void test_masked_matches_rectangles_right_aligned_upscaled(void) {
    assert_paths_match(150U, 0U, FONT_ALIGN_RIGHT, "BAC", 22U, TEST_MASK_CAPACITY); /* x4.4 */
}

void test_masked_bands_when_mask_is_small(void) {
    struct FontBounds bounds;
    font_api_bounds(80U, 7U, FONT_ALIGN_CENTER, &mask_font, "CABBA", 13U, &bounds);
    const uint32_t two_rows = (uint32_t)bounds.width * 2U;

    assert_paths_match(80U, 7U, FONT_ALIGN_CENTER, "CABBA", 13U, two_rows);
    TEST_ASSERT_TRUE_MESSAGE(mask_calls > 1U, "a two-row mask must split the text into bands");
    TEST_ASSERT_TRUE(mask_calls <= ((uint32_t)bounds.height + 1U) / 2U);
}

void test_masked_blends_with_the_text_color(void) {
    const struct RasterMask mask = { .buffer = mask_scratch, .size = TEST_MASK_CAPACITY };
    const struct Color translucent = { .argb = 0x80123456U };
    font_api_draw_masked(0U, 0U, FONT_ALIGN_LEFT, &mask_font, "A", translucent, 5U, &mask, mask_sink);
    TEST_ASSERT_EQUAL_HEX32_MESSAGE(translucent.argb, last_mask_color.argb, "the callback gets the label color, alpha included");
}

void test_masked_rejects_mask_narrower_than_text(void) {
    const struct RasterMask mask = { .buffer = mask_scratch, .size = 3U };
    TEST_ASSERT_EQUAL(RASTER_RC_ERROR, font_api_draw_masked(0U, 0U, FONT_ALIGN_LEFT, &mask_font, "AB", text_color, 5U, &mask, mask_sink));
    TEST_ASSERT_EQUAL(0U, mask_calls);
}

void test_masked_empty_or_unknown_text_draws_nothing(void) {
    const struct RasterMask mask = { .buffer = mask_scratch, .size = TEST_MASK_CAPACITY };
    TEST_ASSERT_EQUAL(RASTER_RC_OK, font_api_draw_masked(0U, 0U, FONT_ALIGN_LEFT, &mask_font, "", text_color, 5U, &mask, mask_sink));
    TEST_ASSERT_EQUAL(RASTER_RC_OK, font_api_draw_masked(0U, 0U, FONT_ALIGN_LEFT, &mask_font, "zz", text_color, 5U, &mask, mask_sink));
    TEST_ASSERT_EQUAL(0U, mask_calls);
}

void test_masked_propagates_callback_error(void) {
    const struct RasterMask mask = { .buffer = mask_scratch, .size = TEST_MASK_CAPACITY };
    mask_return = RASTER_RC_ERROR;
    TEST_ASSERT_EQUAL(RASTER_RC_ERROR, font_api_draw_masked(0U, 0U, FONT_ALIGN_LEFT, &mask_font, "A", text_color, 5U, &mask, mask_sink));
}

void test_masked_null_arguments(void) {
    const struct RasterMask mask = { .buffer = mask_scratch, .size = TEST_MASK_CAPACITY };
    const struct RasterMask no_buffer = { .buffer = NULL, .size = TEST_MASK_CAPACITY };
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, font_api_draw_masked(0U, 0U, FONT_ALIGN_LEFT, NULL, "A", text_color, 5U, &mask, mask_sink));
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, font_api_draw_masked(0U, 0U, FONT_ALIGN_LEFT, &mask_font, NULL, text_color, 5U, &mask, mask_sink));
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, font_api_draw_masked(0U, 0U, FONT_ALIGN_LEFT, &mask_font, "A", text_color, 5U, NULL, mask_sink));
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, font_api_draw_masked(0U, 0U, FONT_ALIGN_LEFT, &mask_font, "A", text_color, 5U, &no_buffer, mask_sink));
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, font_api_draw_masked(0U, 0U, FONT_ALIGN_LEFT, &mask_font, "A", text_color, 5U, &mask, NULL));
}

/*! \} */

/*!
 * \defgroup font_api_bounds Tests for font_api_bounds()
 * \{
 */

void test_bounds_width_and_alignment(void) {
    struct FontBounds bounds;
    const uint16_t length = font_api_length("AB", 10U, &mask_font);

    TEST_ASSERT_EQUAL(RASTER_RC_OK, font_api_bounds(50U, 4U, FONT_ALIGN_LEFT, &mask_font, "AB", 10U, &bounds));
    TEST_ASSERT_EQUAL_UINT16(50U, bounds.x);
    TEST_ASSERT_EQUAL_UINT16(4U, bounds.y);
    TEST_ASSERT_EQUAL_UINT16(length, bounds.width);

    font_api_bounds(50U, 4U, FONT_ALIGN_RIGHT, &mask_font, "AB", 10U, &bounds);
    TEST_ASSERT_EQUAL_UINT16(50U - length, bounds.x);

    font_api_bounds(50U, 4U, FONT_ALIGN_CENTER, &mask_font, "AB", 10U, &bounds);
    TEST_ASSERT_EQUAL_UINT16(50U - length + (length / 2U), bounds.x);
}

void test_bounds_height_is_tallest_scaled_glyph(void) {
    struct FontBounds bounds;
    font_api_bounds(0U, 0U, FONT_ALIGN_LEFT, &mask_font, "C", 10U, &bounds);
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(8U, bounds.height, "'C' is 4 rows at base 5, x2 -> 8");
    font_api_bounds(0U, 0U, FONT_ALIGN_LEFT, &mask_font, "CA", 10U, &bounds);
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(10U, bounds.height, "'A' is 5 rows -> 10");
}

void test_bounds_empty_text(void) {
    struct FontBounds bounds;
    TEST_ASSERT_EQUAL(RASTER_RC_OK, font_api_bounds(7U, 9U, FONT_ALIGN_LEFT, &mask_font, "", 10U, &bounds));
    TEST_ASSERT_EQUAL_UINT16(0U, bounds.width);
    TEST_ASSERT_EQUAL_UINT16(0U, bounds.height);
}

void test_bounds_null_arguments(void) {
    struct FontBounds bounds;
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, font_api_bounds(0U, 0U, FONT_ALIGN_LEFT, NULL, "A", 10U, &bounds));
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, font_api_bounds(0U, 0U, FONT_ALIGN_LEFT, &mask_font, NULL, 10U, &bounds));
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, font_api_bounds(0U, 0U, FONT_ALIGN_LEFT, &mask_font, "A", 10U, NULL));
}

/*! \} */

/*!
 * \defgroup label_box_raster Masked label, box and raster rendering
 * \{
 */

static struct Label test_label;
static struct Box test_boxes[2];

static void init_boxes(void) {
    label_api_init(&test_label, "ABC", 4, 2, &mask_font, 10U, FONT_ALIGN_LEFT, text_color);
    box_api_init(&test_boxes[0], 0U, (struct BoxRectangle){ 0U, 0U, 40U, 20U }, (struct Color){ .argb = 0xFF000000U }, &test_label);
    box_api_init(&test_boxes[1], 1U, (struct BoxRectangle){ 40U, 0U, 40U, 20U }, (struct Color){ .argb = 0xFF000000U }, NULL);
}

void test_label_masked_matches_label_rectangles(void) {
    const struct RasterMask mask = { .buffer = mask_scratch, .size = TEST_MASK_CAPACITY };
    label_api_init(&test_label, "BCA", 0, 0, &mask_font, 13U, FONT_ALIGN_CENTER, text_color);
    TEST_ASSERT_EQUAL(RASTER_RC_OK, label_api_draw(&test_label, 60U, 5U, rect_sink));
    TEST_ASSERT_EQUAL(RASTER_RC_OK, label_api_draw_masked(&test_label, 60U, 5U, &mask, mask_sink));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(rect_map, mask_map, sizeof(rect_map));
}

void test_label_masked_without_text_is_noop(void) {
    const struct RasterMask mask = { .buffer = mask_scratch, .size = TEST_MASK_CAPACITY };
    label_api_init(&test_label, NULL, 0, 0, &mask_font, 10U, FONT_ALIGN_LEFT, text_color);
    TEST_ASSERT_EQUAL(RASTER_RC_OK, label_api_draw_masked(&test_label, 0U, 0U, &mask, mask_sink));
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, label_api_draw_masked(NULL, 0U, 0U, &mask, mask_sink));
    TEST_ASSERT_EQUAL(0U, mask_calls);
}

void test_box_masked_fills_background_and_blends_label(void) {
    const struct RasterMask mask = { .buffer = mask_scratch, .size = TEST_MASK_CAPACITY };
    init_boxes();
    TEST_ASSERT_EQUAL(RASTER_RC_OK, box_api_draw_masked(&test_boxes[0], rect_sink, &mask, mask_sink));
    TEST_ASSERT_EQUAL_MESSAGE(1U, rect_calls, "only the background goes through the rectangle callback");
    TEST_ASSERT_EQUAL_MESSAGE(1U, mask_calls, "the label is one mask blend");
}

void test_box_masked_null_arguments(void) {
    const struct RasterMask mask = { .buffer = mask_scratch, .size = TEST_MASK_CAPACITY };
    init_boxes();
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, box_api_draw_masked(NULL, rect_sink, &mask, mask_sink));
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, box_api_draw_masked(&test_boxes[0], NULL, &mask, mask_sink));
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, box_api_draw_masked(&test_boxes[0], rect_sink, NULL, mask_sink));
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, box_api_draw_masked(&test_boxes[0], rect_sink, &mask, NULL));
}

void test_raster_init_leaves_mask_disabled(void) {
    struct RasterHandler handler;
    memset(&handler, 0xA5, sizeof(handler)); /* stack garbage */
    init_boxes();
    raster_api_init(&handler, test_boxes, 2U, rect_sink, NULL);
    TEST_ASSERT_NULL(handler.draw_mask);
    TEST_ASSERT_NULL(handler.mask.buffer);
}

void test_raster_render_uses_mask_when_set(void) {
    struct RasterHandler handler;
    init_boxes();
    raster_api_init(&handler, test_boxes, 2U, rect_sink, NULL);
    TEST_ASSERT_EQUAL(RASTER_RC_OK, raster_api_set_mask(&handler, mask_sink, mask_scratch, TEST_MASK_CAPACITY));

    TEST_ASSERT_EQUAL(RASTER_RC_OK, raster_api_render(&handler));

    TEST_ASSERT_EQUAL_MESSAGE(2U, rect_calls, "two backgrounds");
    TEST_ASSERT_EQUAL_MESSAGE(1U, mask_calls, "one label");
    TEST_ASSERT_FALSE(test_boxes[0].updated);
}

void test_raster_render_back_to_rectangles_when_mask_cleared(void) {
    struct RasterHandler handler;
    init_boxes();
    raster_api_init(&handler, test_boxes, 2U, rect_sink, NULL);
    raster_api_set_mask(&handler, mask_sink, mask_scratch, TEST_MASK_CAPACITY);
    TEST_ASSERT_EQUAL(RASTER_RC_OK, raster_api_set_mask(&handler, NULL, NULL, 0U));

    raster_api_render(&handler);

    TEST_ASSERT_EQUAL(0U, mask_calls);
    TEST_ASSERT_TRUE_MESSAGE(rect_calls > 2U, "the label goes back to one rectangle per run");
}

void test_raster_set_mask_validation(void) {
    struct RasterHandler handler;
    init_boxes();
    raster_api_init(&handler, test_boxes, 2U, rect_sink, NULL);
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, raster_api_set_mask(NULL, mask_sink, mask_scratch, TEST_MASK_CAPACITY));
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, raster_api_set_mask(&handler, mask_sink, NULL, TEST_MASK_CAPACITY));
    TEST_ASSERT_EQUAL(RASTER_RC_NULL_POINTER, raster_api_set_mask(&handler, mask_sink, mask_scratch, 0U));
    TEST_ASSERT_NULL_MESSAGE(handler.draw_mask, "a rejected call must not enable the mask");
}

/*! \} */

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_masked_matches_rectangles_native_size);
    RUN_TEST(test_masked_matches_rectangles_fractional_scale_centered);
    RUN_TEST(test_masked_matches_rectangles_right_aligned_upscaled);
    RUN_TEST(test_masked_bands_when_mask_is_small);
    RUN_TEST(test_masked_blends_with_the_text_color);
    RUN_TEST(test_masked_rejects_mask_narrower_than_text);
    RUN_TEST(test_masked_empty_or_unknown_text_draws_nothing);
    RUN_TEST(test_masked_propagates_callback_error);
    RUN_TEST(test_masked_null_arguments);

    RUN_TEST(test_bounds_width_and_alignment);
    RUN_TEST(test_bounds_height_is_tallest_scaled_glyph);
    RUN_TEST(test_bounds_empty_text);
    RUN_TEST(test_bounds_null_arguments);

    RUN_TEST(test_label_masked_matches_label_rectangles);
    RUN_TEST(test_label_masked_without_text_is_noop);
    RUN_TEST(test_box_masked_fills_background_and_blends_label);
    RUN_TEST(test_box_masked_null_arguments);
    RUN_TEST(test_raster_init_leaves_mask_disabled);
    RUN_TEST(test_raster_render_uses_mask_when_set);
    RUN_TEST(test_raster_render_back_to_rectangles_when_mask_cleared);
    RUN_TEST(test_raster_set_mask_validation);

    return UNITY_END();
}
