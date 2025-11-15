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
void prv_draw_text_box(struct Box *box, draw_rectangle_callback draw_rectangle, draw_line_callback line_callback) {
#if PARTIAL_RASTER
    if (!box->updated)
        return;
#endif
    // Draw the basic rectangle
    draw_rectangle(box->rect.x, box->rect.y, box->rect.w, box->rect.h, box->color);
    if (box->label) {
        // Format the value accordingly to what we want
        char buf[128];
        struct Label *label = box->label;
        switch (label->type) {
            case LABEL_DATA_INT:
                snprintf(buf, sizeof(buf), "%ld", label->data.int_val);
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

void raster_api_render(struct Box *text_boxes, uint16_t num, draw_line_callback draw_line, draw_rectangle_callback draw_rectangle, clear_screen_callback clear_screen) {
    // Do not clear full screen for max optimization (less time spent)
    if (PARTIAL_RASTER == 0)
        clear_screen();

    for (int i = 0; i < num; i++) {
        prv_draw_text_box(text_boxes + i, draw_rectangle, draw_line);
    }
}

struct Box *raster_api_get_box(struct Box *boxes, uint16_t num, uint16_t id) {
    // Loops and search for IDs (can be good for CAN IDs)
    for (int i = 0; i < num; i++) {
        if ((boxes + i)->id == id) {
            return (boxes + i);
        }
    }
    return NULL;
}

void raster_api_create_label(struct Label *label, union LabelData data, enum LabelDataType type, struct Coords pos, enum FontName font, uint16_t size, enum FontAlign align, struct Color color) {
    if (label) {
        label->data = data;
        label->type = type;
        label->pos = pos;
        label->font = font;
        label->size = size;
        label->align = align;
        label->color = color;
    }
}

void raster_api_set_label_data(struct Box *box, union LabelData data, enum LabelDataType type) {
    if (box && box->label) {
        box->label->data = data;
        box->label->type = type;
    }
}
