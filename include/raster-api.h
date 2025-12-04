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
 * \brief Initializes the RasterHandler struct
 *
 * \details Sets the callbacks inside the RasterHandler struct
 *     passed as argument.
 *     Clear screen callback is optional, and can be set to NULL when RASTER_PARTIAL
 *     is equal to 1.
 *
 * \param[out] hras Pointer to the RasterHandler struct to initialize
 * \param[in] interface Pointer to the defined interface
 * \param[in] size Number of boxes in the interface
 * \param[in] draw_line Draw line callback
 * \param[in] draw_rectangle Draw rectangle callback
 * \param[in] clear_screen Clear screen callback
 */
void raster_api_init(struct RasterHandler *hras, struct RasterBox *interface, uint16_t size, font_draw_line_callback draw_line, raster_draw_rectangle_callback draw_rectangle, raster_clear_screen_callback clear_screen);

/*!
 * \brief Sets the interface inside the RasterHandler struct
 *
 * \details Sets the interface and size inside the RasterHandler struct
 *     passed as argument.
 *
 * \param[out] hras Pointer to the RasterHandler struct to modify
 * \param[in] interface Pointer to the defined interface
 * \param[in] size Number of boxes in the interface
 */
void raster_api_set_interface(struct RasterHandler *hras, struct RasterBox *interface, uint16_t size);

/*!
 * \brief Renders the whole interface
 *
 * \details For every box, draws it using the callbacks that are passed
 *      as arguments to the function.
 *
 *      The signature of the function changes based on the \c RASTER_PARTIAL
 *      env variable, adding or removing the \c clear_screen callback.
 *
 * \param[in] hras Pointer to the RasterHandler struct to use
 * \param[in] boxes Pointer to the defined interface
 * \param[in] num Number of boxes in the interface
 */
void raster_api_render(struct RasterHandler *hras);

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
struct RasterBox *raster_api_get_box(struct RasterBox *boxes, uint16_t num, uint16_t id);

/*!
 * \brief Utility to populate struct Label
 *
 * \param[out] label The label struct to populate
 * \param[in] value Union of possible value types
 * \param[in] type Type of the value passed
 * \param[in] format Formatting options for the value
 * \param[in] pos Position of the text
 * \param[in] font Font name (defined in generation)
 * \param[in] size Text size
 * \param[in] align Alignment of font
 * \param[in] color Color of the text
 */
void raster_api_create_label(struct RasterLabel *label, union RasterLabelData value, enum RasterLabelDataType type, union RasterLabelFormat format, struct RasterCoords pos, enum FontName font, uint16_t size, enum FontAlign align, struct Color color);

/*!
 * \brief Utility to set label data inside a Box
 *
 * \param[in,out] box The box to modify
 * \param[in] value Union of possible value types
 */
void raster_api_set_label_data(struct RasterBox *box, union RasterLabelData value);

/*!
 * \brief Utility to update label formatting options
 *
 * \param[in,out] box The box to modify
 * \param[in] format Formatting options for the value
 */
void raster_api_set_label_format(struct RasterBox *box, union RasterLabelFormat format);

/*!
 * \brief Helper to create default integer formatting options
 *
 * \param[in] is_unsigned Treat as unsigned integer
 * \return struct RasterIntFormat with specified options
 */
struct RasterIntFormat raster_api_int_format(bool is_unsigned);

/*!
 * \brief Helper to create default float formatting options
 *
 * \param[in] precision Number of digits after decimal point
 * \return struct RasterFloatFormat with specified options
 */
struct RasterFloatFormat raster_api_float_format(uint8_t precision);

/*!
 * \brief Helper to create default string formatting options
 *
 * \param[in] max_length Maximum string length (0 for no limit)
 * \return struct RasterStringFormat with specified options
 */
struct RasterStringFormat raster_api_string_format(uint16_t max_length);

#endif // RASTER_API_H
