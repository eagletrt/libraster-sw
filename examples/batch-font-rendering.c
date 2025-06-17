
#include <stdint.h>
#include <stdlib.h>
#include "libraster-api.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 480

uint32_t framebuffer[WINDOW_HEIGHT * WINDOW_WIDTH];

void clear_screen_callback() {
    for (int i = 0; i < WINDOW_WIDTH * WINDOW_HEIGHT; ++i)
        framebuffer[i] = 0x00000000;
}

void batch_glyphs_callback(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color, uint8_t *alphas) {
    if (y + h >= WINDOW_HEIGHT || x + w >= WINDOW_WIDTH || y < 0 || x < 0)
        return;

    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;

    for (int row = 0; row < h; row++) {
        uint32_t *dst_row = framebuffer + (y + row) * WINDOW_WIDTH + x;
        uint8_t *alphas_row = alphas + row * w;

        for (int col = 0; col < w; col++) {
            uint8_t a = alphas_row[col];
            uint32_t *dst = dst_row + col;

            uint8_t dr = (*dst >> 16) & 0xFF;
            uint8_t dg = (*dst >> 8) & 0xFF;
            uint8_t db = *dst & 0xFF;

            uint8_t out_r = (r * a + dr * (255 - a)) / 255;
            uint8_t out_g = (g * a + dg * (255 - a)) / 255;
            uint8_t out_b = (b * a + db * (255 - a)) / 255;

            *dst = (0xFF << 24) | (out_r << 16) | (out_g << 8) | out_b;
        }
    }
}

void draw_rectangle_callback(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color) {
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            framebuffer[(j + x) + WINDOW_WIDTH * (i + y)] = color;
        }
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
    create_label(&l1, "XD", (Coords){ 310, 95 }, FONT_KONEXY, 40, FONT_ALIGN_CENTER);
    Value v1;
    create_value(&v1, 51, false, (Coords){ 140, 80 }, FONT_KONEXY, 70, FONT_ALIGN_CENTER, (Colors){ .thresholds = thresholds }, THRESHOLDS);

    Value v2;
    create_value(&v2, 51, true, (Coords){ 196, 80 }, FONT_KONEXY, 70, FONT_ALIGN_CENTER, (Colors){ .slider = (Slider){ 0xff00ff00, ANCHOR_BOTTOM, 0, 200, 3 } }, SLIDER);

    Label l2;
    create_label(&l2, "PROVA", (Coords){ 196, 80 }, FONT_KONEXY, 70, FONT_ALIGN_CENTER);

    Value v3;
    create_value(&v3, 51.0, true, (Coords){ 196, 80 }, FONT_KONEXY, 70, FONT_ALIGN_CENTER, (Colors){ .interpolation = (LinearInterpolation){ 0xff000000, 0xff00ff00, 0.0, 200.0 } }, INTERPOLATION);

    Box boxes[] = {
        { 1, 0x1, { 2, 2, 397, 237 }, 0xff000000, 0xffffffff, &l1, &v1 },
        { 1, 0x2, { 401, 2, 397, 237 }, 0xff000000, 0xffffffff, NULL, &v2 },
        { 1, 0x3, { 2, 241, 397, 237 }, 0xff000000, 0xffffffff, &l2, NULL },
        { 1, 0x4, { 401, 241, 397, 237 }, 0xff000000, 0xffffffff, NULL, &v3 }
    };

    uint8_t a[100*100];
    render_interface(boxes, 4, batch_glyphs_callback, a, draw_rectangle_callback, clear_screen_callback);

    return 0;
}
