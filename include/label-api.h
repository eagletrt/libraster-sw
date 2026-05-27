/*!
 * \file label-api.h
 * \date 2026-05-27
 * \author Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Public API for creating and rendering labels.
 */

#ifndef LABEL_API_H
#define LABEL_API_H

#include "label.h"
#include "raster.h"

/*!
 * \brief Initialize a Label from explicit components.
 *
 * \param[out] label     Label to initialize.
 * \param[in]  text      Pointer to the string to render. The label keeps the
 *     pointer, so the storage must outlive the label's usage. May be NULL.
 * \param[in]  offset_x  X offset from the parent box's top-left when the
 *     label is rendered through a box. Ignored by \c label_api_draw.
 * \param[in]  offset_y  Y offset from the parent box's top-left when the
 *     label is rendered through a box. Ignored by \c label_api_draw.
 * \param[in]  font      Font used for rendering. Must be non-NULL.
 * \param[in]  size      Target pixel height of the rendered text.
 * \param[in]  alignment Horizontal alignment around the anchor.
 * \param[in]  color     Foreground color.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_NULL_POINTER if \p label or \p font is NULL.
 */
enum RasterReturnCode label_api_init(struct Label *label, const char *text, int16_t offset_x, int16_t offset_y, const struct Font *font, uint16_t size, enum FontAlignment alignment, struct Color color);

/*!
 * \brief Replace the text pointer carried by a label.
 *
 * \details The label keeps the pointer, so the storage must outlive its use.
 *     Passing NULL is allowed and causes the label to render nothing.
 *
 * \param[in,out] label Label to update.
 * \param[in]     text  New text pointer, or NULL.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_NULL_POINTER if \p label is NULL.
 */
enum RasterReturnCode label_api_set_text(struct Label *label, const char *text);

/*!
 * \brief Render a label at an explicit target position.
 *
 * \details The label's \c offset_x and \c offset_y are ignored; (x, y) is
 *     used directly as the anchor for the configured horizontal alignment.
 *
 * \param[in] label Label to render.
 * \param[in] x     Anchor X position in framebuffer coordinates.
 * \param[in] y     Top Y position of the rendered text.
 * \param[in] draw  Rectangle-fill callback used for every glyph span.
 *
 * \retval RASTER_RC_OK on success or when the label has nothing to draw.
 * \retval RASTER_RC_NULL_POINTER if \p label or \p draw is NULL.
 * \retval RASTER_RC_ERROR if the draw callback reported an error.
 */
enum RasterReturnCode label_api_draw(const struct Label *label, uint16_t x, uint16_t y, raster_draw_rectangle_callback draw);

#endif // LABEL_API_H
