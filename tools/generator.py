import argparse
import datetime
import json
import logging
import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont
import numpy as np
from scipy.ndimage import distance_transform_edt
from jinja2 import Environment, FileSystemLoader


SCRIPT_DIR = Path(__file__).resolve().parent

logger = logging.getLogger("libraster-generator")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--json",
        type=Path,
        default=SCRIPT_DIR / "fonts.json",
        help="Path to the fonts.json descriptor.",
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=None,
        help="Directory where raster-fonts.c and raster-fonts.h will be written. "
             "Defaults to the directory containing the JSON file.",
    )
    return parser.parse_args()


def parse_char_set(pattern: str) -> list[str]:
    """Expand patterns like 'A-Za-z0-9 .' into a sorted, deduped list of chars."""
    chars: list[str] = []
    last = None
    i = 0
    while i < len(pattern):
        c = pattern[i]
        if c == '-' and last is not None and i + 1 < len(pattern):
            start = ord(last)
            end = ord(pattern[i + 1])
            if start < end:
                chars.extend(chr(code) for code in range(start + 1, end + 1))
            last = pattern[i + 1]
            chars.append(last)
            i += 2
            continue
        chars.append(c)
        last = c
        i += 1
    return sorted(set(chars))


def smoothstep(edge0: float, edge1: float, x: np.ndarray) -> np.ndarray:
    t = np.clip((x - edge0) / (edge1 - edge0), 0, 1)
    return t * t * (3 - 2 * t)


def compute_sdf(bitmap: np.ndarray, edge0: float, edge1: float) -> np.ndarray:
    if not np.any(bitmap):
        return np.zeros(bitmap.shape, dtype=np.uint8)
    padded = np.pad(bitmap, 1, mode="constant", constant_values=False)
    inside = distance_transform_edt(padded)[1:-1, 1:-1]
    outside = distance_transform_edt(~padded)[1:-1, 1:-1]
    sdf = inside - outside
    normalized = np.clip((sdf + 3) / 6, 0, 1)
    return (smoothstep(edge0, edge1, normalized) * 255).astype(np.uint8)


def compress_rle(data: list[int]) -> list[tuple[int, int]]:
    """Compress an alpha stream into (value, count) pairs. Counts are bounded to 255."""
    out: list[tuple[int, int]] = []
    i = 0
    n = len(data)
    while i < n:
        value = data[i]
        count = 1
        i += 1
        while i < n and data[i] == value and count < 255:
            count += 1
            i += 1
        out.append((value, count))
    return out


def render_glyph(font: ImageFont.FreeTypeFont, char: str, total_height: int, ascent: int) -> tuple[np.ndarray, int, int]:
    """Render a single character into a uint8 bitmap and return (bitmap, w, h)."""
    bbox = font.getbbox(char, anchor="ls")
    width = bbox[2] - bbox[0]
    image = Image.new("L", (width, total_height), 0)
    draw = ImageDraw.Draw(image)
    draw.text((-bbox[0], ascent), char, fill=255, font=font, anchor="ls")
    return np.array(image), width, total_height


def face_for_height(ttf_path: Path, height: int) -> ImageFont.FreeTypeFont:
    """Largest FreeType size whose line height (ascent + descent) fits in height pixels."""
    low, high = 1.0, float(height) * 2.0
    for _ in range(40):
        middle = (low + high) / 2
        ascent, descent = ImageFont.truetype(str(ttf_path), middle).getmetrics()
        if ascent + descent > height:
            high = middle
        else:
            low = middle
    return ImageFont.truetype(str(ttf_path), low)


def build_strike(pil_font: ImageFont.FreeTypeFont, chars: list[str], total_height: int, coverage) -> dict:
    """Render every glyph at one size: RLE stream, glyph metadata, base size."""
    ascent = pil_font.getmetrics()[0]
    sdf_stream: list[int] = []
    glyphs: list[dict] = []

    for char in chars:
        bitmap_8bit, width, height = render_glyph(
            pil_font, char, total_height, ascent)
        pixels = list(coverage(bitmap_8bit).flatten())
        compressed = compress_rle(pixels)

        offset = len(sdf_stream)
        sdf_stream.extend(item for couple in compressed for item in couple)

        glyphs.append({
            "codepoint": ord(char),
            "display": char if char.isprintable() and char != "*/" else f"U+{ord(char):04X}",
            "offset": offset,
            "size": len(compressed) * 2,
            "width": width,
            "height": height,
        })

    return {
        "base_size": total_height,
        "sdfs": sdf_stream,
        "glyphs": glyphs,
    }


def build_font_data(font_json: dict, json_dir: Path) -> dict:
    """Build per-font payload: one strike per size, the largest one first in the Font struct.

    "size" + "edges": one SDF master scaled at draw time (legacy).
    "sizes": the anti-aliased FreeType coverage baked at each pixel height; the
    renderer draws the matching size 1:1 and scales the others down from the
    next larger one.
    """
    ttf_path = json_dir / font_json["font"]
    chars = parse_char_set(font_json["characters"])

    if "sizes" in font_json:
        if "size" in font_json or "edges" in font_json:
            raise ValueError(f"font {font_json['name']}: use either \"sizes\" or \"size\" + \"edges\"")
        heights = sorted(set(int(h) for h in font_json["sizes"]))
        if not heights or heights[0] < 1:
            raise ValueError(f"font {font_json['name']}: \"sizes\" needs positive pixel heights")
        strikes = [build_strike(face_for_height(ttf_path, h), chars, h, lambda bitmap: bitmap) for h in heights]
    else:
        pil_font = ImageFont.truetype(str(ttf_path), font_json["size"])
        ascent, descent = pil_font.getmetrics()
        edge0, edge1 = font_json["edges"]
        strikes = [build_strike(pil_font, chars, ascent + descent, lambda bitmap: compute_sdf(bitmap > 128, edge0, edge1))]

    return {
        "name": font_json["name"],
        "strikes": strikes,
        "baked": "sizes" in font_json,
    }


def render_templates(fonts: list[dict], output_dir: Path) -> None:
    output_dir.mkdir(parents=True, exist_ok=True)
    env = Environment(loader=FileSystemLoader(
        str(SCRIPT_DIR)), keep_trailing_newline=True)
    timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")

    for src_name, dst_name in (("raster-fonts.c.j2", "raster-fonts.c"), ("raster-fonts.h.j2", "raster-fonts.h")):
        template = env.get_template(f"templates/{src_name}")
        rendered = template.render(timestamp=timestamp, fonts=fonts)
        out_path = output_dir / dst_name
        out_path.write_text(rendered)
        logger.info("wrote %s", out_path)


def main() -> int:
    logging.basicConfig(level=logging.INFO, format="[generator] %(message)s")
    args = parse_args()

    json_path: Path = args.json
    if not json_path.is_file():
        logger.error("fonts.json not found at %s", json_path)
        return 1

    output_dir: Path = args.output if args.output is not None else json_path.parent

    with json_path.open("r", encoding="utf-8") as fp:
        descriptor = json.load(fp)

    fonts = [build_font_data(entry, json_path.parent) for entry in descriptor]

    render_templates(fonts, output_dir)
    logger.info("done")
    return 0


if __name__ == "__main__":
    sys.exit(main())
