/*!
 * \file raster.h
 * \date 2024-12-13
 * \author Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Graphics handling structures and types
 */

#ifndef RASTER_H
#define RASTER_H

#include <stdbool.h>
#include <stdint.h>
#include "fontutils.h"
#include "fonts.h"

/**
 * \brief Represents a rectangle area
 */
struct RasterRect {
    uint16_t x; /*!< X position of the rectangle */
    uint16_t y; /*!< Y position of the rectangle */
    uint16_t w; /*!< Width of the rectangle */
    uint16_t h; /*!< Height of the rectangle */
};

/*!
 * \brief Represents a pair of coordinates
 */
struct RasterCoords {
    uint16_t x; /*!< X position in pixels */
    uint16_t y; /*!< Y position in pixels */
};

/*!
 * \brief Union to hold different types of label values
 */
union RasterLabelData {
    char *text;      /*!< Text to display */
    int32_t int_val; /*!< Integer value to display */
    float float_val; /*!< Float value to display */
};

/*!
 * \brief Defines the data type to display
 */
enum RasterLabelDataType {
    LABEL_DATA_STRING, /*!< Text value type */
    LABEL_DATA_INT,    /*!< Integer value type */
    LABEL_DATA_FLOAT   /*!< Float value type */
};

/*!
 * \brief Formatting options for integer values
 */
struct RasterIntFormat {
    bool is_unsigned; /*!< Treat as unsigned integer */
};

/*!
 * \brief Formatting options for floating-point values
 */
struct RasterFloatFormat {
    uint8_t precision; /*!< Number of digits after decimal point */
};

/*!
 * \brief Formatting options for string values
 */
struct RasterStringFormat {
    uint16_t max_length; /*!< Maximum string length (0 for no limit) */
};

/*!
 * \brief Union of formatting options for different data types
 */
union RasterLabelFormat {
    struct RasterIntFormat int_fmt;       /*!< Integer formatting options */
    struct RasterFloatFormat float_fmt;   /*!< Float formatting options */
    struct RasterStringFormat string_fmt; /*!< String formatting options */
};

/*!
 * \brief Function used to draw a rectangle on screen
 * 
 * \details This callback function is used to draw a filled rectangle
 *
 * \param[in] x X position of the rectangle
 * \param[in] y Y position of the rectangle
 * \param[in] w Width of the rectangle
 * \param[in] h Height of the rectangle
 * \param[in] color Color of the rectangle, ARGB format
 */
typedef void (*raster_draw_rectangle_callback)(uint16_t x, uint16_t y, uint16_t w, uint16_t h, struct Color color);

/*!
 * \brief Function used to clear the screen
 * 
 * \details This callback function is used to clear the whole screen
 */
typedef void (*raster_clear_screen_callback)(void);

/*!
 * \brief Defines a label to be drawn on screen
 */
struct RasterLabel {
    union RasterLabelData data;     /*!< Content of the label */
    enum RasterLabelDataType type;  /*!< Type of the label content */
    union RasterLabelFormat format; /*!< Formatting options for the label */
    struct RasterCoords pos;        /*!< Position to draw the label */
    enum FontName font;             /*!< Font name, defined in font.h */
    uint16_t size;                  /*!< Size of the text */
    enum FontAlign align;           /*!< Alignement of the text relative to coords */
    struct Color color;             /*!< Color of the text */
};

/*!
 * \brief Defines a text box to be drawn on screen
 */
struct RasterBox {
    bool updated;              /*!< Flag to indicate if the box needs to be redrawn */
    uint16_t id;               /*!< Unique identifier for the box */
    struct RasterRect rect;    /*!< Rectangle area of the box */
    struct Color color;        /*!< Default background color of the box (ARGB format) */
    struct RasterLabel *label; /*!< Pointer to a Label structure (can be NULL) */
};

/*!
 * \brief Handler for raster operations
 */
struct RasterHandler {
    struct RasterBox *interface; /*!< Pointer to an array of RasterBox structures */
    uint16_t size;               /*!< Number of boxes in the interface array */

    font_draw_line_callback draw_line;             /*!< Callback to draw a horizontal line */
    raster_draw_rectangle_callback draw_rectangle; /*!< Callback to draw a filled rectangle */
    raster_clear_screen_callback clear_screen;     /*!< Callback to clear the screen */
};

#endif // RASTER_H
