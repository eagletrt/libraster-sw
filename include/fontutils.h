/**
 * @file fontutils.h
 * @date 2024-12-30
 * @author Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * @brief Text rendering definitions
 */

#ifndef FONTUTILS_H
#define FONTUTILS_H

#include <stdint.h>

/**
 * @brief Callback to draw a pixel row
 * 
 * @param x Position in pixel of the first pixel on x axis
 * @param y Position in pixel of the first pixel on y axis
 * @param lenght Lenght in pixel of the line
 * @param color Color in ARGB8888 format
 */
typedef void (*draw_line_callback_t)(uint16_t x, uint16_t y, uint16_t lenght, uint32_t color);

/**
 * @brief Callback that tells the user the rendering is finished and ready to be copied in the frambebuffer
 *
 * @param x Position in pixel on x axis where to copy the rendered glyph
 * @param y Position in pixel on y axis where to copy the rendered glyph
 * @param width Width of the rendered glyph
 * @param height Height of the rendered glyph
 */
typedef void (*finished_rendering_callback_t)(uint16_t x, uint16_t y, uint16_t width, uint16_t height);

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
