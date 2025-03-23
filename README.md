# LIBRASTER

This library is made to rasterize an interface defined by the user using only callbacks.

## Why callbacks?

The idea is to let the user decide how things are done. This helps with performances based on the platform, as using 2 for loops for drawing a square isn't always the best option. This library does not use dynamic allocation to make sure it stays compatible with every platform.

> [!TIP]
> On an STM32 you may want to use `DMA2D`, while using SDL2 you may want to use the function `SDL_FillRect` or just 2 for loops.

## Usage

To use this library all you have to do is include `libraster-api.h` in your program, declare the interface and call the function `render_interface`, that uses **your** functions to draw the interface.

> [!IMPORTANT]
> If you're using 2 buffers, you still have to swap them yourself. This library still does not implement a way to do it.

```c
Threshold ranges[] = {
    {0.0f, 50.0f, 0x00FF00, 0x000000},
    {50.1f, 100.0f, 0xFFFF00, 0x000000},
    {100.1f, 200.0f, 0xFF0000, 0xFFFFFF}
};

Thresholds thresholds[] = {
    {ranges, 3}
};

Label l1;
create_label(&l1, "XD", (Coords){310, 95}, KONEXY, 40, FONT_ALIGN_CENTER);
Value v1;
create_value(&v1, 51, false, (Coords){140, 80}, KONEXY, 70, FONT_ALIGN_CENTER, (union Colors){ .thresholds = thresholds}, THRESHOLDS);

Value v2;
create_value(&v2, 51, true, (Coords){ 196, 80 }, KONEXY, 70, FONT_ALIGN_CENTER, (union Colors){ .slider = (struct Slider){0xff00ff00, ANCHOR_BOTTOM, 0, 200, 3}}, SLIDER);

Label l2;
create_label(&l2, "PROVA", (Coords){196, 80}, KONEXY, 70, FONT_ALIGN_CENTER);

Value v3;
create_value(&v3, 51.0, true, (Coords){ 196, 80 }, KONEXY, 70, FONT_ALIGN_CENTER, (union Colors){ .interpolation = (struct LinearInterpolation){0xff000000, 0xff00ff00, 0.0, 200.0}}, INTERPOLATION);

Box boxes[] = {
    { 1, 0x1, { 2, 2, 397, 237 }, 0xff000000, 0xffffffff, &l1, &v1 },
    { 1, 0x2, { 401, 2, 397, 237 }, 0xff000000, 0xffffffff, NULL, &v2 },
    { 1, 0x3, { 2, 241, 397, 237 }, 0xff000000, 0xffffffff, &l2, NULL },
    { 1, 0x4, { 401, 241, 397, 237 }, 0xff000000, 0xffffffff, NULL, &v3 }
};
```

This is a simple but complete interface, and to render it just call `render_interface` like this:
```c
render_interface(boxes, 4, draw_line_callback, draw_rectangle_callback);
```

> [!TIP]
> There is an option called `GRAPHIC_OPT` inside `libraster.h` that changes how the library behaves (and some parameters in functions an structs).
> This is useful on low power devices to greatly increase performances, but it may create visual artifacts in complex interfaces with overlapping boxes, so use with caution.
