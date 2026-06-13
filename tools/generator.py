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
    inside = distance_transform_edt(bitmap)
    outside = distance_transform_edt(1 - bitmap)
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


def build_font_data(font_json: dict, json_dir: Path) -> dict:
    """Build per-font payload (sdf bytes, glyph metadata, base size)."""
    ttf_path = json_dir / font_json["font"]
    pil_font = ImageFont.truetype(str(ttf_path), font_json["size"])

    ascent, descent = pil_font.getmetrics()
    total_height = ascent + descent

    chars = parse_char_set(font_json["characters"])
    edge0, edge1 = font_json["edges"]

    sdf_stream: list[int] = []
    glyphs: list[dict] = []

    for char in chars:
        bitmap_8bit, width, height = render_glyph(
            pil_font, char, total_height, ascent)
        bitmap = bitmap_8bit > 128
        sdf = compute_sdf(bitmap, edge0, edge1)
        pixels = list(sdf.flatten())
        compressed = compress_rle(pixels)

        offset = len(sdf_stream)
        sdf_stream.extend(item for couple in compressed for item in couple)

        # Escape characters that would break the C literal.
        if char == "'" or char == '\\':
            char_literal = "\\" + char
        else:
            char_literal = char

        glyphs.append({
            "char": char_literal,
            "offset": offset,
            "size": len(compressed) * 2,
            "width": width,
            "height": height,
        })

    return {
        "name": font_json["name"],
        "base_size": total_height,
        "sdfs": sdf_stream,
        "glyphs": glyphs,
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
