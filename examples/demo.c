#include <stdint.h>
#include <stdlib.h>
#include "libraster-api.h"

// here we include this as well becase we use some utilities provided by this file
#include "fontutils-api.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 480

uint32_t *framebuffer;

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
    framebuffer = (uint32_t *) malloc(sizeof(uint32_t) * WINDOW_WIDTH * WINDOW_HEIGHT);

    Threshold ranges[] = {
        { 0.0f, 50.0f, 0x00FF00, 0x000000 },
        { 50.1f, 100.0f, 0xFFFF00, 0x000000 },
        { 100.1f, 200.0f, 0xFF0000, 0xFFFFFF }
    };

    Thresholds thresholds[] = {
        { ranges, 3 }
    };

    Label l1;
    create_label(&l1, "XD", (Coords){ 310, 95 }, KONEXY_120, 40, FONT_ALIGN_CENTER);
    Value v1;
    create_value(&v1, 51, false, (Coords){ 140, 80 }, KONEXY_120, 70, FONT_ALIGN_CENTER, (Colors){ .thresholds = thresholds }, THRESHOLDS);

    Value v2;
    create_value(&v2, 51, true, (Coords){ 196, 80 }, KONEXY_120, 70, FONT_ALIGN_CENTER, (Colors){ .slider = (Slider){ 0xff00ff00, ANCHOR_BOTTOM, 0, 200, 3 } }, SLIDER);

    Label l2;
    create_label(&l2, "PROVA", (Coords){ 196, 80 }, KONEXY_120, 70, FONT_ALIGN_CENTER);

    Value v3;
    create_value(&v3, 51.0, true, (Coords){ 196, 80 }, KONEXY_120, 70, FONT_ALIGN_CENTER, (Colors){ .interpolation = (LinearInterpolation){ 0xff000000, 0xff00ff00, 0.0, 200.0 } }, INTERPOLATION);

    Box boxes[] = {
        { 1, 0x1, { 2, 2, 397, 237 }, 0xff000000, 0xffffffff, &l1, &v1 },
        { 1, 0x2, { 401, 2, 397, 237 }, 0xff000000, 0xffffffff, NULL, &v2 },
        { 1, 0x3, { 2, 241, 397, 237 }, 0xff000000, 0xffffffff, &l2, NULL },
        { 1, 0x4, { 401, 241, 397, 237 }, 0xff000000, 0xffffffff, NULL, &v3 }
    };

    render_interface(boxes, 4, draw_line_callback, draw_rectangle_callback);

    free(framebuffer);

    return 0;
}
