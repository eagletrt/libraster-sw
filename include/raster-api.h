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
 *     used transparently. No dynamic allocation is performed and the
 *     handler holds no internal state beyond the user-provided pointers.
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
 * \param[in]  interface Interface (flat array of boxes) to mount.
 * \param[in]  box_count Number of boxes in \p interface.
 * \param[in]  draw      Rectangle-fill callback.
 * \param[in]  clear     Clear-screen callback, or NULL to use partial mode.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_NULL_POINTER if \p handler, \p interface, or \p draw is
 *     NULL, or if \p size is 0.
 */
enum RasterReturnCode raster_api_init(struct RasterHandler *handler, struct Box *interface, uint16_t box_count, raster_draw_rectangle_callback draw, raster_clear_screen_callback clear);

/*!
 * \brief Enable (or disable) rendering labels through a coverage mask.
 *
 * \details With a mask callback set, raster_api_render draws every label as
 *     one mask blend per band (box_api_draw_masked) instead of one rectangle
 *     per anti-aliased run, which is much cheaper when each callback has a
 *     fixed cost (e.g. one DMA2D transfer). Box backgrounds still use the
 *     rectangle callback. raster_api_init leaves it disabled.
 *
 * \param[in,out] handler   Handler to update.
 * \param[in]     draw_mask Mask-blend callback, or NULL to go back to rectangles only.
 * \param[in]     buffer    Scratch for the coverage (ignored when \p draw_mask is NULL).
 *     Must hold at least one row of the widest label.
 * \param[in]     size      Size of \p buffer, in bytes.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_NULL_POINTER if \p handler is NULL, or if \p draw_mask is set
 *     but \p buffer is NULL or \p size is 0.
 */
enum RasterReturnCode raster_api_set_mask(struct RasterHandler *handler, raster_draw_mask_callback draw_mask, uint8_t *buffer, uint32_t size);

/*!
 * \brief Replace the interface mounted on a handler.
 *
 * \details Useful for swapping screens (main view, popup, menu, etc.).
 *
 * \param[in,out] handler   Handler to update.
 * \param[in]     interface New interface to mount.
 * \param[in]     box_count Number of boxes in \p interface.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_NULL_POINTER if \p handler or \p interface is NULL,
 *     or if \p size is 0.
 */
enum RasterReturnCode raster_api_set_interface(struct RasterHandler *handler, struct Box *interface, uint16_t box_count);

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

#endif // RASTER_API_H
