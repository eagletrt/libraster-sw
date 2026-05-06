/*!
 * \file fontutils-api.h
 * \date 2024-03-21
 * \author Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Public glyph rendering API.
 *
 * \details Renders text by walking a glyph's RLE-compressed SDF data and
 *     emitting filled rectangles through the user-provided callback. Scaling
 *     uses Q16 fixed-point arithmetic, with a fast path when the requested
 *     pixel size matches the font's native size.
 */

#ifndef FONTUTILS_API_H
#define FONTUTILS_API_H

#include "fontutils.h"

/*!
 * \brief Render a string at the given anchor position.
 *
 * \param[in] x          Anchor X position. Interpretation depends on \p align.
 * \param[in] y          Top Y position of the rendered text.
 * \param[in] align      Horizontal alignment of the text relative to (x, y).
 * \param[in] font       Font to render with.
 * \param[in] text       NUL-terminated string to render.
 * \param[in] color      Base color; the alpha channel is replaced per pixel
 *                       by the glyph's coverage value.
 * \param[in] pixel_size Target pixel height of the rendered text.
 * \param[in] draw       Rectangle-fill callback used for every glyph span.
 *
 * \retval RASTER_RC_OK if rendering succeeded (or the string was empty).
 * \retval RASTER_RC_NULL_POINTER if \p font, \p text, or \p draw is NULL.
 * \retval RASTER_RC_ERROR if the draw callback reported an error.
 */
enum RasterReturnCode font_api_draw(uint16_t x, uint16_t y, enum FontAlign align, const struct Font *font, const char *text, struct Color color, uint16_t pixel_size, raster_draw_rectangle_callback draw);

/*!
 * \brief Compute the rendered pixel width of a string.
 *
 * \param[in] text       NUL-terminated string to measure.
 * \param[in] pixel_size Target pixel height.
 * \param[in] font       Font to measure with.
 *
 * \return Width in pixels, or 0 if any argument is invalid or the string is empty.
 */
uint16_t font_api_length(const char *text, uint16_t pixel_size, const struct Font *font);

#endif // FONTUTILS_API_H
