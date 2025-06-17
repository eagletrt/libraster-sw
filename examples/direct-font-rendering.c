#include <stdint.h>
#include <stdlib.h>
#include "libraster-api.h"

#define WINDOW_WIDTH 50
#define WINDOW_HEIGHT 30

uint32_t framebuffer[WINDOW_HEIGHT * WINDOW_WIDTH];

void draw_line_callback(uint16_t x, uint16_t y, uint16_t lenght, uint32_t color) {
    for (int i = 0; i < lenght; i++) {
        framebuffer[y * WINDOW_WIDTH + x] = color;
    }
}

void draw_rectangle_callback(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color) {
    for (int i = 0; i < h; i++) {
        draw_line_callback(x, y + i, w, color);
    }
}

int main() {
    Threshold ranges[] = {
        { 0.0f, 50.0f, 0x00FF00, 0x000000 },
        { 50.1f, 100.0f, 0xFFFF00, 0x000000 },
        { 100.1f, 200.0f, 0xFF0000, 0xFFFFFF }
    };

    Thresholds thresholds[] = {
        { ranges, 3 }
    };

    Label l1;
    create_label(&l1, "XD", (Coords){ 0, 0 }, FONT_KONEXY, 10, FONT_ALIGN_CENTER);
    Value v1;
    create_value(&v1, 51, false, (Coords){ 10, 0 }, FONT_KONEXY, 10, FONT_ALIGN_CENTER, (Colors){ .thresholds = thresholds }, THRESHOLDS);

    Box boxes[] = {
        { 1, 0x1, { 2, 2, 397, 237 }, 0xff000000, 0xffffffff, &l1, &v1 },
    };

    render_interface(boxes, 1, draw_line_callback, draw_rectangle_callback);

    return 0;
}
