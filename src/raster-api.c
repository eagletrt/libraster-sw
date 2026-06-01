/*!
 * \file raster-api.c
 * \date 2024-12-13
 * \author Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief LibRaster public API implementation.
 *
 * \details The render mode (partial vs. full) is decided at runtime by the
 *     presence of a clear callback on the handler. No build-time macros
 *     are involved, so the same compiled library object can serve both
 *     modes in a project that switches between them.
 */

#include "raster-api.h"
#include "box-api.h"
#include <stdbool.h>
#include <stddef.h>

enum RasterReturnCode raster_api_init(struct RasterHandler *handler, struct Box *interface, uint16_t box_count, raster_draw_rectangle_callback draw, raster_clear_screen_callback clear) {
    if (handler == NULL || interface == NULL || box_count == 0u || draw == NULL) {
        return RASTER_RC_NULL_POINTER;
    }
    handler->interface = interface;
    handler->box_count = box_count;
    handler->draw = draw;
    handler->clear = clear;
    return RASTER_RC_OK;
}

enum RasterReturnCode raster_api_set_interface(struct RasterHandler *handler, struct Box *interface, uint16_t box_count) {
    if (handler == NULL || interface == NULL || box_count == 0U) {
        return RASTER_RC_NULL_POINTER;
    }
    handler->interface = interface;
    handler->box_count = box_count;
    return RASTER_RC_OK;
}

enum RasterReturnCode raster_api_render(struct RasterHandler *handler) {
    if (handler == NULL || handler->draw == NULL || handler->interface == NULL) {
        return RASTER_RC_NULL_POINTER;
    }

    const bool full_redraw = (handler->clear != NULL);
    if (full_redraw) {
        if (handler->clear() != RASTER_RC_OK) {
            return RASTER_RC_ERROR;
        }
    }

    for (uint16_t i = 0; i < handler->box_count; ++i) {
        struct Box *box = &handler->interface[i];
        if (!full_redraw && !box->updated) {
            continue;
        }
        if (box_api_draw(box, handler->draw) != RASTER_RC_OK) {
            return RASTER_RC_ERROR;
        }
        if (!full_redraw) {
            box->updated = false;
        }
    }

    return RASTER_RC_OK;
}
