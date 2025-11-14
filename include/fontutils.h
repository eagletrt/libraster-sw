/*!
 * \file fontutils.h
 * \date 2024-12-30
 * \author Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Text rendering definitions
 */

#ifndef FONTUTILS_H
#define FONTUTILS_H

#include <stdint.h>
#include "fonts.h"
#ifdef __has_include
#if __has_include("raster_config.h")
#include "raster_config.h"
#endif
#endif

#ifndef PARTIAL_RASTER
/*!
 * \brief Enable partial raster rendering by default
 *
 * \details This macro enables partial raster rendering if it is not
 *     already defined. Partial raster rendering allows for more efficient
 *     drawing of text by only updating the necessary parts of the screen.
 */
#define PARTIAL_RASTER 1
#endif

/*!
 * \brief Represents a color in ARGB format
 *
 * \details This structure represents a color using a union that allows
 *      access to the color as a single 32-bit integer in ARGB format or
 *      as individual 8-bit components for alpha, red, green, and blue.
 */
struct Color {
    union {
        uint32_t argb; /*!< Color in ARGB format */
        struct {
            uint8_t b;  /*!< Blue component */
            uint8_t g;  /*!< Green component */
            uint8_t r;  /*!< Red component */
            uint8_t a;  /*!< Alpha component */
        };
    };
};

/*!
 * \brief Callback type for drawing a horizontal line of pixels
 *
 * \details This callback function is used to draw a horizontal line
 *      of pixels on the screen. The function takes the starting X and Y
 *      positions, the length of the line, and the color of the line in
 *      ARGB format.
 *
 * \param[in] x X position to start drawing
 * \param[in] y Y position to draw
 * \param[in] lenght Length of the line to draw
 * \param[in] color Color of the line (ARGB format)
 */
typedef void (*draw_line_callback)(uint16_t x, uint16_t y, uint16_t lenght, struct Color color);

/*!
 * \brief Represents the alignment of the rendering of the text
 *
 * \details This enumeration defines the possible alignments for text rendering,
 *     including left, center, and right alignment.
 */
enum FontAlign {
    FONT_ALIGN_LEFT,   /*!< Left alignment */
    FONT_ALIGN_CENTER, /*!< Center alignment */
    FONT_ALIGN_RIGHT   /*!< Right alignment */
};

#endif // FONTUTILS_H
