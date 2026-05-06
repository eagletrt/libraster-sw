/*!
 * \file fontutils.h
 * \date 2024-12-30
 * \author Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Common types for libraster: return codes, callbacks, font/glyph descriptors.
 */

#ifndef FONTUTILS_H
#define FONTUTILS_H

#include "colors.h"
#include <stdint.h>

#ifdef __has_include
#if __has_include("raster-config.h")
#include "raster-config.h"
#endif
#endif

#ifndef RASTER_PARTIAL
/*!
 * \brief Enable partial raster rendering by default.
 *
 * \details When set to 1 (default), only boxes flagged as updated are redrawn,
 *     and the flag is cleared after rendering. When set to 0 the screen is
 *     cleared and every box is redrawn each frame; the clear callback must
 *     then be provided.
 */
#define RASTER_PARTIAL (1)
#endif

/*!
 * \brief Return codes for libraster operations.
 */
enum RasterReturnCode {
    RASTER_RC_OK,           /*!< Operation completed successfully */
    RASTER_RC_ERROR,        /*!< Operation failed */
    RASTER_RC_NULL_POINTER, /*!< A required pointer was NULL */
};

/*!
 * \brief Horizontal text alignment relative to the label position.
 */
enum FontAlign {
    FONT_ALIGN_LEFT,   /*!< Position is the left edge of the rendered text */
    FONT_ALIGN_CENTER, /*!< Position is the horizontal center of the rendered text */
    FONT_ALIGN_RIGHT,  /*!< Position is the right edge of the rendered text */
};

/*!
 * \brief Callback invoked to fill a rectangle with a single color.
 *
 * \details This is the only drawing primitive the user must implement. The
 *     library uses it both for box backgrounds and for glyph rasterization
 *     (where the alpha channel of \p color carries the coverage value).
 *     A horizontal line is just a rectangle with `h == 1`.
 *
 *     On STM32 this maps naturally to a DMA2D fill; on SDL it maps to
 *     SDL_FillRect; on a software framebuffer it is two nested loops.
 *
 * \param[in] x     Top-left X position of the rectangle.
 * \param[in] y     Top-left Y position of the rectangle.
 * \param[in] w     Width of the rectangle in pixels.
 * \param[in] h     Height of the rectangle in pixels.
 * \param[in] color Fill color in ARGB.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_ERROR on hardware/driver failure.
 */
typedef enum RasterReturnCode (*raster_draw_rectangle_callback)(uint16_t x, uint16_t y, uint16_t w, uint16_t h, struct Color color);

/*!
 * \brief Callback invoked to clear the entire framebuffer.
 *
 * \details Required only when RASTER_PARTIAL is 0; may be NULL otherwise.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_ERROR on hardware/driver failure.
 */
typedef enum RasterReturnCode (*raster_clear_screen_callback)(void);

/*!
 * \brief Metadata for a single glyph inside a Font.
 *
 * \details The glyph table is sorted by character so it can be searched
 *     with binary lookup.
 */
struct Glyph {
    char character;  /*!< ASCII character represented by this glyph */
    uint32_t offset; /*!< Byte offset of the glyph data inside Font::sdf_data */
    uint16_t size;   /*!< Length of the glyph data in bytes */
    uint16_t width;  /*!< Native pixel width of the glyph */
    uint16_t height; /*!< Native pixel height of the glyph */
};

/*!
 * \brief A rasterizable font.
 *
 * \details Fonts are produced by tools/generator.py and live in the user's
 *     project, not in libraster itself. Labels reference a font by pointer,
 *     so the library never embeds specific font tables and any number of
 *     fonts can coexist.
 */
struct Font {
    uint16_t base_size;         /*!< Native pixel height of the glyphs as generated */
    const uint8_t *sdf_data;    /*!< RLE-compressed SDF data for all glyphs */
    const struct Glyph *glyphs; /*!< Glyph table, sorted by character */
    uint16_t glyph_count;       /*!< Number of entries in glyphs[] */
};

/*!
 * \brief Look up the glyph for a character via binary search.
 *
 * \param[in] font Font to search in.
 * \param[in] c    Character to look up.
 *
 * \return Pointer to the matching Glyph, or NULL if not found or \p font is NULL.
 */
const struct Glyph *font_find_glyph(const struct Font *font, char c);

#endif // FONTUTILS_H
