/**
 * @file libraster-api.h
 * @date 2025-03-21
 * @author Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * @brief APIs of the rasterization library
 */

#ifndef RASTER_API_H
#define RASTER_API_H

#include "libraster.h"

/**
 * @brief Renders the whole interface
 *
 * @param boxes Pointer to the defined interface
 * @param num Number of Box in the interface
 * @param draw_pixel Draw pixel callback
 * @param draw_rectangle Draw rectangle callback
 * @param clear_screen Clear screen callback
 */
void render_interface(Box *boxes, uint16_t num, draw_line_callback_t draw_line, draw_rectangle_callback_t draw_rectangle
#if GRAPHICS_OPT == 0
                      ,
                      clear_screen_callback_t clear_screen
#endif
);

/**
 * @brief Utility to get a Box based on id value
 *
 * @param boxes Pointer to the defined interface
 * @param num Nunber of Box in the inteface
 * @param id ID of the box to search for
 * @return struct Box*
 *     - Box pointer if found
 *     - NULL if not found
 */
Box *get_box(Box *boxes, uint16_t num, uint16_t id);

/**
 * @brief Utility to populate struct Label
 *
 * @param label The label struct to populate
 * @param text Text in the label
 * @param pos Position of the text
 * @param font Font name (defined in font.h)
 * @param font_size Text size
 * @param align Alignment of font
 */
void create_label(Label *label, char *text, Coords pos, FontName font, uint16_t font_size, FontAlign align);

/**
 * @brief Utility to populate struct Value
 *
 * @param value The value struct to populate
 * @param val Value
 * @param is_float Says is the value is to cast to int
 * @param pos Position of the value
 * @param font Font name (defined in font.h)
 * @param font_size Value size
 * @param align ALignment of font
 * @param colors Pointer to color ranges
 * @param colors_num Number of color ranges
 */
void create_value(Value *value, float val, bool is_float, Coords pos, FontName font, uint16_t font_size, FontAlign align, Colors colors, ColorType color_type);

#endif // RASTER_API_H
