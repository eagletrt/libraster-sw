/*!
 * \file direct-font-rendering.c
 *
 * \brief Minimal end-to-end example.
 *
 * \details Builds a two-box interface (each with a string label) and renders
 *     it into a software framebuffer using a trivial rectangle-fill callback.
 *
 *     The font `font_konexy` is provided by the user's project. Run the
 *     generator first:
 *
 *         python tools/generator.py --json tools/fonts.json --output build/generated
 *
 *     and add `build/generated/fonts.c` to the build alongside this file.
 */

#include "raster-fonts.h"
#include "raster-api.h"
#include "box-api.h"
#include "label-api.h"
#include <stdint.h>
#include <stddef.h>

#define WINDOW_WIDTH (800U)
#define WINDOW_HEIGHT (480U)

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
    struct Label title;
    label_api_init(&title, "HELLO", 200, 100, &font_konexy, 32, FONT_ALIGN_CENTER, (struct Color){ .argb = 0xFFFFFFFF });
    struct Label value;
    label_api_init(&value, "99", 200, 100, &font_konexy, 32, FONT_ALIGN_CENTER, (struct Color){ .argb = 0xFFFFFFFF });

    struct Box boxes[2];
    box_api_init(&boxes[0], 0x1, (struct BoxRectangle){ 0, 0, 400, 240 }, (struct Color){ .argb = 0xFF000000 }, &title);
    box_api_init(&boxes[1], 0x2, (struct BoxRectangle){ 400, 0, 400, 240 }, (struct Color){ .argb = 0xFF000000 }, &value);

    struct RasterHandler handler;
    raster_api_init(&handler, boxes, sizeof(boxes) / sizeof(boxes[0]), draw_rect, NULL);
    raster_api_render(&handler);

    box_api_set_label_text(&boxes[1], "42");
    raster_api_render(&handler);

    return 0;
}
