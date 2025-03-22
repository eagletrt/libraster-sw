import os
from PIL import Image, ImageDraw, ImageFont
import numpy as np
from scipy.ndimage import distance_transform_edt
import json


SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
fonts_json_path = os.path.join(SCRIPT_DIR, "fonts.json")


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


def generate_bitmaps_fixed(fonts):
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

    sdf_datas = []
    glyph_metadatas = []

    for font_json in fonts:
        font = ImageFont.truetype(os.path.join(SCRIPT_DIR, font_json["font"]),
                                  font_json["size"])

        ascent, descent = font.getmetrics()
        total_height = ascent + descent

        font_sdf_data = []
        font_glyph_metadata = []

        for char_code in range(32, 127):
            char = chr(char_code)

            bbox = font.getbbox(char, anchor="ls")  # "ls" = left baseline
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
            font_sdf_data.extend([item for pair in compressed for item in pair])
            font_glyph_metadata.append(
                (offset, len(compressed) * 2, width, height))

        sdf_datas.append(font_sdf_data)
        glyph_metadatas.append(font_glyph_metadata)

    return sdf_datas, glyph_metadatas


def generate_c_files(fonts, sdf_datas, glyph_metadatas):
    c_file_path = os.path.join(SCRIPT_DIR, "..", "src", "fonts.c")
    h_file_path = os.path.join(SCRIPT_DIR, "..", "include", "fonts.h")

    font_entries = []
    enum_entries = []

    for f in fonts:
        font_name = f["name"]
        font_size = f["size"]
        font_entries.append(
            f"    {{ {font_size}, sdf_data_{font_name}, glyphs_{font_name} }},")
        enum_entries.append(f"    {font_name.upper()},\n")

    with open(c_file_path, "w") as c_file, open(h_file_path, "w") as h_file:
        # Header file content
        h_file.write("#ifndef FONT_H\n#define FONT_H\n\n#include <stdint.h>\n\n")
        h_file.write("typedef struct {\n")
        h_file.write("    uint32_t offset;\n")
        h_file.write("    uint16_t size;\n")
        h_file.write("    uint16_t width;\n")
        h_file.write("    uint16_t height;\n")
        h_file.write("} Glyph;\n\n")

        for font_json in fonts:
            name = font_json["name"]
            h_file.write(f"extern const uint8_t sdf_data_{name}[];\n")
            h_file.write(f"extern const Glyph glyphs_{name}[];\n\n")
        h_file.write("typedef struct {\n")
        h_file.write("    uint8_t size;\n")
        h_file.write("    const uint8_t* sdf_data;\n")
        h_file.write("    const Glyph* glyphs;\n")
        h_file.write("} Font;\n\n")

        h_file.write("typedef enum {\n")
        h_file.write("".join(enum_entries))
        h_file.write("} FontName;\n\n")

        h_file.write("static const Font fonts[] = {\n")
        h_file.write("\n".join(font_entries) + "\n")
        h_file.write("};\n\n")
        h_file.write("#endif // FONT_H\n")

        c_file.write("#include \"fonts.h\"\n\n")

        for j, sdf_data in enumerate(sdf_datas):
            name = fonts[j]["name"]
            c_file.write(f"const uint8_t sdf_data_{name}[] = {{\n")
            for i, value in enumerate(sdf_data):
                c_file.write(f"{value}, ")
                if (i + 1) % 12 == 0:
                    c_file.write("\n")
            c_file.write("};\n\n")

            c_file.write(f"const Glyph glyphs_{name}[] = {{\n")
            for offset, size, width, height in glyph_metadatas[j]:
                c_file.write(f"    {{ {offset}, {size}, {width}, {height} }},\n")
            c_file.write("};\n")


def main():
    with open(fonts_json_path) as json_data:
        fonts = json.load(json_data)
        json_data.close()

        print("Bitmap generation...")
        sdf_datas, glyph_metadatas = generate_bitmaps_fixed(fonts)

        print("C and H files generation...")
        generate_c_files(fonts, sdf_datas, glyph_metadatas)

        print("Process completed!")


if __name__ == "__main__":
    main()
