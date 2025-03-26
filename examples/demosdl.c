#include <SDL.h>
#include <stdint.h>
#include "SDL_timer.h"
#include "libraster-api.h"

// here we include this as well becase we use some utilities provided by this file
#include "fontutils-api.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 480

typedef struct
{
    SDL_Window *window;
    SDL_Renderer *renderer;
} SDLContext;

void sdl_clear_screen() {
}

void sdl_draw_line(uint16_t x, uint16_t y, uint16_t lenght, uint32_t color) {
    SDL_SetRenderDrawColor(SDL_GetRenderer(SDL_GetWindowFromID(1)),
                           get_red(color),
                           get_green(color),
                           get_blue(color),
                           get_alpha(color));
    for (int i = 0; i < lenght; i++) {
        SDL_RenderDrawPoint(SDL_GetRenderer(SDL_GetWindowFromID(1)), x + i, y);
    }
}

void sdl_draw_rectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color) {
    SDL_Rect rect = { x, y, w, h };
    SDL_SetRenderDrawColor(SDL_GetRenderer(SDL_GetWindowFromID(1)),
                           get_red(color),
                           get_green(color),
                           get_blue(color),
                           get_alpha(color));
    SDL_RenderFillRect(SDL_GetRenderer(SDL_GetWindowFromID(1)), &rect);
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("SDL_Init Error: %s", SDL_GetError());
        return 1;
    }

    SDLContext sdl_ctx = {
        .window = SDL_CreateWindow("Graphics Demo",
                                   SDL_WINDOWPOS_CENTERED,
                                   SDL_WINDOWPOS_CENTERED,
                                   WINDOW_WIDTH,
                                   WINDOW_HEIGHT,
                                   SDL_WINDOW_SHOWN),
        .renderer = NULL
    };

    if (!sdl_ctx.window) {
        SDL_Log("SDL_CreateWindow Error: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    sdl_ctx.renderer = SDL_CreateRenderer(sdl_ctx.window, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");

    if (!sdl_ctx.renderer) {
        SDL_Log("SDL_CreateRenderer Error: %s", SDL_GetError());
        SDL_DestroyWindow(sdl_ctx.window);
        SDL_Quit();
        return 1;
    }

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

    int running = 1;
    SDL_Event event;
    uint32_t last_time = SDL_GetTicks();
    int dir = 1;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        SDL_SetRenderDrawColor(sdl_ctx.renderer, 255, 255, 255, 255);
        SDL_RenderClear(sdl_ctx.renderer);

        render_interface(boxes, 4, sdl_draw_line, sdl_draw_rectangle);

        if (SDL_GetTicks() - last_time > 60) {
            Box *box = get_box(boxes, 4, 0x1);
            Box *box2 = get_box(boxes, 4, 0x4);
            Box *box3 = get_box(boxes, 4, 0x2);
            box->value->value += dir;
            box2->value->value += dir;
            box3->value->value += dir;
            if (box->value->value > 199)
                dir = -1;
            else if (box->value->value < 2)
                dir = 1;
        }

        SDL_RenderPresent(sdl_ctx.renderer);

        SDL_Delay(16);
    }

    SDL_DestroyRenderer(sdl_ctx.renderer);
    SDL_DestroyWindow(sdl_ctx.window);
    SDL_Quit();

    return 0;
}
