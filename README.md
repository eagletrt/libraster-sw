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
All you have to do is include `raster-api.h` in your program, create labels and boxes, then initialize a handler with the interface, and call the function `raster_api_render` with the handler.

> [!IMPORTANT]
> If you're using 2 buffers, you still have to swap them yourself.

#### Callback Functions

You need to implement three callback functions:

```c
// Draw a horizontal line of pixels
void draw_line_callback(uint16_t x, uint16_t y, uint16_t lenght, struct Color color) {
    // Your implementation here
    // This is called for text rendering
}

// Draw a filled rectangle
void draw_rectangle_callback(uint16_t x, uint16_t y, uint16_t w, uint16_t h, struct Color color) {
    // Your implementation here
    // This is called for box backgrounds
}

// Clear the entire screen (only used when PARTIAL_RASTER is disabled)
void clear_screen_callback(void) {
    // Your implementation here
    // Only needed if PARTIAL_RASTER = 0
}
```

#### Label Structure 
Each label (`struct RasterLabel`) contains:
- `data` - Union containing the actual data (string, int, float)
- `type` - Type of data (see Label Data Types)
- `pos` - Position of the label inside the box (x, y)
- `font` - Enum defining the font to use
- `size` - Font size
- `align` - Text alignment (left, center, right)
- `color` - Font color (`Color` structure)

#### Label Data Types

The library supports the following label data types:
- `LABEL_DATA_STRING` - Text string (char*)
- `LABEL_DATA_INT` - Integer value
- `LABEL_DATA_FLOAT_1` - Float with 1 decimal place
- `LABEL_DATA_FLOAT_2` - Float with 2 decimal places
- `LABEL_DATA_FLOAT_3` - Float with 3 decimal places

#### Box Structure

Each box (`struct RasterBox`) contains:
- `updated` - Flag for partial rendering optimization (only if PARTIAL_RASTER is enabled)
- `id` - Unique identifier for the box
- `rect` - Rectangle dimensions (x, y, width, height)
- `color` - Background color (ARGB format)
- `label` - Pointer to label structure (optional, can be NULL)

> [!TIP]
> Create a file `raster_config.h` with the following defines to customize behaviour:
> - `PARTIAL_RASTER` - Enable/disable partial rendering optimization (default = 1). When enabled, only boxes with `updated = true` will be redrawn.
