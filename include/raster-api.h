/*!
 * \file raster-api.h
 * \date 2025-03-21
 * \author Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief LibRaster API functions
 *
 * \details A rasterizer is a software renderer that works by writing pixels to
 *      a framebuffer.
 *      This implementation lets the user define 3 callbacks that defines the
 *      technique used to write them, so that hardware accelerators can be used
 *      to archive big speedups.
 */

#ifndef RASTER_API_H
#define RASTER_API_H

#include "raster.h"

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
void raster_api_render(struct Box *boxes, uint16_t num, draw_line_callback draw_line, draw_rectangle_callback draw_rectangle, clear_screen_callback clear_screen);

/*!
 * \brief Utility to get a Box based on id value
 *
 * \details Used to retrieve a specific box that needs to be modified.
 *
 * \param[in] boxes Pointer to the defined interface
 * \param[in] num Number of Box in the interface
 * \param[in] id ID of the box to search for
 *
 * \return struct Box*
 *     - Box pointer if found
 *     - NULL if not found
 */
struct Box *raster_api_get_box(struct Box *boxes, uint16_t num, uint16_t id);

/*!
 * \brief Utility to populate struct Label
 *
 * \param[out] label The label struct to populate
 * \param[in] value Union of possible value types
 * \param[in] pos Position of the text
 * \param[in] font Font name (defined in generation)
 * \param[in] font_size Text size
 * \param[in] align Alignment of font
 */
void raster_api_create_label(struct Label *label, union LabelData value, enum LabelDataType type, struct Coords pos, enum FontName font, uint16_t size, enum FontAlign align, struct Color color);

/*!
 * \brief Utility to set label data inside a Box
 *
 * \param[in,out] box The box to modify
 * \param[in] value Union of possible value types
 * \param[in] type Type of the value passed
 */
void raster_api_set_label_data(struct Box *box, union LabelData value, enum LabelDataType type);

#endif // RASTER_API_H
