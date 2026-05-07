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
 * \details The presence of a clear callback selects the render mode. Pass
 *     NULL to opt into partial mode (the common case): only boxes flagged
 *     as updated are redrawn each frame. Pass a non-NULL clear callback to
 *     opt into full-redraw mode: every frame starts by clearing the screen
 *     and ends with every box drawn.
 *
 * \param[out] handler   Handler to initialize.
 * \param[in]  interface Interface to mount.
 * \param[in]  size      Number of boxes in \p interface.
 * \param[in]  draw      Rectangle-fill callback.
 * \param[in]  clear     Clear-screen callback, or NULL to use partial mode.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_NULL_POINTER if \p handler, \p interface, or \p draw is
 *     NULL, or if \p size is 0.
 */
enum RasterReturnCode raster_api_init(struct RasterHandler *handler, struct RasterBox *interface, uint16_t size, raster_draw_rectangle_callback draw, raster_clear_screen_callback clear);

/*!
 * \brief Replace the interface mounted on a handler.
 *
 * \details Useful for swapping screens (main view, popup, menu, etc.).
 *
 * \param[in,out] handler   Handler to update.
 * \param[in]     interface New interface to mount.
 * \param[in]     size      Number of boxes in \p interface.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_NULL_POINTER if \p handler or \p interface is NULL,
 *     or if \p size is 0.
 */
enum RasterReturnCode raster_api_set_interface(struct RasterHandler *handler, struct RasterBox *interface, uint16_t size);

/*!
 * \brief Render the current interface.
 *
 * \details In partial mode (clear callback unset) only boxes whose \c
 *     updated flag is set are redrawn, and the flag is cleared after each
 *     successful redraw. In full-redraw mode (clear callback set) the clear
 *     callback runs first and every box is drawn afterwards.
 *
 * \param[in] handler Handler to render.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_NULL_POINTER if \p handler, \p handler->draw, or
 *     \p handler->interface is NULL.
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
 * \return Pointer to the matching box, or NULL if no box has that id or if
 *     \p boxes is NULL.
 */
struct RasterBox *raster_api_get_box(struct RasterBox *boxes, uint16_t size, uint16_t id);

/*!
 * \brief Initialize a RasterLabel from explicit components.
 *
 * \param[out] label  Label to initialize.
 * \param[in]  data   Initial value.
 * \param[in]  type   Discriminator for \p data and \p format.
 * \param[in]  pos    Anchor position relative to the owning box.
 * \param[in]  font   Font used for rendering. Must be non-NULL.
 * \param[in]  size   Target pixel height of the rendered text.
 * \param[in]  align  Horizontal alignment around \p pos.
 * \param[in]  color  Text color.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_NULL_POINTER if \p label or \p font is NULL.
 */
enum RasterReturnCode raster_api_create_label(struct RasterLabel *label, union RasterLabelData data, enum RasterLabelDataType type, struct RasterCoords pos, const struct Font *font, uint16_t size, enum FontAlign align, struct Color color);

/*!
 * \brief Sets the value of a label, setting \p type accordingly to the type of \p value.
 *
 * \param[in,out] box   Box owning the label to update.
 * \param[in]     value New integer value.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_NULL_POINTER if \p box or \p box->label is NULL.
 */
enum RasterReturnCode raster_api_set_label_int(struct RasterBox *box, int32_t value);

/*!
 * \brief Sets the value of a label, setting \p type accordingly to the type of \p value.
 *
 * \param[in,out] box   Box owning the label to update.
 * \param[in]     value New float value.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_NULL_POINTER if \p box or \p box->label is NULL.
 */
enum RasterReturnCode raster_api_set_label_float(struct RasterBox *box, float value);

/*!
 * \brief Sets the value of a label, setting \p type accordingly to the type of \p value.
 *
 * \param[in,out] box    Box owning the label to update.
 * \param[in]     string New string value. The label keeps a pointer to it, so
 *     the storage must live at least as long as the label uses it.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_NULL_POINTER if \p box or \p box->label is NULL.
 */
enum RasterReturnCode raster_api_set_label_string(struct RasterBox *box, char *string);

/*!
 * \brief Build a RasterIntFormat.
 *
 * \details This function also sets the type discriminator in the format union.
 *
 * \param[in,out] box         Box owning the label to update.
 * \param[in]     is_unsigned Whether the integer should be formatted as unsigned.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_NULL_POINTER if \p box or \p box->label is NULL.
 */
enum RasterReturnCode raster_api_set_label_int_format(struct RasterBox *box, bool is_unsigned);

/*!
 * \brief Build a RasterFloatFormat.
 *
 * \details This function also sets the type discriminator in the format union.
 *
 * \param[in,out] box       Box owning the label to update.
 * \param[in]     precision Number of decimal digits to display.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_NULL_POINTER if \p box or \p box->label is NULL.
 */
enum RasterReturnCode raster_api_set_label_float_format(struct RasterBox *box, uint8_t precision);

/*!
 * \brief Build a RasterStringFormat.
 *
 * \details This function also sets the type discriminator in the format union.
 *
 * \param[in,out] box        Box owning the label to update.
 * \param[in]     max_length Maximum number of characters to display, 0 for no limit.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_NULL_POINTER if \p box or \p box->label is NULL.
 */
enum RasterReturnCode raster_api_set_label_string_format(struct RasterBox *box, uint16_t max_length);

#endif // RASTER_API_H
