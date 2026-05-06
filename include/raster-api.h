/*!
 * \file raster-api.h
 * \date 2025-03-21
 * \author Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief LibRaster public API.
 *
 * \details A rasterizer is a software renderer that writes pixels to a
 *     framebuffer. Libraster lets the user define a single rectangle-fill
 *     callback so that hardware accelerators (e.g. STM32 DMA2D) can be
 *     used transparently. No dynamic allocation is performed.
 */

#ifndef RASTER_API_H
#define RASTER_API_H

#include "raster.h"

/*!
 * \brief Initialize a RasterHandler.
 *
 * \details The clear callback may be NULL when RASTER_PARTIAL == 1 (the default).
 *
 * \param[out] handler   Handler to initialize.
 * \param[in]  interface Interface to mount.
 * \param[in]  size      Number of boxes in \p interface.
 * \param[in]  draw      Rectangle-fill callback.
 * \param[in]  clear     Clear-screen callback. Required only when RASTER_PARTIAL is 0.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_NULL_POINTER if \p handler, \p interface, or \p draw is NULL,
 *     or if \p size is 0.
 */
enum RasterReturnCode raster_api_init(struct RasterHandler *handler, struct RasterBox *interface, uint16_t size, raster_draw_rectangle_callback draw, raster_clear_screen_callback clear);

/*!
 * \brief Replace the interface mounted on a handler.
 *
 * \details Useful for swapping screens (main view, popup, menu...).
 *
 * \param[in,out] handler   Handler to update.
 * \param[in]     interface New interface to mount.
 * \param[in]     size      Number of boxes in \p interface.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_NULL_POINTER if \p handler or \p interface is NULL.
 */
enum RasterReturnCode raster_api_set_interface(struct RasterHandler *handler, struct RasterBox *interface, uint16_t size);

/*!
 * \brief Render the current interface.
 *
 * \details Behavior depends on RASTER_PARTIAL:
 *     - When 1 (default): only boxes whose `updated` flag is set are
 *       redrawn, and the flag is cleared after a successful redraw.
 *     - When 0: the screen is cleared and every box is drawn.
 *
 * \param[in] handler Handler to render.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_NULL_POINTER if \p handler is NULL.
 * \retval RASTER_RC_ERROR if a callback reported an error.
 */
enum RasterReturnCode raster_api_render(struct RasterHandler *handler);

/*!
 * \brief Find a box by id within a flat array.
 *
 * \details Linear search; convenient for small interfaces (the typical case
 *     for embedded UIs) and decoupled from any particular handler.
 *
 * \param[in] boxes Array to search.
 * \param[in] size  Number of entries in \p boxes.
 * \param[in] id    Identifier to look up.
 *
 * \return Pointer to the matching box, or NULL if no box has that id.
 */
struct RasterBox *raster_api_get_box(struct RasterBox *boxes, uint16_t size, uint16_t id);

/*!
 * \brief Initialize a RasterLabel from explicit components.
 *
 * \details The data, type, and format must be consistent with each other.
 *     For most cases you can construct the format union with the
 *     raster_api_*_format helpers, or simply use a designated initializer
 *     directly on the RasterLabel.
 *
 * \param[out] label   Label to initialize.
 * \param[in]  data    Initial value.
 * \param[in]  type    Type of the value.
 * \param[in]  format  Formatting options.
 * \param[in]  pos     Position of the label within its box, in pixels.
 * \param[in]  font    Font to use for rendering the label.
 * \param[in]  size    Font size to use for rendering the label.
 * \param[in]  align   Alignment of the label within its box.
 * \param[in]  color   Color to use for rendering the label.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_NULL_POINTER if \p label is NULL or \p font is NULL.
 */
enum RasterReturnCode raster_api_create_label(struct RasterLabel *label, union RasterLabelData data, enum RasterLabelDataType type, union RasterLabelFormat format, struct RasterCoords pos, const struct Font *font, uint16_t size, enum FontAlign align, struct Color color);

/*!
 * \brief Update the value of a label and mark its box for redraw.
 *
 * \details The label keeps its current type. The data union member must
 *     match the existing type — these helpers do not change the type.
 *
 * \param[in,out] box   Box owning the label to update.
 * \param[in]     data  New value.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_NULL_POINTER if \p box or \p box->label is NULL.
 */
enum RasterReturnCode raster_api_set_label_data(struct RasterBox *box, union RasterLabelData data);

/*! 
 * \brief Convenience wrapper around raster_api_set_label_data for strings.
 *
 * \param[in,out] box   Box owning the label to update.
 * \param[in]     text  New string value. The label will point to this string, so it must remain valid as long as the label needs it.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_NULL_POINTER if \p box or \p box->label is NULL.
 */
enum RasterReturnCode raster_api_set_label_text(struct RasterBox *box, char *text);

/*!
 * \brief Convenience wrapper around raster_api_set_label_data for integers.
 *
 * \param[in,out] box   Box owning the label to update.
 * \param[in]     value New integer value.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_NULL_POINTER if \p box or \p box->label is
 */
enum RasterReturnCode raster_api_set_label_int(struct RasterBox *box, int32_t value);

/*!
 * \brief Convenience wrapper around raster_api_set_label_data for floats.
 *
 * \param[in,out] box   Box owning the label to update.
 * \param[in]     value New float value.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_NULL_POINTER if \p box or \p box->label is
 */
enum RasterReturnCode raster_api_set_label_float(struct RasterBox *box, float value);

/*!
 * \brief Update the formatting options of a label and mark its box for redraw.
 *
 * \param[in,out] box     Box owning the label to update.
 * \param[in]     format  New formatting options.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_NULL_POINTER if \p box or \p box->label is NULL.
 */
enum RasterReturnCode raster_api_set_label_format(struct RasterBox *box, union RasterLabelFormat format);

/*!
 * \brief Build a RasterIntFormat.
 *
 * \param[in] is_unsigned Whether the integer is unsigned (true) or signed (false).
 *
 * \return A RasterIntFormat with the specified properties.
 */
struct RasterIntFormat raster_api_int_format(bool is_unsigned);

/*!
 * \brief Build a RasterFloatFormat.
 *
 * \param[in] precision Number of decimal places to display.
 *
 * \return A RasterFloatFormat with the specified properties.
 */
struct RasterFloatFormat raster_api_float_format(uint8_t precision);

/*!
 * \brief Build a RasterStringFormat.
 *
 * \param[in] max_length Maximum number of characters to display. Longer strings will be truncated.
 *
 * \return A RasterStringFormat with the specified properties.
 */
struct RasterStringFormat raster_api_string_format(uint16_t max_length);

#endif // RASTER_API_H
