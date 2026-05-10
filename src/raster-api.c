/*!
 * \file raster-api.c
 * \date 2024-12-13
 * \author Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief LibRaster public API implementation.
 *
 * \details The render mode (partial vs. full) is decided at runtime by the
 *     presence of a clear callback on the handler. No build-time macros
 *     are involved, so the same compiled library object can serve both
 *     modes in a project that switches between them.
 */

#include "raster-api.h"
#include "eagletrt-api.h"
#include "fontutils-api.h"
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

/*!
 * \brief Maximum stack buffer used to format a label's value into text.
 */
#define RASTER_LABEL_BUFFER_SIZE (64u)

/*!
 * \brief Default printf formats for each label type, used when \c format is NULL.
 */
#define RASTER_FMT_INT "%" PRId32
#define RASTER_FMT_UINT "%" PRIu32
#define RASTER_FMT_FLOAT "%.3f"
#define RASTER_FMT_STRING "%s"

/*!
 * \brief Format a label's value into a NUL-terminated buffer.
 *
 * \param[in]  label  Label to format.
 * \param[out] buffer Buffer to write the formatted string into.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_ERROR if the label has an unrecognized type.
 */
EAGLETRT_STATIC enum RasterReturnCode prv_format_label(const struct RasterLabel *label, char *buffer) {
    int32_t snprintf_res = 0;
    const char *fmt = label->format;
    switch (label->type) {
        case RASTER_LABEL_DATA_INT: {
            snprintf_res = snprintf(buffer, RASTER_LABEL_BUFFER_SIZE, fmt ? fmt : RASTER_FMT_INT, label->int32);
            break;
        }
        case RASTER_LABEL_DATA_UINT: {
            snprintf_res = snprintf(buffer, RASTER_LABEL_BUFFER_SIZE, fmt ? fmt : RASTER_FMT_UINT, label->uint32);
            break;
        }

        case RASTER_LABEL_DATA_FLOAT: {
            snprintf_res = snprintf(buffer, RASTER_LABEL_BUFFER_SIZE, fmt ? fmt : RASTER_FMT_FLOAT, label->floating);
            break;
        }

        case RASTER_LABEL_DATA_STRING: {
            snprintf_res = snprintf(buffer, RASTER_LABEL_BUFFER_SIZE, fmt ? fmt : RASTER_FMT_STRING, label->string);
            break;
        }
    }

    if (snprintf_res >= 0 && snprintf_res < (int32_t)RASTER_LABEL_BUFFER_SIZE) {
        return RASTER_RC_OK;
    }

    buffer[0] = '\0';
    return RASTER_RC_ERROR;
}

/*!
 * \brief Render a single box: background plus optional label.
 *
 * \param[in,out] box  Box to render. Its \c updated flag is cleared on
 *     successful redraw when \p clear_updated is true.
 * \param[in]     draw Rectangle-fill callback.
 * \param[in]     clear_updated Whether to clear \c box->updated after a
 *     successful redraw (true in partial mode, false in full-redraw mode).
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_ERROR if a callback reported an error.
 */
EAGLETRT_STATIC enum RasterReturnCode prv_draw_box(struct RasterBox *box, raster_draw_rectangle_callback draw, bool clear_updated) {
    if (draw(box->rect.x, box->rect.y, box->rect.w, box->rect.h, box->color) != RASTER_RC_OK) {
        return RASTER_RC_ERROR;
    }

    if (box->label != NULL && box->label->font != NULL) {
        char buffer[RASTER_LABEL_BUFFER_SIZE] = { 0 };
        if (prv_format_label(box->label, buffer) != RASTER_RC_OK) {
            return RASTER_RC_ERROR;
        }
        const uint16_t lx = (uint16_t)(box->rect.x + box->label->pos.x);
        const uint16_t ly = (uint16_t)(box->rect.y + box->label->pos.y);
        if (font_api_draw(lx, ly, box->label->align, box->label->font, buffer, box->label->color, box->label->size, draw) != RASTER_RC_OK) {
            return RASTER_RC_ERROR;
        }
    }

    if (clear_updated) {
        box->updated = false;
    }
    return RASTER_RC_OK;
}

enum RasterReturnCode raster_api_init(struct RasterHandler *handler, struct RasterBox *interface, uint16_t size, raster_draw_rectangle_callback draw, raster_clear_screen_callback clear) {
    if (handler == NULL || interface == NULL || size == 0u || draw == NULL) {
        return RASTER_RC_NULL_POINTER;
    }
    handler->interface = interface;
    handler->size = size;
    handler->draw = draw;
    handler->clear = clear;
    return RASTER_RC_OK;
}

enum RasterReturnCode raster_api_set_interface(struct RasterHandler *handler, struct RasterBox *interface, uint16_t size) {
    if (handler == NULL || interface == NULL || size == 0u) {
        return RASTER_RC_NULL_POINTER;
    }
    handler->interface = interface;
    handler->size = size;
    return RASTER_RC_OK;
}

