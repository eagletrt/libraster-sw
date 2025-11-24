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
#include "fonts.h"

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
 * \param[in] size Of which size (vertically) the text is to be rendere
 * \param[in] line_callback Callback used to draw a line
 */
void font_api_draw(uint16_t x, uint16_t y, enum FontAlign align, enum FontName font, const char *__restrict__ text, struct Color color, uint16_t size, font_draw_line_callback line_callback);

/*!
 * \brief Calculate the length of the text in pixel
 *
 * \details Based on text, scaling and font calculates the length of the
 *      rendered text. This is useful to align horizontally the text.
 *
 * \param[in] text The string to be rendered
 * \param[in] size Of which size (vertically) the text is to be rendered
 * \param[in] font Which font to be used
 */
uint16_t font_api_length(const char *__restrict__ text, uint16_t size, enum FontName font);

#endif // FONTUTILS_API_H
