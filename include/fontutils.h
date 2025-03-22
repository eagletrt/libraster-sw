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
#include "fonts.h"

/**
 * @brief Callback used to draw pixel
 */
typedef void (*draw_line_callback_t)(uint16_t x, uint16_t y, uint16_t lenght, uint32_t color);

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
