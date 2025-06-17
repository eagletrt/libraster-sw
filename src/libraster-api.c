/**
 * @file libraster-api.c
 * @date 2024-12-13
 * @author Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * @brief Graphics handling functions
 */

#include "libraster-api.h"
#include "fontutils-api.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

uint32_t _interpolate_color(uint32_t color1, uint32_t color2, float min, float max, float actual_value) {
    if (actual_value < min)
        actual_value = min;
    if (actual_value > max)
        actual_value = max;

    float t = (actual_value - min) / (max - min);

    uint8_t a1 = (color1 >> 24) & 0xFF;
    uint8_t r1 = (color1 >> 16) & 0xFF;
    uint8_t g1 = (color1 >> 8) & 0xFF;
    uint8_t b1 = color1 & 0xFF;

    uint8_t a2 = (color2 >> 24) & 0xFF;
    uint8_t r2 = (color2 >> 16) & 0xFF;
    uint8_t g2 = (color2 >> 8) & 0xFF;
    uint8_t b2 = color2 & 0xFF;

    uint8_t a = (uint8_t)((1 - t) * a1 + t * a2);
    uint8_t r = (uint8_t)((1 - t) * r1 + t * r2);
    uint8_t g = (uint8_t)((1 - t) * g1 + t * g2);
    uint8_t b = (uint8_t)((1 - t) * b1 + t * b2);

    return (a << 24) | (r << 16) | (g << 8) | b;
}

void _extract_threshold(Box *box, uint32_t *bg_color, uint32_t *fg_color) {
    for (int i = 0; i < box->value->colors.thresholds->thresholds_num; i++) {
        if (box->value->colors.thresholds->threshold[i].min <= box->value->value && box->value->value <= box->value->colors.thresholds->threshold[i].max) {
            *bg_color = box->value->colors.thresholds->threshold[i].bg_color;
            *fg_color = box->value->colors.thresholds->threshold[i].fg_color;
        }
    }
}

void _calculate_slider_position(Box *box, uint32_t *x, uint32_t *y, uint32_t *width, uint32_t *height) {
    float percent = 1.f - ((box->value->value - box->value->colors.slider.min) /
                           (box->value->colors.slider.max - box->value->colors.slider.min));
    if (box->value->colors.slider.anchor == ANCHOR_TOP || box->value->colors.slider.anchor == ANCHOR_BOTTOM) {
        *width = box->rect.w - 2 * box->value->colors.slider.margin;
        *height = (box->rect.h - 2 * box->value->colors.slider.margin) * percent;
        *x = box->rect.x + box->value->colors.slider.margin;
        *y = box->rect.y + (box->value->colors.slider.anchor == ANCHOR_TOP ? box->value->colors.slider.margin : box->rect.h - box->value->colors.slider.margin - *height);
    } else if (box->value->colors.slider.anchor == ANCHOR_LEFT || box->value->colors.slider.anchor == ANCHOR_RIGHT) {
        *width = (box->rect.w - 2 * box->value->colors.slider.margin) * percent;
        *height = box->rect.h - 2 * box->value->colors.slider.margin;
        *x = box->rect.x + (box->value->colors.slider.anchor == ANCHOR_LEFT ? box->value->colors.slider.margin : box->rect.w - box->value->colors.slider.margin - *width);
        *y = box->rect.y + box->value->colors.slider.margin;
    }
}

void _draw_text_box(Box *box, draw_rectangle_callback_t draw_rectangle, draw_line_callback_t line_callback) {
#if PARTIAL_RASTER
    if (!box->updated)
        return;
#endif
    uint32_t bg_color = box->default_bg_color;
    uint32_t fg_color = box->default_fg_color;

    // In this block colors are selected based on thresholds selected for value (if any)
    if (box->value && box->value->color_type == THRESHOLDS && box->value->colors.thresholds) {
        _extract_threshold(box, &bg_color, &fg_color);
    } else if (box->value && box->value->color_type == INTERPOLATION) {
        bg_color = _interpolate_color(box->value->colors.interpolation.color_min,
                                      box->value->colors.interpolation.color_max,
                                      box->value->colors.interpolation.min,
                                      box->value->colors.interpolation.max,
                                      box->value->value);
    }

    // Draw the basic rectangle
    draw_rectangle(box->rect.x, box->rect.y, box->rect.w, box->rect.h, bg_color);
    if (box->value && box->value->color_type == SLIDER) {
        uint32_t x, y, width, height;
        _calculate_slider_position(box, &x, &y, &width, &height);
        draw_rectangle(x, y, width, height, box->value->colors.slider.color);
    }
    if (box->value) {
        // Format the value accordingly to what we want
        char buf[15];
        if (box->value->is_float) {
            snprintf(buf, sizeof(buf), "%.2f", box->value->value);
        } else {
            snprintf(buf, sizeof(buf), "%d", (int)box->value->value);
        }
        // Plot the value
        draw_text(box->rect.x + box->value->pos.x,
                  box->rect.y + box->value->pos.y,
                  box->value->align,
                  box->value->font,
                  buf,
                  fg_color,
                  box->value->font_size,
                  line_callback);
    }

    if (box->label) {
        // Plot the label
        draw_text(box->rect.x + box->label->pos.x,
                  box->rect.y + box->label->pos.y,
                  box->label->align,
                  box->label->font,
                  box->label->text,
                  fg_color,
                  box->label->font_size,
                  line_callback);
    }
}

void render_interface(Box *text_boxes, uint16_t num, draw_line_callback_t draw_line, draw_rectangle_callback_t draw_rectangle
#if PARTIAL_RASTER == 0
                      ,
                      clear_screen_callback_t clear_screen
#endif
) {
// Do not clear full screen for max optimization (less time spent)
#if PARTIAL_RASTER == 0
    clear_screen();
#endif
    for (int i = 0; i < num; i++) {
        _draw_text_box(text_boxes + i, draw_rectangle, draw_line);
    }
}

Box *get_box(Box *boxes, uint16_t num, uint16_t id) {
    // Loops and search for IDs (can be good for CAN IDs)
    for (int i = 0; i < num; i++) {
        if ((boxes + i)->id == id) {
            return (boxes + i);
        }
    }
    return NULL;
}

void create_label(Label *label, char *text, Coords pos, FontName font, uint16_t font_size, FontAlign align) {
    if (label) {
        label->text = text;
        label->pos = pos;
        label->font = font;
        label->font_size = font_size;
        label->align = align;
    }
}

void create_value(Value *value, float val, bool is_float, Coords pos, FontName font, uint16_t font_size, FontAlign align, Colors colors, ColorType color_type) {
    if (value) {
        value->value = val;
        value->is_float = is_float;
        value->pos = pos;
        value->font = font;
        value->font_size = font_size;
        value->align = align;
        value->colors = colors;
        value->color_type = color_type;
    }
}
