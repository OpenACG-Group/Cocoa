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

#ifndef COCOA_GLAMOR_WAYLAND_WAYLANDINPUTCONTEXT_H
#define COCOA_GLAMOR_WAYLAND_WAYLANDINPUTCONTEXT_H

#include <xkbcommon/xkbcommon.h>

#include "Glamor/Glamor.h"
GLAMOR_NAMESPACE_BEGIN

class WaylandDisplay;

class WaylandInputContext
{
public:
    WaylandInputContext(WaylandDisplay *display, xkb_context *context);
    ~WaylandInputContext();

    static std::unique_ptr<WaylandInputContext> Make(WaylandDisplay *display);

    g_nodiscard g_inline xkb_context *GetXkbContext() const {
        return xkb_context_;
    }

private:
    WaylandDisplay      *display_;
    xkb_context         *xkb_context_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_WAYLAND_WAYLANDINPUTCONTEXT_H
