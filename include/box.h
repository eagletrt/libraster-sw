/*!
 * \file box.h
 * \date 2026-05-27
 * \author Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Rectangular region with a background color and an optional label.
 *
 * \details A box is a value type. It owns its geometry, background color
 *     and an updated flag used by the raster to decide whether the box
 *     needs to be redrawn in partial mode. The optional label is referenced
 *     by pointer, so the same label storage can be shared between boxes if
 *     needed and a box can be moved without invalidating its label content.
 */

#ifndef BOX_H
#define BOX_H

#include "label.h"
#include <stdbool.h>
#include <stdint.h>

/*!
 * \brief A rectangle in pixel coordinates.
 */
struct BoxRectangle {
    uint16_t x;      /*!< Top-left X position */
    uint16_t y;      /*!< Top-left Y position */
    uint16_t width;  /*!< Width in pixels */
    uint16_t height; /*!< Height in pixels */
};

/*!
 * \brief A rectangle filled with a background color and an optional label.
 */
struct Box {
    bool updated;             /*!< Set to true to request a redraw on the next render (only used if clear callback is not NULL) */
    uint16_t id;              /*!< Caller-defined identifier */
    struct BoxRectangle rect; /*!< Box geometry on screen */
    struct Color color;       /*!< Background color */
    struct Label *label;      /*!< Optional label, NULL for an empty box */
};

#endif // BOX_H