enum RasterReturnCode raster_api_render(struct RasterHandler *handler) {
    if (handler == NULL || handler->draw == NULL || handler->interface == NULL) {
        return RASTER_RC_NULL_POINTER;
    }

    const bool full_redraw = (handler->clear != NULL);
    if (full_redraw) {
        if (handler->clear() != RASTER_RC_OK) {
            return RASTER_RC_ERROR;
        }
    }

    for (uint16_t i = 0; i < handler->size; ++i) {
        struct RasterBox *box = &handler->interface[i];
        if (!full_redraw && !box->updated) {
            continue;
        }
        if (prv_draw_box(box, handler->draw, !full_redraw) != RASTER_RC_OK) {
            return RASTER_RC_ERROR;
        }
    }

    return RASTER_RC_OK;
}

struct RasterBox *raster_api_get_box(struct RasterBox *boxes, uint16_t size, uint16_t id) {
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

enum RasterReturnCode raster_api_create_label_int32(struct RasterLabel *label, int32_t int32, const char *format, struct RasterCoords pos, const struct Font *font, uint16_t size, enum FontAlign align, struct Color color) {
    if (label == NULL || font == NULL) {
        return RASTER_RC_NULL_POINTER;
    }
    label->int32 = int32;
    label->format = format;
    label->type = RASTER_LABEL_DATA_INT;
    label->pos = pos;
    label->font = font;
    label->size = size;
    label->align = align;
    label->color = color;
    return RASTER_RC_OK;
}

enum RasterReturnCode raster_api_create_label_uint32(struct RasterLabel *label, uint32_t uint32, const char *format, struct RasterCoords pos, const struct Font *font, uint16_t size, enum FontAlign align, struct Color color) {
    if (label == NULL || font == NULL) {
        return RASTER_RC_NULL_POINTER;
    }
    label->uint32 = uint32;
    label->format = format;
    label->type = RASTER_LABEL_DATA_UINT;
    label->pos = pos;
    label->font = font;
    label->size = size;
    label->align = align;
    label->color = color;
    return RASTER_RC_OK;
}

enum RasterReturnCode raster_api_create_label_float(struct RasterLabel *label, float floating, const char *format, struct RasterCoords pos, const struct Font *font, uint16_t size, enum FontAlign align, struct Color color) {
    if (label == NULL || font == NULL) {
        return RASTER_RC_NULL_POINTER;
    }
    label->floating = floating;
    label->format = format;
    label->type = RASTER_LABEL_DATA_FLOAT;
    label->pos = pos;
    label->font = font;
    label->size = size;
    label->align = align;
    label->color = color;
    return RASTER_RC_OK;
}

enum RasterReturnCode raster_api_create_label_string(struct RasterLabel *label, char *string, const char *format, struct RasterCoords pos, const struct Font *font, uint16_t size, enum FontAlign align, struct Color color) {
    if (label == NULL || font == NULL || string == NULL) {
        return RASTER_RC_NULL_POINTER;
    }
    label->string = string;
    label->format = format;
    label->type = RASTER_LABEL_DATA_STRING;
    label->pos = pos;
    label->font = font;
    label->size = size;
    label->align = align;
    label->color = color;
    return RASTER_RC_OK;
}

enum RasterReturnCode raster_api_set_label_int32(struct RasterBox *box, int32_t value) {
    if (box == NULL || box->label == NULL) {
        return RASTER_RC_NULL_POINTER;
    }
    box->label->int32 = value;
    box->label->type = RASTER_LABEL_DATA_INT;
    box->updated = true;
    return RASTER_RC_OK;
}

enum RasterReturnCode raster_api_set_label_uint32(struct RasterBox *box, uint32_t value) {
    if (box == NULL || box->label == NULL) {
        return RASTER_RC_NULL_POINTER;
    }
    box->label->uint32 = value;
    box->label->type = RASTER_LABEL_DATA_UINT;
    box->updated = true;
    return RASTER_RC_OK;
}

enum RasterReturnCode raster_api_set_label_float(struct RasterBox *box, float value) {
    if (box == NULL || box->label == NULL) {
        return RASTER_RC_NULL_POINTER;
    }
    box->label->floating = value;
    box->label->type = RASTER_LABEL_DATA_FLOAT;
    box->updated = true;
    return RASTER_RC_OK;
}

enum RasterReturnCode raster_api_set_label_string(struct RasterBox *box, char *string) {
    if (box == NULL || box->label == NULL) {
        return RASTER_RC_NULL_POINTER;
    }
    box->label->string = string;
    box->label->type = RASTER_LABEL_DATA_STRING;
    box->updated = true;
    return RASTER_RC_OK;
}

enum RasterReturnCode raster_api_set_label_format(struct RasterBox *box, const char *format) {
    if (box == NULL || box->label == NULL) {
        return RASTER_RC_NULL_POINTER;
    }
    box->label->format = format;
    box->updated = true;
    return RASTER_RC_OK;
}
