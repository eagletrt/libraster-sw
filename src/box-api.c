/*!
 * \file box-api.c
 * \date 2026-05-27
 * \author Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Box API implementation.
 */

#include "box-api.h"
#include "label-api.h"
#include <stddef.h>

enum RasterReturnCode box_api_init(struct Box *box, uint16_t id, struct BoxRectangle rect, struct Color color, struct Label *label) {
    if (box == NULL) {
        return RASTER_RC_NULL_POINTER;
    }
    box->updated = true;
    box->id = id;
    box->rect = rect;
    box->color = color;
    box->label = label;
    return RASTER_RC_OK;
}

enum RasterReturnCode box_api_set_position(struct Box *box, uint16_t x, uint16_t y) {
    if (box == NULL) {
        return RASTER_RC_NULL_POINTER;
    }
    box->rect.x = x;
    box->rect.y = y;
    box->updated = true;
    return RASTER_RC_OK;
}

enum RasterReturnCode box_api_set_label(struct Box *box, struct Label *label) {
    if (box == NULL) {
        return RASTER_RC_NULL_POINTER;
    }
    box->label = label;
    box->updated = true;
    return RASTER_RC_OK;
}

enum RasterReturnCode box_api_set_label_text(struct Box *box, const char *text) {
    if (box == NULL || box->label == NULL) {
        return RASTER_RC_NULL_POINTER;
    }
    box->updated = true;
    return label_api_set_text(box->label, text);
}

enum RasterReturnCode box_api_clear_label(struct Box *box) {
    if (box == NULL) {
        return RASTER_RC_NULL_POINTER;
    }
    box->label = NULL;
    box->updated = true;
    return RASTER_RC_OK;
}

enum RasterReturnCode box_api_draw(const struct Box *box, raster_draw_rectangle_callback draw) {
    if (box == NULL || draw == NULL) {
        return RASTER_RC_NULL_POINTER;
    }
    if (draw(box->rect.x, box->rect.y, box->rect.width, box->rect.height, box->color) != RASTER_RC_OK) {
        return RASTER_RC_ERROR;
    }
    if (box->label != NULL) {
        const uint16_t lx = (uint16_t)((int32_t)box->rect.x + box->label->offset_x);
        const uint16_t ly = (uint16_t)((int32_t)box->rect.y + box->label->offset_y);
        if (label_api_draw(box->label, lx, ly, draw) != RASTER_RC_OK) {
            return RASTER_RC_ERROR;
        }
    }
    return RASTER_RC_OK;
}

struct Box *box_api_find(struct Box *boxes, uint16_t size, uint16_t id) {
    if (boxes == NULL) {
        return NULL;
    }
    for (uint16_t i = 0; i < size; ++i) {
        if (boxes[i].id == id) {
            return &boxes[i];
        }
    }
    return NULL;
}
