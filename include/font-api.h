/*!
 * \file font-api.h
 * \date 2024-12-30
 * \author Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Public glyph and text rendering API.
 *
 * \details Renders text by walking a glyph's RLE-compressed SDF data and
 *     emitting filled rectangles through the user-provided callback. Scaling
 *     uses Q16 fixed-point arithmetic, with a fast path when the requested
 *     pixel size matches the font's native size.
 */

#ifndef FONT_API_H
#define FONT_API_H

#include "font.h"
#include "raster.h"

/*!
 * \brief Look up the glyph for a character in a font.
 *
 * \details Convenience wrapper around \c font->find_glyph. Returns NULL on
 *     malformed inputs so callers can chain it without extra checks.
 *
 * \param[in] font       Font to search in. May be NULL.
 * \param[in] character  Character to look up.
 *
 * \return Pointer to the matching glyph, or NULL if \p font is NULL, has no
 *     find function, or the character is not in the font.
 */
const struct FontGlyph *font_api_find_glyph(const struct Font *font, char character);

/*!
 * \brief Render a string at the given anchor position.
 *
 * \param[in] x          Anchor X position. Interpretation depends on \p align.
 * \param[in] y          Top Y position of the rendered text.
 * \param[in] alignment  Horizontal alignment of the text relative to (x, y).
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
enum RasterReturnCode font_api_draw(uint16_t x, uint16_t y, enum FontAlignment alignment, const struct Font *font, const char *text, struct Color color, uint16_t pixel_size, raster_draw_rectangle_callback draw);

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

#endif // FONT_API_H
