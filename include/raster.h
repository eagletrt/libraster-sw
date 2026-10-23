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
 * \brief Callback used to blend a single color through a coverage mask.
 *
 * \details Optional (see raster_api_set_mask). Each byte of \p mask is the
 *     coverage (0 = transparent, 255 = opaque) of the matching pixel of the
 *     \p width x \p height destination rectangle, row after row with a
 *     stride of \p width bytes. The final alpha of a pixel is
 *     `mask * color.a / 255`.
 *
 *     It lets the library hand over a whole label at once instead of one
 *     rectangle per anti-aliased run: on STM32 it maps to a single DMA2D
 *     transfer with an A8 foreground and a constant foreground color, where
 *     the rectangle path needs thousands of tiny transfers.
 *
 * \param[in] x      Top-left X position of the rectangle, in pixels.
 * \param[in] y      Top-left Y position of the rectangle, in pixels.
 * \param[in] width  Width of the rectangle (and stride of \p mask), in pixels.
 * \param[in] height Height of the rectangle, in pixels.
 * \param[in] mask   Coverage values, \p width * \p height bytes.
 * \param[in] color  Color to blend; its alpha scales the coverage.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_ERROR on hardware/driver failure.
 */
typedef enum RasterReturnCode (*raster_draw_mask_callback)(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *mask, struct Color color);

/*!
 * \brief Scratch buffer the library renders coverage masks into.
 *
 * \details Owned by the user (the library never allocates). It must hold at
 *     least one row of the widest text; taller text is rendered in bands of
 *     as many rows as fit. It must be reachable by whatever implements the
 *     mask callback (e.g. not in a memory the DMA can't read).
 */
struct RasterMask {
    uint8_t *buffer; /*!< Scratch memory */
    uint32_t size;   /*!< Size of \c buffer, in bytes */
};

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
    uint16_t box_count;    /*!< Number of boxes in \c interface */

    raster_draw_rectangle_callback draw;   /*!< Required: rectangle-fill callback */
    raster_clear_screen_callback clear;    /*!< Optional: when set, full-redraw mode */
    raster_draw_mask_callback draw_mask;   /*!< Optional: when set, labels are blended through \c mask */
    struct RasterMask mask;                /*!< Scratch for \c draw_mask */
};

#endif // RASTER_H
