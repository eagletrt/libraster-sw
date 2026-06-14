/*!
 * \file raster.h
 * \date 2024-12-13
 * \author Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Top-level handle bundling an interface and the rendering callbacks.
 *
 * \details The raster module is stateless: it only orchestrates traversal
 *     over a flat array of boxes (the interface) and delegates the actual
 *     drawing to the box module and to the user-provided callbacks.
 */

#ifndef RASTER_H
#define RASTER_H

#include "box.h"
#include "colors.h"

/*!
 * \brief Return codes for libraster operations.
 */
enum RasterReturnCode {
    RASTER_RC_OK,           /*!< Operation completed successfully */
    RASTER_RC_ERROR,        /*!< Operation failed */
    RASTER_RC_NULL_POINTER, /*!< A required pointer was NULL */
};

/*!
 * \brief Callback used to fill a rectangle with a single color.
 *
 * \details This is the only drawing primitive the user must implement. The
 *     library uses it both for box backgrounds and for glyph rasterization,
 *     where the alpha channel of \p color carries the coverage value. A
 *     horizontal line is just a rectangle with `h == 1`.
 *
 *     On STM32 this maps naturally to a DMA2D fill; on SDL it maps to
 *     SDL_FillRect; on a software framebuffer it is two nested loops.
 *
 * \param[in] x       Top-left X position of the rectangle, in pixels.
 * \param[in] y       Top-left Y position of the rectangle, in pixels.
 * \param[in] width   Width of the rectangle, in pixels.
 * \param[in] heigth  Height of the rectangle, in pixels.
 * \param[in] color Fill color in ARGB.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_ERROR on hardware/driver failure.
 */
typedef enum RasterReturnCode (*raster_draw_rectangle_callback)(uint16_t x, uint16_t y, uint16_t width, uint16_t heigth, struct Color color);

/*!
 * \brief Callback used to clear the entire framebuffer.
 *
 * \details Optional. When set on a RasterHandler, raster_api_render switches
 *     to full-redraw mode: every box is drawn each frame after a clear. When
 *     unset, only boxes flagged as updated are drawn (partial mode).
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_ERROR on hardware/driver failure.
 */
typedef enum RasterReturnCode (*raster_clear_screen_callback)(void);

/*!
 * \brief Top-level handle bundling an interface and the rendering callbacks.
 *
 * \details The clear callback selects the render mode at runtime:
 *     - When \c clear is NULL the renderer is in partial mode and only
 *       boxes whose \c updated flag is set are redrawn (the flag is
 *       cleared after each successful redraw).
 *     - When \c clear is not NULL the renderer is in full-redraw mode and
 *       every box is drawn each frame after the clear callback has run.
 */
struct RasterHandler {
    struct Box *interface; /*!< Currently mounted interface */
    uint16_t box_count;         /*!< Number of boxes in \c interface */

    raster_draw_rectangle_callback draw; /*!< Required: rectangle-fill callback */
    raster_clear_screen_callback clear;  /*!< Optional: when set, full-redraw mode */
};

#endif // RASTER_H
