/**
 * This file is part of Cocoa.
 *
 * Cocoa is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * Cocoa is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cocoa. If not, see <https://www.gnu.org/licenses/>.
 */

#include <xkbcommon/xkbcommon.h>

#include "Core/Errors.h"
#include "Core/Journal.h"
#include "Glamor/Wayland/WaylandInputContext.h"
GLAMOR_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Glamor.Wayland.WaylandInputContext)

WaylandInputContext::WaylandInputContext(WaylandDisplay *display, xkb_context *context)
    : display_(display)
    , xkb_context_(context)
{
    CHECK(display_ && xkb_context_);
}

WaylandInputContext::~WaylandInputContext()
{
    xkb_context_unref(xkb_context_);
}

Unique<WaylandInputContext> WaylandInputContext::Make(WaylandDisplay *display)
{
    CHECK(display);

    xkb_context *xkb_ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (!xkb_ctx)
    {
        QLOG(LOG_ERROR, "Failed to create a XKB context");
        return nullptr;
    }

    return std::make_unique<WaylandInputContext>(display, xkb_ctx);
}

GLAMOR_NAMESPACE_END
