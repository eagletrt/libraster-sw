/**
 * @file fontutils-api.h
 * @date 2024-03-21
 * @author Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * @brief Text rendering APIs
 */

#ifndef FONTUTILS_API_H
#define FONTUTILS_API_H

#include "fontutils.h"

/**
 * @brief Draws text
 *
 * @param x Position on x axis in buffer
 * @param y Position on y axis in buffer
 * @param align Alignment on x axis
 * @param font Font name described (uppercase)
 * @param text Pointer to text to draw
 * @param color ARGB color value (alpha is ignored)
 * @param pixel_size Size of the font to render
 * @param line_callback Callback used to draw a line
 */
void draw_text(uint16_t x, uint16_t y, FontAlign align, FontName font, const char *text, uint32_t color, uint16_t pixel_size, draw_line_callback_t line_callback);

/**
 * @brief Calcaulate the lenght on the text in pixel
 */
uint16_t text_length(const char *text, uint16_t pixel_size, FontName font);

/**
 * @brief Utility to extract 8 bit alpha value from ARGB8888 format
 *
 * @param color Color value
 * @return alpha value
 */
uint8_t get_alpha(uint32_t color);

/**
 * @brief Utility to extract 8 bit red value from ARGB8888 format
 *
 * @param color Color value
 * @return red value
 */
uint8_t get_red(uint32_t color);

/**
 * @brief Utility to extract 8 bit green value from ARGB8888 format
 *
 * @param color Color value
 * @return green value
 */
uint8_t get_green(uint32_t color);

/**
 * @brief Utility to extract 8 bit blue value from ARGB8888 format
 *
 * @param color Color value
 * @return blue value
 */
uint8_t get_blue(uint32_t color);

#endif // FONTUTILS_API_H
