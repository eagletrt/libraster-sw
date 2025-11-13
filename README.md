# LIBRASTER

This library is made to rasterize an interface defined by the user using only callbacks.

## Why callbacks?

The idea is to let the user decide how things are done. This helps with performances based on the platform, as using 2 for loops for drawing a square isn't always the best option. This library does not use dynamic allocation to make sure it stays compatible with every platform.

> [!TIP]
> On an STM32 you may want to use `DMA2D`, while using SDL2 you may want to use the function `SDL_FillRect` or just 2 for loops.

## Usage

### First of all you need to generate the actual font. There are 2 main cases:
1. **Using PlatformIO package manager**
    You will need to add a script as such:
    ```python
    import os
    import hashlib
    import sys
    from pathlib import Path
    from SCons.Script import Import
    import logging

    Import("env")

    json_path = Path("tools/fonts.json")
    hash_path = Path("tools/.fonts.json.sha256")
    libgen = next(Path(".pio").rglob("generator.py"))


    def hash_file(p):
        return hashlib.sha256(p.read_bytes()).hexdigest()

    logger = logging.getLogger("libraster")
    logging.basicConfig(encoding='utf-8', level=logging.INFO)

    if not json_path.exists():
        logger.warning("[libraster] fonts.json not found")
        exit(1)
    elif not hash_path.exists() or hash_file(json_path) != hash_path.read_text():
        logger.info("[libraster] generating")
        os.system(f"{sys.executable} {libgen} --json {json_path}")
        hash_path.write_text(hash_file(json_path))
    else:
        logger.warning("[libraster] fonts.json unchanged, skipping")

    ```
    This will let you create a folder named `tools` containing the `fonts.json`.
    Then in your `platformio.ini` just call the script on-build like this:
    ```
    extra_scripts = pre:tools/generate_fonts.py
    ```

2. **Using the library without PlatformIO package manager**
    You will still need to create the `fonts.json`, but after that everything is on you. You will decide how and when to call the generator. You can pass the path to `fonts.json` using the `--json` argument.

### Actual usage
All you have to do is include `libraster-api.h` in your program, declare the interface and call the function `render_interface`, that uses **your** functions to draw the interface.

> [!IMPORTANT]
> If you're using 2 buffers, you still have to swap them yourself. This library still does not implement a way to do it.

```c
struct Threshold ranges[] = {
    {0.0f, 50.0f, 0x00FF00, 0x000000},
    {50.1f, 100.0f, 0xFFFF00, 0x000000},
    {100.1f, 200.0f, 0xFF0000, 0xFFFFFF}
};

struct Thresholds thresholds[] = {
    {ranges, 3}
};

struct Label l1;
create_label(&l1, "XD", (struct Coords){310, 95}, FONT_KONEXY, 40, FONT_ALIGN_CENTER);
struct Value v1;
create_value(&v1, 51, false, (struct Coords){140, 80}, FONT_KONEXY, 70, FONT_ALIGN_CENTER, (union Colors){ .thresholds = thresholds}, THRESHOLDS);

struct Value v2;
create_value(&v2, 51, true, (struct Coords){ 196, 80 }, FONT_KONEXY, 70, FONT_ALIGN_CENTER, (union Colors){ .slider = (struct Slider){0xff00ff00, ANCHOR_BOTTOM, 0, 200, 3}}, SLIDER);

struct Label l2;
create_label(&l2, "PROVA", (struct Coords){196, 80}, FONT_KONEXY, 70, FONT_ALIGN_CENTER);

struct Value v3;
create_value(&v3, 51.0, true, (struct Coords){ 196, 80 }, FONT_KONEXY, 70, FONT_ALIGN_CENTER, (union Colors){ .interpolation = (struct LinearInterpolation){0xff000000, 0xff00ff00, 0.0, 200.0}}, INTERPOLATION);

struct Box boxes[] = {
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
> Create a file `raster_config.h` with the following defines to customize behaviour:
> - `PARTIAL_RASTER` that changes how the library behaves (default = 1).

