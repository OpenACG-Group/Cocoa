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

#ifndef COCOA_GLAMOR_WAYLAND_WAYLANDSEAT_H
#define COCOA_GLAMOR_WAYLAND_WAYLANDSEAT_H

#include <wayland-client.h>

#include "Glamor/Glamor.h"
GLAMOR_NAMESPACE_BEGIN

class WaylandDisplay;
class WaylandSeatListener;
class WaylandSurface;

class WaylandSeat
{
    friend class WaylandSeatListener;

public:
    WaylandSeat(std::weak_ptr<WaylandDisplay> display,
                wl_seat *seat,
                uint32_t registry_id);
    ~WaylandSeat();

    static Shared<WaylandSeat> Make(const std::shared_ptr<WaylandDisplay>& display,
                                    wl_seat *seat,
                                    uint32_t registry_id);

    g_nodiscard g_inline uint32_t GetRegistryId() const {
        return registry_id_;
    }

    g_nodiscard g_inline const auto& GetListener() const {
        return listener_;
    }

    g_nodiscard g_inline const std::string& GetName() const {
        return seat_name_;
    }

    Shared<WaylandSurface> FindSurfaceByNativeHandle(wl_surface *surface);

    g_nodiscard g_inline wl_keyboard *GetKeyboardDevice() const {
        return keyboard_device_;
    }

    g_nodiscard g_inline wl_pointer *GetPointerDevice() const {
        return pointer_device_;
    }

    g_nodiscard g_inline wl_touch *GetTouchDevice() const {
        return touch_device_;
    }

private:
    std::unique_ptr<WaylandSeatListener> listener_;
    std::weak_ptr<WaylandDisplay>   display_;
    wl_seat                        *wl_seat_;
    uint32_t                        registry_id_;

    wl_keyboard                    *keyboard_device_;
    wl_pointer                     *pointer_device_;
    wl_touch                       *touch_device_;
    std::string                     seat_name_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_WAYLAND_WAYLANDSEAT_H
