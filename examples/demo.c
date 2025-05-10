#include <stdint.h>
#include <stdlib.h>
#include "libraster-api.h"

// here we include this as well becase we use some utilities provided by this file
#include "fontutils-api.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 480

#define W_MAX 120
uint32_t *fb1;
uint32_t *fb2;
uint32_t *curr;
uint32_t *framebuffer;

void draw_line_fn(uint16_t x, uint16_t y, uint16_t lenght, uint32_t color) {
    for (int i = 0; i < lenght; i++) {
        curr[y * WINDOW_WIDTH + x + i] = color;
    }
}

void draw_rectangle_fn(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color) {
    for(int i=y; i<y+h; i++) {
        draw_line_fn(x, i, w, color);
    }
}

void copy_sprite_fn(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            uint32_t color = curr[j * W_MAX + i];
            curr[j * W_MAX + i] = 0;
            framebuffer[(j + y) * WINDOW_WIDTH + (i + x)] = color;
        }
    }
    curr = curr == fb1 ? fb2 : fb1;
}

int main() {
    framebuffer = (uint32_t *)malloc(sizeof(uint32_t) * WINDOW_WIDTH * WINDOW_HEIGHT);
    fb1 = (uint32_t *)malloc(sizeof(uint32_t) * W_MAX * W_MAX);
    fb2 = (uint32_t *)malloc(sizeof(uint32_t) * W_MAX * W_MAX);
    curr = fb1;

    Threshold ranges[] = {
        { 0.0f, 50.0f, 0xff00ff00, 0xff000000 },
        { 50.1f, 100.0f, 0xffffff00, 0xff000000 },
        { 100.1f, 200.0f, 0xffff0000, 0xffffffff }
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

    render_interface(boxes, 4, draw_line_fn, draw_rectangle_fn, copy_sprite_fn);

    free(framebuffer);
    free(fb1);
    free(fb2);

    return 0;
}
