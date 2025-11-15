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

/**
 * \brief Represents a rectangle area
 */
struct Rect {
    uint16_t x; /*!< X position of the rectangle */
    uint16_t y; /*!< Y position of the rectangle */
    uint16_t w; /*!< Width of the rectangle */
    uint16_t h; /*!< Height of the rectangle */
};

/*!
 * \brief Represents a pair of coordinates
 */
struct Coords {
    uint16_t x; /*!< X position in pixels */
    uint16_t y; /*!< Y position in pixels */
};

/*!
 * \brief Union to hold different types of label values
 */
union LabelData {
    char *text;      /*!< Text to display */
    int32_t int_val; /*!< Integer value to display */
    float float_val; /*!< Float value to display */
};

/*!
 * \brief Defines the coloring method to use
 */
enum LabelDataType {
    LABEL_DATA_STRING,  /*!< Text value type */
    LABEL_DATA_INT,     /*!< Integer value type */
    LABEL_DATA_FLOAT_1, /*!< Float value type with 1 decimal */
    LABEL_DATA_FLOAT_2, /*!< Float value type with 2 decimals */
    LABEL_DATA_FLOAT_3  /*!< Float value type with 3 decimals */
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
typedef void (*draw_rectangle_callback)(uint16_t x, uint16_t y, uint16_t w, uint16_t h, struct Color color);

/*!
 * \brief Function used to clear the screen
 * 
 * \details This callback function is used to clear the whole screen
 */
typedef void (*clear_screen_callback)(void);

/*!
 * \brief Defines a label to be drawn on screen
 */
struct Label {
    union LabelData data;    /*!< Content of the label */
    enum LabelDataType type; /*!< Type of the label content */
    struct Coords pos;       /*!< Position to draw the label */
    enum FontName font;      /*!< Font name, defined in font.h */
    uint16_t size;           /*!< Size of the text */
    enum FontAlign align;    /*!< Alignement of the text relative to coords */
    struct Color color;      /*!< Color of the text */
};

/*!
 * \brief Defines a text box to be drawn on screen
 */
struct Box {
#if PARTIAL_RASTER
    bool updated; /*!< Flag to indicate if the box needs to be redrawn */
#endif
    uint16_t id;         /*!< Unique identifier for the box */
    struct Rect rect;    /*!< Rectangle area of the box */
    struct Color color;  /*!< Default background color of the box (ARGB format) */
    struct Label *label; /*!< Pointer to a Label structure (can be NULL) */
};

#endif // RASTER_H
