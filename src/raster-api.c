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
#include "eagletrt.h"
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
 * \brief Format a label's value into a NUL-terminated buffer.
 *
 * \param[in]  label       Label to format.
 * \param[out] buffer      Buffer to write the formatted string into.
 * \param[in]  buffer_size Size of \p buffer in bytes.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_ERROR if the label has an unrecognized type.
 */
EAGLETRT_STATIC enum RasterReturnCode prv_format_label(const struct RasterLabel *label, char *buffer, size_t buffer_size) {
    switch (label->type) {
        case RASTER_LABEL_DATA_INT: {
            const struct RasterIntFormat *fmt = &label->format.int_fmt;
            if (fmt->is_unsigned) {
                snprintf(buffer, buffer_size, "%" PRIu32, (uint32_t)label->data.int_val);
            } else {
                snprintf(buffer, buffer_size, "%" PRId32, label->data.int_val);
            }
            return RASTER_RC_OK;
        }

        case RASTER_LABEL_DATA_FLOAT: {
            const struct RasterFloatFormat *fmt = &label->format.float_fmt;
            snprintf(buffer, buffer_size, "%.*f", fmt->precision, (double)label->data.float_val);
            return RASTER_RC_OK;
        }

        case RASTER_LABEL_DATA_STRING: {
            const struct RasterStringFormat *fmt = &label->format.string_fmt;
            if (label->data.text == NULL) {
                buffer[0] = '\0';
                return RASTER_RC_OK;
            }
            size_t cap = buffer_size - 1u;
            if (fmt->max_length > 0u && fmt->max_length < cap) {
                cap = fmt->max_length;
            }
            strncpy(buffer, label->data.text, cap);
            buffer[cap] = '\0';
            return RASTER_RC_OK;
        }
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
        char buffer[RASTER_LABEL_BUFFER_SIZE];
        if (prv_format_label(box->label, buffer, sizeof(buffer)) != RASTER_RC_OK) {
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

enum RasterReturnCode raster_api_create_label(struct RasterLabel *label, union RasterLabelData data, enum RasterLabelDataType type, union RasterLabelFormat format, struct RasterCoords pos, const struct Font *font, uint16_t size, enum FontAlign align, struct Color color) {
    if (label == NULL || font == NULL) {
        return RASTER_RC_NULL_POINTER;
    }
    label->data = data;
    label->type = type;
    label->format = format;
    label->pos = pos;
    label->font = font;
    label->size = size;
    label->align = align;
    label->color = color;
    return RASTER_RC_OK;
}

enum RasterReturnCode raster_api_set_label_data(struct RasterBox *box, union RasterLabelData data) {
    if (box == NULL || box->label == NULL) {
        return RASTER_RC_NULL_POINTER;
    }
    box->label->data = data;
    box->updated = true;
    return RASTER_RC_OK;
}

enum RasterReturnCode raster_api_set_label_text(struct RasterBox *box, char *text) {
    return raster_api_set_label_data(box, (union RasterLabelData){ .text = text });
}

enum RasterReturnCode raster_api_set_label_int(struct RasterBox *box, int32_t value) {
    return raster_api_set_label_data(box, (union RasterLabelData){ .int_val = value });
}

enum RasterReturnCode raster_api_set_label_float(struct RasterBox *box, float value) {
    return raster_api_set_label_data(box, (union RasterLabelData){ .float_val = value });
}

enum RasterReturnCode raster_api_set_label_format(struct RasterBox *box, union RasterLabelFormat format) {
    if (box == NULL || box->label == NULL) {
        return RASTER_RC_NULL_POINTER;
    }
    box->label->format = format;
    box->updated = true;
    return RASTER_RC_OK;
}

struct RasterIntFormat raster_api_int_format(bool is_unsigned) {
    return (struct RasterIntFormat){ .is_unsigned = is_unsigned };
}

struct RasterFloatFormat raster_api_float_format(uint8_t precision) {
    return (struct RasterFloatFormat){ .precision = precision };
}

struct RasterStringFormat raster_api_string_format(uint16_t max_length) {
    return (struct RasterStringFormat){ .max_length = max_length };
}
