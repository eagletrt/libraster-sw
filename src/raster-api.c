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
    if (box->label) {
        // Format the value accordingly to what we want
        char buf[MAX_BUFFER_SIZE];
        struct RasterLabel *label = box->label;
        switch (label->type) {
            case LABEL_DATA_INT:
                snprintf(buf, sizeof(buf), "%" PRId32, label->data.int_val);
                break;
            case LABEL_DATA_FLOAT_1:
                snprintf(buf, sizeof(buf), "%.1f", label->data.float_val);
                break;
            case LABEL_DATA_FLOAT_2:
                snprintf(buf, sizeof(buf), "%.2f", label->data.float_val);
                break;
            case LABEL_DATA_FLOAT_3:
                snprintf(buf, sizeof(buf), "%.3f", label->data.float_val);
                break;
            case LABEL_DATA_STRING:
                strncpy(buf, label->data.text, sizeof(buf));
                buf[sizeof(buf) - 1] = '\0';
                break;
            default:
                buf[0] = '\0';
                break;
        }
        // Plot the value
        font_api_draw(box->rect.x + label->pos.x,
                      box->rect.y + label->pos.y,
                      label->align,
                      label->font,
                      buf,
                      label->color,
                      label->size,
                      line_callback);
    }
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

void raster_api_create_label(struct RasterLabel *label, union RasterLabelData data, enum RasterLabelDataType type, struct RasterCoords pos, enum FontName font, uint16_t size, enum FontAlign align, struct Color color) {
    if (label == NULL)
        return;
    label->data = data;
    label->type = type;
    label->pos = pos;
    label->font = font;
    label->size = size;
    label->align = align;
    label->color = color;
}

void raster_api_set_label_data(struct RasterBox *box, union RasterLabelData data, enum RasterLabelDataType type) {
    if (box == NULL || box->label == NULL)
        return;
    box->label->data = data;
    box->label->type = type;
}
