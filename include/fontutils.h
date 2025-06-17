/**
 * @file fontutils.h
 * @date 2024-12-30
 * @author Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * @brief Text rendering definitions
 */

#ifndef FONTUTILS_H
#define FONTUTILS_H

#define RASTER_BATCH_GLYPHS 1

#include <stdint.h>
#include "fonts.h"

#if RASTER_BATCH_GLYPHS
typedef void (*draw_batch_at_position_t)(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color, uint8_t *alphas);
#else
/**
 * @brief Callback to draw a pixel row
 * 
 * @param x Position in pixel of the first pixel on x axis
 * @param y Position in pixel of the first pixel on y axis
 * @param lenght Lenght in pixel of the line
 * @param color Color in ARGB8888 format
 */
typedef void (*draw_line_callback_t)(uint16_t x, uint16_t y, uint16_t lenght, uint32_t color);
#endif

/**
 * @brief Represents the alignment of the rendering of the text
 *
 * @details
 *     - FONT_ALIGN_LEFT means that the text rendering will start from Coords.x to the left
 *     - FONT_ALIGN_CENTER means that the text rendering will be centered to Coords.x
 *     - FONT_ALIGN_RIGHT means that the text rendering will end at Coords.x
 */
typedef enum {
    FONT_ALIGN_LEFT,
    FONT_ALIGN_CENTER,
    FONT_ALIGN_RIGHT
} FontAlign;

#endif // FONTUTILS_H
