/*!
 * \file raster-api.c
 * \date 2024-12-13
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief LibRaster API functions implementations
 *
 * \details A rasterizer is a software renderer that works by writing pixels to
 *      a framebuffer.
 *      This implementation lets the user define 3 callbacks that defines the
 *      technique used to write them, so that hardware accelerators can be used
 *      to archive big speedups.
 */

#include "raster-api.h"
#include "fontutils-api.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#define MAX_BUFFER_SIZE (128)

/*!
 * \brief Internal function to format label data into a string buffer
 *
 * \details Formats the label data according to its type and formatting options
 *
 * \param[in] label Pointer to the RasterLabel structure
 * \param[out] buffer Buffer to store the formatted string
 * \param[in] buffer_size Size of the buffer
 */
static void prv_format_label_data(const struct RasterLabel *label, char *buffer, size_t buffer_size) {
    if (label == NULL || buffer == NULL || buffer_size == 0) {
        if (buffer != NULL && buffer_size > 0) {
            buffer[0] = '\0';
        }
        return;
    }

    switch (label->type) {
        case LABEL_DATA_INT: {
            const struct RasterIntFormat *fmt = &label->format.int_fmt;
            if (fmt->is_unsigned) {
                snprintf(buffer, buffer_size, "%" PRIu32, (uint32_t)label->data.int_val);
            } else {
                snprintf(buffer, buffer_size, "%" PRId32, label->data.int_val);
            }
            break;
        }

        case LABEL_DATA_FLOAT: {
            const struct RasterFloatFormat *fmt = &label->format.float_fmt;
            snprintf(buffer, buffer_size, "%.*f", fmt->precision, label->data.float_val);
            break;
        }

        case LABEL_DATA_STRING: {
            const struct RasterStringFormat *fmt = &label->format.string_fmt;
            if (label->data.text == NULL) {
                buffer[0] = '\0';
            } else if (fmt->max_length > 0 && fmt->max_length < buffer_size) {
                strncpy(buffer, label->data.text, fmt->max_length);
                buffer[fmt->max_length] = '\0';
            } else {
                strncpy(buffer, label->data.text, buffer_size - 1);
                buffer[buffer_size - 1] = '\0';
            }
            break;
        }

        default:
            buffer[0] = '\0';
            break;
    }
}

/*!
 * \brief Draws a text box with background, value, and label
 *
 * \details This function draws a text box on the screen using the provided
 *     drawing callbacks. It handles background color, value formatting,
 *     threshold-based coloring, slider rendering, and label drawing.
 * 
 * \param[in] box Pointer to the Box structure containing the text box configuration
 * \param[in] draw_rectangle Callback function to draw rectangles
 * \param[in] line_callback Callback function to draw lines of text
 */
void prv_draw_text_box(struct RasterBox *box, raster_draw_rectangle_callback draw_rectangle, font_draw_line_callback line_callback) {
    if (RASTER_PARTIAL == 0 && !box->updated)
        return;
    // Draw the basic rectangle
    draw_rectangle(box->rect.x, box->rect.y, box->rect.w, box->rect.h, box->color);
    if (box->label == NULL)
        return;

    // Format the value using the internal formatting function
    char buf[MAX_BUFFER_SIZE];
    prv_format_label_data(box->label, buf, sizeof(buf));

    // Plot the value
    font_api_draw(box->rect.x + box->label->pos.x,
                  box->rect.y + box->label->pos.y,
                  box->label->align,
                  box->label->font,
                  buf,
                  box->label->color,
                  box->label->size,
                  line_callback);
}

void raster_api_init(struct RasterHandler *hras, struct RasterBox *interface, uint16_t size, font_draw_line_callback draw_line, raster_draw_rectangle_callback draw_rectangle, raster_clear_screen_callback clear_screen) {
    if (hras == NULL)
        return;
    hras->interface = interface;
    hras->size = size;
    hras->draw_line = draw_line;
    hras->draw_rectangle = draw_rectangle;
    hras->clear_screen = clear_screen;
}

void raster_api_set_interface(struct RasterHandler *hras, struct RasterBox *interface, uint16_t size) {
    if (hras == NULL)
        return;
    hras->interface = interface;
    hras->size = size;
}

void raster_api_render(struct RasterHandler *hras) {
    // Do not clear full screen for max optimization (less time spent)
    if (RASTER_PARTIAL == 0)
        hras->clear_screen();

    for (int i = 0; i < hras->size; i++) {
        prv_draw_text_box(&hras->interface[i], hras->draw_rectangle, hras->draw_line);
    }
}

struct RasterBox *raster_api_get_box(struct RasterBox *boxes, uint16_t num, uint16_t id) {
    // Loops and search for IDs (can be good for CAN IDs)
    for (int i = 0; i < num; i++) {
        struct RasterBox *box = boxes + i;
        if (box->id == id) {
            return (boxes + i);
        }
    }
    return NULL;
}

void raster_api_create_label(struct RasterLabel *label, union RasterLabelData data, enum RasterLabelDataType type, union RasterLabelFormat format, struct RasterCoords pos, enum FontName font, uint16_t size, enum FontAlign align, struct Color color) {
    if (label == NULL)
        return;
    label->data = data;
    label->type = type;
    label->format = format;
    label->pos = pos;
    label->font = font;
    label->size = size;
    label->align = align;
    label->color = color;
}

void raster_api_set_label_data(struct RasterBox *box, union RasterLabelData data) {
    if (box == NULL || box->label == NULL)
        return;
    box->label->data = data;
}

void raster_api_set_label_format(struct RasterBox *box, union RasterLabelFormat format) {
    if (box == NULL || box->label == NULL)
        return;
    box->label->format = format;
}

struct RasterIntFormat raster_api_int_format(bool is_unsigned) {
    struct RasterIntFormat fmt = {
        .is_unsigned = is_unsigned
    };
    return fmt;
}

struct RasterFloatFormat raster_api_float_format(uint8_t precision) {
    struct RasterFloatFormat fmt = {
        .precision = precision
    };
    return fmt;
}

struct RasterStringFormat raster_api_string_format(uint16_t max_length) {
    struct RasterStringFormat fmt = {
        .max_length = max_length
    };
    return fmt;
}
