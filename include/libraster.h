/*!
 * \file libraster.h
 * \date 2024-12-13
 * \author Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Graphics handling functions
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
 * \brief Function used to draw a rectangle on screen
 * 
 * \details This callback function is used to draw a filled rectangle
 *
 * \param[in] x X position of the rectangle
 * \param[in] y Y position of the rectangle
 * \param[in] w Width of the rectangle
 * \param[in] h Height of the rectangle
 * \param[in] color Color of the rectangle (ARGB format)
 */
typedef void (*draw_rectangle_callback_t)(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color);

/*!
 * \brief Function used to clear the screen
 * 
 * \details This callback function is used to clear the whole screen
 */
typedef void (*clear_screen_callback_t)(void);

/*!
 * \brief Defines a label to be drawn on screen
 */
struct Label {
    char *text;           /*!< Text to display */
    struct Coords pos;    /*!< Position to draw the label */
    enum FontName font;   /*!< Font name, defined in font.h */
    uint16_t font_size;   /*!< Size of the text */
    enum FontAlign align; /*!< Alignement of the text */
};

/*!
 * \brief Defines one threshold for coloring
 */
struct Threshold {
    float min;         /*!< Minimum value of the threshold */
    float max;         /*!< Maximum value of the threshold */
    uint32_t bg_color; /*!< Background color to apply if in range */
    uint32_t fg_color; /*!< Foreground color to apply if in range */
};

/*!
 * \brief Defines multiple thresholds for coloring
 */
struct Thresholds {
    struct Threshold *threshold; /*!< Pointer to an array of thresholds */
    uint8_t thresholds_num;      /*!< Number of thresholds present in the array */
};

/*!
 * \brief Defines parameters for linear color interpolation
 */
struct LinearInterpolation {
    uint32_t color_min; /*!< Color for the minimum value */
    uint32_t color_max; /*!< Color for the maximum value */
    float min;          /*!< Minimum value for interpolation */
    float max;          /*!< Maximum value for interpolation */
};

/*!
 * \brief Defines on which side to anchor the slider
 */
enum AnchorSlider {
    ANCHOR_TOP,    /*!< Slider is placed on the top side of the box */
    ANCHOR_BOTTOM, /*!< Slider is placed on the bottom side of the box */
    ANCHOR_LEFT,   /*!< Slider is placed on the left side of the box */
    ANCHOR_RIGHT,  /*!< Slider is placed on the right side of the box */
};

/*!
 * \brief Defines parameters for a slider representation
 */
struct Slider {
    uint32_t color;           /*!< Color of the slider (ARGB format) */
    enum AnchorSlider anchor; /*!< Side where to anchor the slider */
    float max;                /*!< Maximum value for the slider */
    float min;                /*!< Minimum value for the slider */
    uint16_t margin;          /*!< Margin from the box sides */
};

/*!
 * \brief Defines the coloring method type
 */
enum ColorType {
    THRESHOLDS,    /*!< Use thresholds coloring method */
    INTERPOLATION, /*!< Use linear interpolation coloring method */
    SLIDER,        /*!< Use slider coloring method */
};

/*!
 * \brief Defines possible coloring methods
 */
union Colors {
    struct Thresholds *thresholds;            /*!< Pointer to thresholds structure */
    struct LinearInterpolation interpolation; /*!< Linear interpolation structure */
    struct Slider slider;                     /*!< Slider structure */
};

/*!
 * \brief Defines a value to be drawn on screen
 */
struct Value {
    float value;               /*!< The value to display */
    bool is_float;             /*!< Defines if the value is float or integer */
    struct Coords pos;         /*!< Position to draw the value */
    enum FontName font;        /*!< Font name, defined in font.h */
    float font_size;           /*!< Size of the text */
    enum FontAlign align;      /*!< Alignement of the text */
    union Colors colors;       /*!< Coloring method to use */
    enum ColorType color_type; /*!< Which coloring method is used */
};

/*!
 * \brief Defines a text box to be drawn on screen
 */
struct Box {
#if PARTIAL_RASTER
    bool updated; /*!< Flag to indicate if the box needs to be redrawn */
#endif
    uint16_t id; /*!< Unique identifier for the box */
    ;
    struct Rect rect;          /*!< Rectangle area of the box */
    uint32_t default_bg_color; /*!< Default background color of the box (ARGB format) */
    uint32_t default_fg_color; /*!< Default foreground color of the box (ARGB format) */
    struct Label *label;       /*!< Pointer to a Label structure (can be NULL) */
    struct Value *value;       /*!< Pointer to a Value structure (can be NULL) */
};

#endif // RASTER_H
