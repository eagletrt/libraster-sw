/*!
 * \file raster.h
 * \date 2024-12-13
 * \author Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Box and label structures used to describe a raster interface.
 *
 * \details An interface is a flat array of RasterBox. Each box owns its
 *     rectangle and an optional label. The library renders the interface
 *     using a single user-provided rectangle-fill callback. Whether a frame
 *     is a partial redraw or a full clear-and-redraw is decided at runtime
 *     by the presence of a clear callback on the handler.
 */

#ifndef RASTER_H
#define RASTER_H

#include "fontutils.h"
#include <stdbool.h>
#include <stdint.h>

/*!
 * \brief A rectangle in pixel coordinates.
 */
struct RasterRect {
    uint16_t x; /*!< Top-left X position */
    uint16_t y; /*!< Top-left Y position */
    uint16_t w; /*!< Width in pixels */
    uint16_t h; /*!< Height in pixels */
};

/*!
 * \brief A pair of pixel coordinates.
 */
struct RasterCoords {
    uint16_t x; /*!< X position in pixels */
    uint16_t y; /*!< Y position in pixels */
};

/*!
 * \brief Discriminator for the value stored in a RasterLabel.
 */
enum RasterLabelDataType {
    RASTER_LABEL_DATA_STRING, /*!< Label holds a C string */
    RASTER_LABEL_DATA_INT,    /*!< Label holds a 32-bit integer */
    RASTER_LABEL_DATA_FLOAT,  /*!< Label holds a single-precision float */
};

/*!
 * \brief Active member of RasterLabel::data.
 */
union RasterLabelData {
    char *text;      /*!< Used when type == RASTER_LABEL_DATA_STRING */
    int32_t int_val; /*!< Used when type == RASTER_LABEL_DATA_INT */
    float float_val; /*!< Used when type == RASTER_LABEL_DATA_FLOAT */
};

/*!
 * \brief Formatting options for integer labels.
 */
struct RasterIntFormat {
    bool is_unsigned; /*!< Treat the value as unsigned */
};

/*!
 * \brief Formatting options for float labels.
 */
struct RasterFloatFormat {
    uint8_t precision; /*!< Digits after the decimal point */
};

/*!
 * \brief Formatting options for string labels.
 */
struct RasterStringFormat {
    uint16_t max_length; /*!< Truncation length, 0 means no limit */
};

/*!
 * \brief Active member of RasterLabel::format.
 */
union RasterLabelFormat {
    struct RasterIntFormat int_fmt;       /*!< Used when type == RASTER_LABEL_DATA_INT */
    struct RasterFloatFormat float_fmt;   /*!< Used when type == RASTER_LABEL_DATA_FLOAT */
    struct RasterStringFormat string_fmt; /*!< Used when type == RASTER_LABEL_DATA_STRING */
};

/*!
 * \brief A drawable text element rendered inside a box.
 */
struct RasterLabel {
    union RasterLabelData data;     /*!< Value to display */
    enum RasterLabelDataType type;  /*!< Discriminator for \c data and \c format */
    union RasterLabelFormat format; /*!< Formatting options for \c data */
    struct RasterCoords pos;        /*!< Anchor position relative to the box */
    const struct Font *font;        /*!< Font to render with */
    uint16_t size;                  /*!< Pixel height of the text */
    enum FontAlign align;           /*!< Horizontal alignment around \c pos */
    struct Color color;             /*!< Text color */
};

/*!
 * \brief A rectangle filled with a background color and an optional label.
 */
struct RasterBox {
    bool updated;              /*!< Set to true to request a redraw on the next render */
    uint16_t id;               /*!< Caller-defined identifier */
    struct RasterRect rect;    /*!< Box geometry on screen */
    struct Color color;        /*!< Background color */
    struct RasterLabel *label; /*!< Optional label, NULL for an empty box */
};

/*!
 * \brief Top-level handle bundling an interface and the rendering callbacks.
 *
 * \details The clear callback selects the render mode at runtime:
 *     - When \c clear is NULL the renderer is in partial mode and only
 *       boxes whose \c updated flag is set are redrawn (the flag is
 *       cleared after each successful redraw).
 *     - When \c clear is not NULL the renderer is in full-redraw mode and
 *       every box is drawn each frame after the clear callback has run.
 */
struct RasterHandler {
    struct RasterBox *interface; /*!< Currently mounted interface */
    uint16_t size;               /*!< Number of boxes in \c interface */

    raster_draw_rectangle_callback draw; /*!< Required: rectangle-fill callback */
    raster_clear_screen_callback clear;  /*!< Optional: when set, full-redraw mode */
};

#endif // RASTER_H
