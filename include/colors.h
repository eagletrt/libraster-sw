#ifndef COLORS_H
#define COLORS_H

#include <stdint.h>

/*!
 * \brief Represents a color in ARGB format
 *
 * \details This structure represents a color using a union that allows
 *      access to the color as a single 32-bit integer in ARGB format or
 *      as individual 8-bit components for alpha, red, green, and blue.
 */
struct Color {
    union {
        uint32_t argb; /*!< Color in ARGB format */
        struct {
            uint8_t b; /*!< Blue component */
            uint8_t g; /*!< Green component */
            uint8_t r; /*!< Red component */
            uint8_t a; /*!< Alpha component */
        };
    };
};

#endif // COLORS_H
