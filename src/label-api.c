/*!
 * \file label-api.c
 * \date 2026-05-27
 * \author Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Label API implementation.
 */

#include "label-api.h"
#include "font-api.h"
#include <stddef.h>

enum RasterReturnCode label_api_init(struct Label *label, const char *text, int16_t offset_x, int16_t offset_y, const struct Font *font, uint16_t size, enum FontAlignment alignment, struct Color color) {
    if (label == NULL || font == NULL) {
        return RASTER_RC_NULL_POINTER;
    }
    label->text = text;
    label->offset_x = offset_x;
    label->offset_y = offset_y;
    label->font = font;
    label->size = size;
    label->alignment = alignment;
    label->color = color;
    return RASTER_RC_OK;
}

enum RasterReturnCode label_api_set_text(struct Label *label, const char *text) {
    if (label == NULL) {
        return RASTER_RC_NULL_POINTER;
    }
    label->text = text;
    return RASTER_RC_OK;
}

enum RasterReturnCode label_api_draw(const struct Label *label, uint16_t x, uint16_t y, raster_draw_rectangle_callback draw) {
    if (label == NULL || draw == NULL) {
        return RASTER_RC_NULL_POINTER;
    }
    if (label->text == NULL || label->font == NULL) {
        return RASTER_RC_OK;
    }
    return font_api_draw(x, y, label->alignment, label->font, label->text, label->color, label->size, draw);
}
