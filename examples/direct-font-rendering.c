/*!
 * \file direct-font-rendering.c
 *
 * \brief Minimal end-to-end example.
 *
 * \details Builds a two-box interface (one with a string label, one with an
 *     integer label) and renders it into a software framebuffer using a
 *     trivial rectangle-fill callback.
 *
 *     The font `font_konexy` is provided by the user's project. Run the
 *     generator first:
 *
 *         python tools/generator.py --json tools/fonts.json --output build/generated
 *
 *     and add `build/generated/fonts.c` to the build alongside this file.
 */

#include "fonts.h"
#include "raster-api.h"
#include <stdint.h>

#define WINDOW_WIDTH (800u)
#define WINDOW_HEIGHT (480u)

static uint32_t framebuffer[WINDOW_HEIGHT * WINDOW_WIDTH];

static enum RasterReturnCode draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, struct Color color) {
    for (uint16_t row = 0; row < h; ++row) {
        const uint32_t base = (uint32_t)(y + row) * WINDOW_WIDTH + x;
        for (uint16_t col = 0; col < w; ++col) {
            framebuffer[base + col] = color.argb;
        }
    }
    return RASTER_RC_OK;
}

int main(void) {
    struct RasterLabel title = {
        .type = LABEL_DATA_STRING,
        .data.text = "HELLO",
        .format.string_fmt = { .max_length = 0 },
        .pos = { .x = 200, .y = 100 },
        .font = &font_konexy,
        .size = 32,
        .align = FONT_ALIGN_CENTER,
        .color = { .argb = 0xFFFFFFFF },
    };
    struct RasterLabel value = {
        .type = LABEL_DATA_INT,
        .data.int_val = 51,
        .format.int_fmt = { .is_unsigned = false },
        .pos = { .x = 200, .y = 100 },
        .font = &font_konexy,
        .size = 32,
        .align = FONT_ALIGN_CENTER,
        .color = { .argb = 0xFFFFFFFF },
    };

    struct RasterBox boxes[] = {
        { .updated = true, .id = 0x1, .rect = { 0, 0, 400, 240 }, .color = { .argb = 0xFF000000 }, .label = &title },
        { .updated = true, .id = 0x2, .rect = { 400, 0, 400, 240 }, .color = { .argb = 0xFF000000 }, .label = &value },
    };

    struct RasterHandler handler;
    raster_api_init(&handler, boxes, sizeof(boxes) / sizeof(boxes[0]), draw_rect, NULL);
    raster_api_render(&handler);

    raster_api_set_label_int(&boxes[1], 99);
    raster_api_render(&handler);

    return 0;
}
