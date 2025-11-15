#include <stdint.h>
#include <stdlib.h>
#include "raster-api.h"

#define WINDOW_WIDTH 50
#define WINDOW_HEIGHT 30

uint32_t framebuffer[WINDOW_HEIGHT * WINDOW_WIDTH];

void draw_line_cb(uint16_t x, uint16_t y, uint16_t lenght, struct Color color) {
    for (int i = 0; i < lenght; i++) {
        framebuffer[y * WINDOW_WIDTH + x] = color.argb;
    }
}

void draw_rectangle_cb(uint16_t x, uint16_t y, uint16_t w, uint16_t h, struct Color color) {
    for (int i = 0; i < h; i++) {
        draw_line_cb(x, y + i, w, color);
    }
}

int main() {
    struct Label l1;
    raster_api_create_label(&l1, (union LabelData){ .text = "XD" }, LABEL_DATA_STRING, (struct Coords){ 0, 0 }, FONT_KONEXY, 10, FONT_ALIGN_CENTER, (struct Color){ .argb = 0xffffffff });
    struct Label v1;
    raster_api_create_label(&v1, (union LabelData){ .int_val = 51 }, LABEL_DATA_INT, (struct Coords){ 10, 0 }, FONT_KONEXY, 10, FONT_ALIGN_CENTER, (struct Color){ .argb = 0xffffffff });

    struct Box boxes[] = {
        { true, 0x1, { 2, 2, 397, 237 }, {.argb = 0xff000000}, &l1 },
        { true, 0x2, { 400, 2, 397, 237 }, {.argb = 0xff000000}, &v1 },
    };

    raster_api_render(boxes, 2, draw_line_cb, draw_rectangle_cb, NULL);
    return 0;
}
