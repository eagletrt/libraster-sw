# LIBRASTER

A small graphics library for embedded UIs. The user describes the screen
as a flat array of boxes (each with a label) and provides one drawing
primitive — libraster takes care of the rest.

## Why callbacks?

Drawing is delegated to the user via a single `raster_draw_rectangle_callback`
so hardware accelerators can be plugged in without changing library code.
The library never allocates dynamically.

> [!TIP]
> On an STM32 you may want to wire the callback to `DMA2D`; with SDL2 you
> can use `SDL_FillRect`; on a raw framebuffer two nested loops are enough.

## Render modes

The render mode is selected at runtime by whether you pass a clear callback
to `raster_api_init`:

- **Partial mode** (`clear == NULL`): only boxes whose `updated` flag is set
  are redrawn each frame. The flag is cleared automatically after a successful
  redraw. This is the common path for low-power UIs.
- **Full-redraw mode** (`clear != NULL`): the clear callback runs first and
  every box is drawn each frame.

There is no `raster-config.h`, no compile-time flag, and no need to
recompile the library to switch modes.

## Fonts

The bundled generator turns one or more TTFs into a pair of `raster-fonts.c`/`raster-fonts.h` 
files that you compile and link with your application. Each font carries its own `find_glyph`
function (a switch-case the compiler can fold into a jump table), so glyph
lookup is a direct call rather than a generic data-driven scan.

### Generating fonts

Create a `fonts.json` describing the fonts you want:

```json
[
    {
        "name": "konexy",
        "font": "KonexyFont.ttf",
        "size": 120,
        "edges": [0.2, 0.5],
        "characters": "A-Za-z0-9 ."
    }
]
```

Run the generator, pointing it at your JSON and an output directory:

```sh
python tools/generator.py --json path/to/fonts.json --output path/to/output
```

This writes `fonts.c` and `fonts.h` into the output directory. Each font
in the JSON becomes an `extern const struct Font font_<name>;` declaration
in the header.

### Wiring into PlatformIO

A typical PlatformIO project keeps the generator hooked to fonts.json via
a pre-build script and adds the output to its source filter:

```ini
build_flags =
    -I tools/generated
build_src_filter = +<*> +<../tools/generated/*>
extra_scripts = pre:tools/generate_fonts.py
```

The pre-build script regenerates only when fonts.json changes.
Here is a sample `generate_fonts.py`:

```python
import hashlib
import logging
import os
import sys
from pathlib import Path

from SCons.Script import Import

Import("env")

JSON_PATH = Path("tools/fonts.json")
HASH_PATH = Path("tools/.fonts.json.sha256")
OUTPUT_DIR = Path("tools/generated")

logging.basicConfig(level=logging.INFO, format="[libraster] %(message)s")
logger = logging.getLogger("libraster")


def hash_file(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main() -> int:
    if not JSON_PATH.exists():
        logger.warning("fonts.json not found at %s", JSON_PATH)
        return 1

    try:
        generator = next(Path(".pio").rglob("generator.py"))
    except StopIteration:
        logger.warning("libraster generator.py not found under .pio/ (LibRaster not installed?)")
        return 1

    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    current_hash = hash_file(JSON_PATH)
    output_files_present = (
        OUTPUT_DIR / "raster-fonts.c").exists() and (OUTPUT_DIR / "raster-fonts.h").exists()
    cache_valid = HASH_PATH.exists() and HASH_PATH.read_text() == current_hash

    if cache_valid and output_files_present:
        logger.info("fonts.json unchanged, skipping")
        return 0

    logger.info("regenerating fonts into %s", OUTPUT_DIR)
    cmd = f"{sys.executable} {generator} --json {JSON_PATH} --output {OUTPUT_DIR}"
    if os.system(cmd) != 0:
        logger.error("font generator failed")
        return 1

    HASH_PATH.write_text(current_hash)
    return 0


if main() != 0:
    sys.exit(1)
```

## Module layout

The library is split into four modules stacked in a single dependency chain:

```
raster -> box -> label -> font
```

| Header                           | Holds                                                      |
|----------------------------------|------------------------------------------------------------|
| `font.h` / `font-api.h`          | Fonts, glyphs, SDF rendering |
| `label.h` / `label-api.h`        | Standalone text element with a font reference              |
| `box.h` / `box-api.h`            | Rectangular region with background color and optional label |
| `raster.h` / `raster-api.h`      | Stateless orchestrator over a flat array of boxes          |

Each module declares its types in the plain header and its functions in the
matching `-api.h`. The raster module holds no state of its own beyond the
user-provided pointers; it only walks the interface and delegates to
`box_api_draw`.

## Usage

Include the API headers you need and the generated `raster-fonts.h`, then
build an interface and render it:

```c
#include "raster-fonts.h"
#include "raster-api.h"
#include "box-api.h"
#include "label-api.h"

static enum RasterReturnCode draw(uint16_t x, uint16_t y, uint16_t w, uint16_t h, struct Color color) {
    /* Fill the rectangle in your framebuffer here. */
    return RASTER_RC_OK;
}

static struct Label speed_label;
static struct Box boxes[1];

int main(void) {
    label_api_init(&speed_label, "100", 100, 100, &font_konexy, 60, FONT_ALIGN_CENTER, (struct Color){ .argb = 0xFFFFFFFF });
    box_api_init(&boxes[0], 0x1, (struct BoxRectangle){ 0, 0, 200, 200 }, (struct Color){ .argb = 0xFF000000 }, &speed_label);

    struct RasterHandler handler;
    raster_api_init(&handler, boxes, 1, draw, NULL); /* partial mode */
    raster_api_render(&handler);

    /* Update the label content and request a redraw next frame. */
    label_api_set_text(&speed_label, "42");
    boxes[0].updated = true;
    raster_api_render(&handler);
    return 0;
}
```

### Manipulating boxes and labels

```c
/* Reposition a box without altering its label content. */
box_api_set_position(&boxes[0], 50, 50);

/* Attach or detach a label. */
box_api_set_label(&boxes[0], &speed_label);
box_api_clear_label(&boxes[0]);

/* Look a box up by id within the interface. */
struct Box *target = box_api_find(boxes, 1, 0x1);

/* Render a label outside any box, at an explicit screen position. */
label_api_draw(&speed_label, 320, 240, draw);
```
