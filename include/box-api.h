/*!
 * \file box-api.h
 * \date 2026-05-27
 * \author Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Public API for creating, manipulating and rendering boxes.
 */

#ifndef BOX_API_H
#define BOX_API_H

#include "box.h"
#include "raster.h"

/*!
 * \brief Initialize a Box from explicit components.
 *
 * \details The \c updated flag is set so the box is drawn at the next
 *     partial-mode render.
 *
 * \param[out] box     Box to initialize.
 * \param[in]  box_id  Caller-defined identifier used by \c box_api_find.
 * \param[in]  rect    Geometry on screen.
 * \param[in]  color   Background color.
 * \param[in]  label   Optional label, or NULL.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_NULL_POINTER if \p box is NULL.
 */
enum RasterReturnCode box_api_init(struct Box *box, uint16_t box_id, struct BoxRectangle rect, struct Color color, struct Label *label);

/*!
 * \brief Reposition a box without altering its size or label content.
 *
 * \param[in,out] box Box to reposition.
 * \param[in]     x   New top-left X.
 * \param[in]     y   New top-left Y.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_NULL_POINTER if \p box is NULL.
 */
enum RasterReturnCode box_api_set_position(struct Box *box, uint16_t x, uint16_t y);

/*!
 * \brief Attach a label to a box.
 *
 * \details The box stores the pointer as-is; the label storage must outlive
 *     the box's usage. Marks the box as updated so the change is visible on
 *     the next render.
 *
 * \param[in,out] box   Box to update.
 * \param[in]     label Label to attach. May be NULL (equivalent to
 *     \c box_api_clear_label).
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_NULL_POINTER if \p box is NULL.
 */
enum RasterReturnCode box_api_set_label(struct Box *box, struct Label *label);

/*!
 * \brief Update the label's text content.
 *
 * \details The box stores the pointer as-is; the label storage must outlive
 *     the box's usage. Marks the box as updated so the change is visible.
 *
 * \param[in,out] box Box to update.
 * \param[in]     text New text content for the label. May be NULL (equivalent to \c box_api_clear_label).
 *
 * \warning The \p text parameter is not copied; the caller must ensure that the string storage
 *    remains valid for the duration of the box's usage. The string is expected to be null-terminated.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_NULL_POINTER if \p box is NULL.
 * \retval RASTER_RC_ERROR if \p box has no label attached.
 */
enum RasterReturnCode box_api_set_label_text(struct Box *box, const char *text);

/*!
 * \brief Detach the label from a box.
 *
 * \param[in,out] box Box to update.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_NULL_POINTER if \p box is NULL.
 */
enum RasterReturnCode box_api_clear_label(struct Box *box);

/*!
 * \brief Render a box: background fill followed by the optional label.
 *
 * \details The label, when present, is placed at
 *     (box.rect.x + label.offset_x, box.rect.y + label.offset_y). Does not
 *     touch the \c updated flag; the raster module is responsible for
 *     clearing it in partial mode.
 *
 * \param[in] box  Box to render.
 * \param[in] draw Rectangle-fill callback.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_NULL_POINTER if \p box or \p draw is NULL.
 * \retval RASTER_RC_ERROR if a callback reported an error.
 */
enum RasterReturnCode box_api_draw(const struct Box *box, raster_draw_rectangle_callback draw);

/*!
 * \brief Find a box by id within a flat array.
 *
 * \details Linear search; convenient for small interfaces (the typical case
 *     for embedded UIs) and decoupled from any particular handler.
 *
 * \param[in] boxes   Array to search.
 * \param[in] size    Number of entries in \p boxes.
 * \param[in] box_id  Identifier to look up.
 *
 * \return Pointer to the matching box, or NULL if no box has that id or if
 *     \p boxes is NULL.
 */
struct Box *box_api_find(struct Box *boxes, uint16_t size, uint16_t box_id);

#endif // BOX_API_H
