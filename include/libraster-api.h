/*!
 * \file libraster-api.h
 * \date 2025-03-21
 * \author Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief FontUtils APIs functions implementations
 *
 * \details A rasterizer is a software renderer that works by writing pixels to
 *      a framebuffer.
 *      This implementation lets the user define 3 callbacks that defines the
 *      technique used to write them, so that hardware accelerators can be used
 *      to archive big speedups.
 */

#ifndef RASTER_API_H
#define RASTER_API_H

#include "libraster.h"

/*!
 * \brief Renders the whole interface
 *
 * \details For every box, draws it using the callbacks that are passed
 *      as arguments to the function.
 *
 *      The signature of the function changes based on the \c PARTIAL_RASTER
 *      env variable, adding or removing the \c clear_screen callback.
 *
 * \param[in] boxes Pointer to the defined interface
 * \param[in] num Number of boxes in the interface
 * \param[in] draw_line Draw line callback
 * \param[in] draw_rectangle Draw rectangle callback
 * \param[in] clear_screen Clear screen callback
 */
void render_interface(struct Box *boxes, uint16_t num, draw_line_callback_t draw_line, draw_rectangle_callback_t draw_rectangle
#if PARTIAL_RASTER == 0
                      ,
                      clear_screen_callback_t clear_screen
#endif
);

/*!
 * \brief Utility to get a Box based on id value
 *
 * \details Used to retrieve a specific box that needs to be modified.
 *
 * \param[in] boxes Pointer to the defined interface
 * \param[in] num Nunber of Box in the inteface
 * \param[in] id ID of the box to search for
 * \return struct Box*
 *     - Box pointer if found
 *     - NULL if not found
 */
struct Box *get_box(struct Box *boxes, uint16_t num, uint16_t id);

/*!
 * \brief Utility to populate struct Label
 *
 * \param[out] label The label struct to populate
 * \param[in] text Text in the label
 * \param[in] pos Position of the text
 * \param[in] font Font name (defined in generation)
 * \param[in] font_size Text size
 * \param[in] align Alignment of font
 */
void create_label(struct Label *label, char *text, struct Coords pos, enum FontName font, uint16_t font_size, enum FontAlign align);

/*!
 * \brief Utility to populate struct Value
 *
 * \param[out] value The value struct to populate
 * \param[in] val Value
 * \param[in] is_float Says is the value is to cast to int
 * \param[in] pos Position of the value
 * \param[in] font Font name (defined in generation)
 * \param[in] font_size Value size
 * \param[in] align ALignment of font
 * \param[in] colors Pointer to color ranges
 */
void create_value(struct Value *value, float val, bool is_float, struct Coords pos, enum FontName font, uint16_t font_size, enum FontAlign align, union Colors colors, enum ColorType color_type);

#endif // RASTER_API_H
