/**
 * @file libraster.h
 * @date 2024-12-13
 * @author Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * @brief Graphics handling functions
 */

#ifndef RASTER_H
#define RASTER_H

#include <stdbool.h>
#include <stdint.h>
#include "fontutils.h"

/**
 * @brief Rectangle definition for the UI
 *
 * @details 
 *     - x Is the position in pixel on the x axis
 *     - y Is the position in pixel on the y axis
 *     - w Is the width in pixel
 *     - h Is the height in pixel
 */
struct Rect {
    uint16_t x;
    uint16_t y;
    uint16_t w;
    uint16_t h;
};

/**
 * @brief Represents a set of coordinates
 *
 * @details
 *     - x Is the position in pixel on the x axis
 *     - y Is the position in pixel on the y axis
 */
struct Coords {
    uint16_t x;
    uint16_t y;
};

/**
  * @brief Callback to draw a rectangle (may use DMA2D)
  *
  * @param x Position in pixel on x axis
  * @param y Position in pixel on y axis
  * @param w Width in pixel of rectangle
  * @param h Height in pixel of rectangle
  * @param color Color in ARGB8888 format
  */
typedef void (*draw_rectangle_callback_t)(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color);

/**
  * @brief Function used to clear screen (user decides the color)
  */
typedef void (*clear_screen_callback_t)(void);

/**
 * @brief Defines one component of the UI
 *
 * @details
 *     - text Is a pointer to the string to plot
 *     - pos Is the position where to plot the text
 *     - font Font name (defined in font.h)
 *     - font_size Defines the size of the text
 *     - align Defines the alignement of the text
 */
struct Label {
    char *text;
    struct Coords pos;
    enum FontName font;
    uint16_t font_size;
    enum FontAlign align;
};

/**
 * @brief Defines a range for whom the color may be applied
 *
 * @details
 *     - min Is the min value for whom the color is applied
 *     - max Is the max value for whom the color is applied
 *     - bg_color Is the background color to apply
 *     - fg_color Is the foreground color to apply
 */
struct Threshold {
    float min;
    float max;
    uint32_t bg_color;
    uint32_t fg_color;
};

/**
 * @brief Includes all the thresholds in one struct
 *
 * @details
 *     - threshold Pointer to Threshold struct
 *     - thresholds_num Number of thresholds in the array
 */
struct Thresholds {
    struct Threshold *threshold;
    uint8_t thresholds_num;
};

/**
 * @brief Defines an interpolation between 2 colors based on 2 values
 *
 * @details
 *     - color_min The color applied if the value is lower or equal to min
 *     - color_max The color applied if the value is higher ot equal to max
 *     - min The minimum value
 *     - max The maximum value
 */
struct LinearInterpolation {
    uint32_t color_min;
    uint32_t color_max;
    float min;
    float max;
};

/**
 * @brief Defines how the slider should behave (where to attach in the box)
 *
 * @details
 *     - ANCHOR_<self descriptive> Attaches in the direction described in the name
 */
enum AnchorSlider {
    ANCHOR_TOP,
    ANCHOR_BOTTOM,
    ANCHOR_LEFT,
    ANCHOR_RIGHT,
};

/**
 * @brief Describes all the useful parameters of the slider
 *
 * @details
 *     - color The color of the slider (background used is the default one)
 *     - anchor Defines on which side of the box to attach the slider
 *     - max Max value of the slider
 *     - min Min value of the slider
 *     - margin Space to leave in between the box borders and the slider
 */
struct Slider {
    uint32_t color;
    enum AnchorSlider anchor;
    float max;
    float min;
    uint16_t margin;
};

/**
 * @brief Defines which type of box is this
 */
enum ColorType {
    THRESHOLDS,
    INTERPOLATION,
    SLIDER,
};

/**
 * @brief Contains the struct that defines one coloring method. ColorType should match the one placed in here.
 */
union Colors {
    struct Thresholds *thresholds;
    struct LinearInterpolation interpolation;
    struct Slider slider;
};

/**
 * @brief Defines one component of the UI
 *
 * @details
 *     - value Is the value to display
 *     - is_float Tells if the value is to be casted
 *     - pos Is the position there to plot the value
 *     - font Font name, defined in font.h
 *     - font_size Defines the size of the text
 *     - align Defines the alignement of the text
 *     - colors Contains one possible coloring method
 *     - color_type Describes which coloring method to use with the box based on the value
 */
struct Value {
    float value;
    bool is_float;
    struct Coords pos;
    enum FontName font;
    float font_size;
    enum FontAlign align;
    union Colors colors;
    enum ColorType color_type;
};

/**
 * @brief Defines a composed component of the UI
 *
 * @details
 *     - updated Is used to optimize the interface drawing (usable only is GRAPHICS_OPT == 1)
 *     - id Is used to identify the component
 *     - rect Defines the boundings of the Box
 *     - default_bg_color Is used if no ranges are present or in range
 *     - default_fg_color Is used if no ranges are present or in range
 *     - label Is a pointer to Label component
 *     - value Is a pointer to Value component
 */
struct Box {
#if PARTIAL_RASTER
    bool updated;
#endif
    uint16_t id;
    struct Rect rect;
    uint32_t default_bg_color;
    uint32_t default_fg_color;
    struct Label *label;
    struct Value *value;
};

#endif // RASTER_H
