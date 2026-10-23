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
 * \brief Look up the glyph for a Unicode codepoint in a font.
 *
 * \details Convenience wrapper around \c font->find_glyph. Returns NULL on
 *     malformed inputs so callers can chain it without extra checks.
 *
 * \param[in] font      Font to search in. May be NULL.
 * \param[in] codepoint Unicode codepoint to look up.
 *
 * \return Pointer to the matching glyph, or NULL if \p font is NULL, has no
 *     find function, or the codepoint is not in the font.
 */
const struct FontGlyph *font_api_find_glyph(const struct Font *font, uint32_t codepoint);

/*!
 * \brief Render a string at the given anchor position.
 *
 * \param[in] x          Anchor X position. Interpretation depends on \p align.
 * \param[in] y          Top Y position of the rendered text.
 * \param[in] alignment  Horizontal alignment of the text relative to (x, y).
 * \param[in] font       Font to render with.
 * \param[in] text       NUL-terminated UTF-8 string to render. Invalid
 *                       UTF-8 bytes are skipped one byte at a time.
 * \param[in] color      Base color; the alpha channel is replaced per pixel
 *                       by the glyph's coverage value.
 * \param[in] pixel_size Target pixel height of the rendered text.
 * \param[in] draw       Rectangle-fill callback used for every glyph span.
 *
 * \warning The \p text parameter is not copied; the caller must ensure that the string storage
 *    remains valid for the duration of the box's usage. The string is expected to be null-terminated.
 *
 * \retval RASTER_RC_OK if rendering succeeded (or the string was empty).
 * \retval RASTER_RC_NULL_POINTER if \p font, \p text, or \p draw is NULL.
 * \retval RASTER_RC_ERROR if the draw callback reported an error.
 */
enum RasterReturnCode font_api_draw(uint16_t x, uint16_t y, enum FontAlignment alignment, const struct Font *font, const char *text, struct Color color, uint16_t pixel_size, raster_draw_rectangle_callback draw);

/*!
 * \brief Compute the rendered pixel width of a string.
 *
 * \param[in] text       NUL-terminated UTF-8 string to measure. Invalid
 *                       UTF-8 bytes are skipped one byte at a time.
 * \param[in] pixel_size Target pixel height.
 * \param[in] font       Font to measure with.
 *
 * \warning The \p text parameter is not copied; the caller must ensure that the string storage
 *    remains valid for the duration of the box's usage. The string is expected to be null-terminated.
 *
 * \return Width in pixels, or 0 if any argument is invalid or the string is empty.
 */
uint16_t font_api_length(const char *text, uint16_t pixel_size, const struct Font *font);

/*!
 * \brief Compute the screen rectangle a string covers when rendered.
 *
 * \param[in]  x          Anchor X position. Interpretation depends on \p alignment.
 * \param[in]  y          Top Y position of the rendered text.
 * \param[in]  alignment  Horizontal alignment of the text relative to (x, y).
 * \param[in]  font       Font to render with.
 * \param[in]  text       NUL-terminated UTF-8 string to measure.
 * \param[in]  pixel_size Target pixel height of the rendered text.
 * \param[out] bounds     Left edge (after alignment), top, width and height. Width and height are 0
 *     for an empty string or a font without glyphs for it.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_NULL_POINTER if \p font, \p text, or \p bounds is NULL.
 */
enum RasterReturnCode font_api_bounds(uint16_t x, uint16_t y, enum FontAlignment alignment, const struct Font *font, const char *text, uint16_t pixel_size, struct FontBounds *bounds);

/*!
 * \brief Render a string through a coverage mask.
 *
 * \details Same output as font_api_draw, but the glyph runs are written into
 *     \p mask (the highest coverage wins where runs overlap) and handed to
 *     \p draw_mask once per band instead of one rectangle per run. A band is
 *     as many rows of the text's bounds as fit in the mask; with a mask of at
 *     least width * height bytes the whole string is one call. Not reentrant.
 *
 * \param[in] x          Anchor X position. Interpretation depends on \p alignment.
 * \param[in] y          Top Y position of the rendered text.
 * \param[in] alignment  Horizontal alignment of the text relative to (x, y).
 * \param[in] font       Font to render with.
 * \param[in] text       NUL-terminated UTF-8 string to render.
 * \param[in] color      Text color; its alpha scales the coverage.
 * \param[in] pixel_size Target pixel height of the rendered text.
 * \param[in] mask       Scratch buffer, at least one row of the text wide.
 * \param[in] draw_mask  Mask-blend callback.
 *
 * \retval RASTER_RC_OK if rendering succeeded (or the string was empty).
 * \retval RASTER_RC_NULL_POINTER if a pointer argument (or \c mask->buffer) is NULL.
 * \retval RASTER_RC_ERROR if the mask is narrower than the text or the callback reported an error.
 */
enum RasterReturnCode font_api_draw_masked(uint16_t x, uint16_t y, enum FontAlignment alignment, const struct Font *font, const char *text, struct Color color, uint16_t pixel_size, const struct RasterMask *mask, raster_draw_mask_callback draw_mask);

#endif // FONT_API_H
