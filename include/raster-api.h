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
 * \brief Initialize a RasterLabel int from explicit components.
 *
 * \param[out] label  Label to initialize.
 * \param[in]  int32  Initial value.
 * \param[in]  format Printf-style format string, e.g. "%d" or "%04X" (optional).
 * \param[in]  pos    Anchor position relative to the owning box.
 * \param[in]  font   Font used for rendering. Must be non-NULL.
 * \param[in]  size   Target pixel height of the rendered text.
 * \param[in]  align  Horizontal alignment around \p pos.
 * \param[in]  color  Text color.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_NULL_POINTER if \p label or \p font is NULL.
 */
enum RasterReturnCode raster_api_create_label_int32(struct RasterLabel *label, int32_t int32, const char *format, struct RasterCoords pos, const struct Font *font, uint16_t size, enum FontAlign align, struct Color color);

/*!
 * \brief Initialize a RasterLabel int from explicit components.
 *
 * \param[out] label  Label to initialize.
 * \param[in]  uint32 Initial value.
 * \param[in]  format Printf-style format string, e.g. "%d" or "%04X" (optional).
 * \param[in]  pos    Anchor position relative to the owning box.
 * \param[in]  font   Font used for rendering. Must be non-NULL.
 * \param[in]  size   Target pixel height of the rendered text.
 * \param[in]  align  Horizontal alignment around \p pos.
 * \param[in]  color  Text color.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_NULL_POINTER if \p label or \p font is NULL.
 */
enum RasterReturnCode raster_api_create_label_uint32(struct RasterLabel *label, uint32_t uint32, const char *format, struct RasterCoords pos, const struct Font *font, uint16_t size, enum FontAlign align, struct Color color);

/*!
 * \brief Initialize a RasterLabel float from explicit components.
 *
 * \param[out] label  Label to initialize.
 * \param[in]  floating Initial value.
 * \param[in]  format Printf-style format string, e.g. "%f" or "%.2e" (optional).
 * \param[in]  pos    Anchor position relative to the owning box.
 * \param[in]  font   Font used for rendering. Must be non-NULL.
 * \param[in]  size   Target pixel height of the rendered text.
 * \param[in]  align  Horizontal alignment around \p pos.
 * \param[in]  color  Text color.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_NULL_POINTER if \p label or \p font is NULL.
 */
enum RasterReturnCode raster_api_create_label_float(struct RasterLabel *label, float floating, const char *format, struct RasterCoords pos, const struct Font *font, uint16_t size, enum FontAlign align, struct Color color);

/*!
 * \brief Initialize a RasterLabel string from explicit components.
 *
 * \param[out] label  Label to initialize.
 * \param[in]  string Initial value. The label keeps a pointer to it, so
 *    the storage must live at least as long as the label uses it.
 * \param[in]  format Printf-style format string, e.g. "%s" or "%.5s" (optional).
 * \param[in]  pos    Anchor position relative to the owning box.
 * \param[in]  font   Font used for rendering. Must be non-NULL.
 * \param[in]  size   Target pixel height of the rendered text.
 * \param[in]  align  Horizontal alignment around \p pos.
 * \param[in]  color  Text color.
 */
enum RasterReturnCode raster_api_create_label_string(struct RasterLabel *label, char *string, const char *format, struct RasterCoords pos, const struct Font *font, uint16_t size, enum FontAlign align, struct Color color);

/*!
 * \brief Sets the value of a label, setting \p type accordingly to the type of \p value.
 *
 * \param[in,out] box   Box owning the label to update.
 * \param[in]     value New integer value.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_NULL_POINTER if \p box or \p box->label is NULL.
 */
enum RasterReturnCode raster_api_set_label_int32(struct RasterBox *box, int32_t value);

/*!
 * \brief Sets the value of a label, setting \p type accordingly to the type of \p value.
 *
 * \param[in,out] box   Box owning the label to update.
 * \param[in]     value New unsigned integer value.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_NULL_POINTER if \p box or \p box->label is NULL.
 */
enum RasterReturnCode raster_api_set_label_uint32(struct RasterBox *box, uint32_t value);

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
 * \brief Sets the formatting of a label, without changing its value.
 *    The label's \p type is not modified, so the new format
 *    string must be compatible with the current value.
 *    If NULL is passed, the label will be rendered with a default format (e.g. "%d" for integers, "%f" for floats, "%s" for strings).
 *
 * \param[in,out] box         Box owning the label to update.
 * \param[in]     format      New printf-style format string, e.g. "%d" or "%.2e" (can be null).
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_NULL_POINTER if \p box or \p box->label is NULL.
 */
enum RasterReturnCode raster_api_set_label_format(struct RasterBox *box, const char *format);

#endif // RASTER_API_H
