/*!
 * \file label.h
 * \date 2026-05-27
 * \author Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief A drawable text element that references a font.
 *
 * \details A label is a value type: it holds a pointer to a NUL-terminated
 *     extended-ASCII string (the storage is owned by the caller), a font,
 *     and the visual options needed to render the text. Labels are
 *     standalone: \c label_api_draw can draw a label anywhere on screen, and
 *     when a label is attached to a box the box renderer offsets it by
 *     (\c offset_x, \c offset_y) relative to the box's top-left corner.
 *
 *     The offset is signed so the label can extend outside the bounds of
 *     its parent box.
 */

#ifndef LABEL_H
#define LABEL_H

#include "font.h"
#include "colors.h"
#include <stdint.h>

/*!
 * \brief A drawable text element.
 */
struct Label {
    const char *text;             /*!< NUL-terminated extended-ASCII string to render */
    int16_t offset_x;             /*!< X offset from the parent box's top-left, signed so the label can spill out */
    int16_t offset_y;             /*!< Y offset from the parent box's top-left, signed so the label can spill out */
    const struct Font *font;      /*!< Font to render with */
    uint16_t size;                /*!< Pixel height of the rendered text */
    enum FontAlignment alignment; /*!< Horizontal alignment around the anchor */
    struct Color color;           /*!< Foreground color */
};

#endif // LABEL_H
