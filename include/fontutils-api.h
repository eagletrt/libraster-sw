/*!
 * \file fontutils-api.h
 * \date 2024-03-21
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief FontUtils APIs functions implementations
 *
 * \details A rasterizer is a software renderer that works by writing pixels to
 *      a framebuffer.
 *      This implementation lets the user define 3 callbacks that defines the
 *      technique used to write them, so that hardware accelerators can be used
 *      to archive big speedups.
 */

#ifndef FONTUTILS_API_H
#define FONTUTILS_API_H

#include "fontutils.h"

/*!
 * \brief Draws text
 *
 * \details This function puts together all the functionalities offered by the
 *      file to give a clear and simple API used to draw text in a framebuffer.
 *
 * \param[in] x Position on x axis in buffer
 * \param[in] y Position on y axis in buffer
 * \param[in] align Alignment on x axis
 * \param[in] font Font name described (uppercase)
 * \param[in] text Pointer to text to draw
 * \param[in] color ARGB color value (alpha is ignored)
 * \param[in] pixel_size Size of the font to render
 * \param[in] line_callback Callback used to draw a line
 */
void draw_text(uint16_t x, uint16_t y, enum FontAlign align, enum FontName font, const char *text, uint32_t color, uint16_t pixel_size, draw_line_callback_t line_callback);

/*!
 * \brief Calcaulate the length on the text in pixel
 *
 * \details Based on text, scaling and font calculates the length of the
 *      rendered text. This is useful to align horizontally the text.
 *
 * \param[in] text The string to be rendered
 * \param[in] pixel_size Of which size (vertically) the text is to be rendered
 * \param[in] font Which font to be used
 */
uint16_t text_length(const char *text, uint16_t pixel_size, enum FontName font);

/*!
 * \brief Utility to extract 8 bit alpha value from ARGB8888 format
 *
 * \param[in] color Color value
 *
 * \return alpha value
 */
uint8_t get_alpha(uint32_t color);

/*!
 * \brief Utility to extract 8 bit red value from ARGB8888 format
 *
 * \param[in] color Color value
 *
 * \return red value
 */
uint8_t get_red(uint32_t color);

/*!
 * \brief Utility to extract 8 bit green value from ARGB8888 format
 *
 * \param[in] color Color value
 *
 * \return green value
 */
uint8_t get_green(uint32_t color);

/*!
 * \brief Utility to extract 8 bit blue value from ARGB8888 format
 *
 * \param[in] color Color value
 *
 * \return blue value
 */
uint8_t get_blue(uint32_t color);

#endif // FONTUTILS_API_H
