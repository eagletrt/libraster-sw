import os
from PIL import Image, ImageDraw, ImageFont
import numpy as np
from scipy.ndimage import distance_transform_edt
import json
import datetime
import argparse
from pathlib import Path
from jinja2 import Environment, FileSystemLoader
import logging


parser = argparse.ArgumentParser()
parser.add_argument("--json", type=Path, default=Path(__file__).parent
                    / "fonts.json")
args = parser.parse_args()


SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
json_base = args.json.parent


def compress_rle_4bit_paired(data):
    compressed = []
    i = 0
    while i < len(data):
        sdf1 = data[i] // 16
        count1 = 1
        i += 1
        while i < len(data) and data[i] // 16 == sdf1 and count1 < 255:
            count1 += 1
            i += 1
        if i < len(data):
            sdf2 = data[i] // 16
            count2 = 1
            i += 1
            while i < len(data) and data[i] // 16 == sdf2 and count2 < 255:
                count2 += 1
                i += 1
        else:
            count2 = 0
        compressed.append(((sdf1 << 4) | sdf2, count1, count2))
    return compressed


def generate_bitmaps(fonts):
    def smoothstep(edge0, edge1, x):
        t = np.clip((x - edge0) / (edge1 - edge0), 0, 1)
        return t * t * (3 - 2 * t)

    def compute_sdf(bitmap, edge0, edge1):
        inside = distance_transform_edt(bitmap)
        outside = distance_transform_edt(1 - bitmap)
        sdf = inside - outside
        normalized_sdf = np.clip((sdf + 3) / 6, 0, 1)
        normalized_sdf = smoothstep(edge0, edge1, normalized_sdf) * 255
        return normalized_sdf.astype(np.uint8)

    def parse_char_set(pattern: str) -> list[str]:
        chars = []
        last = None

        i = 0
        while i < len(pattern):
            c = pattern[i]

            if c == '-' and last is not None and i + 1 < len(pattern):
                start = ord(last)
                end = ord(pattern[i + 1])

                if start < end:
                    chars.extend(chr(code) for code in
                                 range(start + 1, end + 1))

                last = pattern[i + 1]
                chars.append(last)
                i += 2
                continue
            else:
                chars.append(c)
                last = c

            i += 1

        return sorted(set(chars))

    sdf_datas = []
    glyph_metadatas = []

    for font_json in fonts:
        font = ImageFont.truetype(os.path.join(json_base, font_json["font"]),
                                  font_json["size"])

        ascent, descent = font.getmetrics()
        total_height = ascent + descent

        font_sdf_data = []
        font_glyph_metadata = []

        characters = parse_char_set(font_json["characters"])

        for char in characters:
            bbox = font.getbbox(char, anchor="ls")
            char_width = bbox[2] - bbox[0]

            image = Image.new("L", (char_width, total_height), 0)
            draw = ImageDraw.Draw(image)

            x_offset = -bbox[0]
            y_offset = ascent
            draw.text((x_offset, y_offset), char, fill=255,
                      font=font, anchor="ls")

            bitmap = np.array(image) > 128
            sdf = compute_sdf(bitmap, font_json["edges"][0],
                              font_json["edges"][1])
            sdf_image = Image.fromarray(sdf)

            width, height = sdf_image.size
            pixels = list(sdf_image.getdata())
            compressed = compress_rle_4bit_paired(pixels)
            offset = len(font_sdf_data)

            # Append SDF data
            font_sdf_data.extend([item for pair in compressed
                                  for item in pair])
            if char == '\'':
                char = '\\' + char
            font_glyph_metadata.append(
                (offset, len(compressed) * 3, width, height, char))

        sdf_datas.append(font_sdf_data)
        glyph_metadatas.append(font_glyph_metadata)

    return sdf_datas, glyph_metadatas


def generate_c_files(fonts):
    c_file_path = os.path.join(SCRIPT_DIR, "..", "src", "fonts.c")
    h_file_path = os.path.join(SCRIPT_DIR, "..", "include", "fonts.h")

    datetime_info = datetime.datetime.today().strftime("%H:%M:%S  %d/%m/%Y")

    env = Environment(loader=FileSystemLoader(SCRIPT_DIR))
    c_template = env.get_template("templates/fonts.c.j2")
    rendered = c_template.render(datetime_info=datetime_info, fonts=fonts)
    with open(c_file_path, "w") as c_file:
        c_file.write(rendered)

    h_template = env.get_template("templates/fonts.h.j2")
    rendered = h_template.render(datetime_info=datetime_info, fonts=fonts)
    with open(h_file_path, "w") as h_file:
        h_file.write(rendered)


def main():
    logger = logging.getLogger("font-generator")
    logging.basicConfig(encoding='utf-8', level=logging.INFO)

    with open(os.path.join(json_base, "fonts.json")) as json_data:
        fonts = json.load(json_data)

        logger.info("bitmap generation")
        sdfs, glyphs = generate_bitmaps(fonts)
        for i, font in enumerate(fonts):
            font["sdfs"] = sdfs[i]
            font["glyphs"] = [
                {
                    "offset": g[0],
                    "size": g[1],
                    "width": g[2],
                    "height": g[3],
                    "char": g[4]
                }
                for g in glyphs[i]
            ]

        logger.info("C and H generation")
        generate_c_files(fonts)

        logger.info("ok")


if __name__ == "__main__":
    main()
