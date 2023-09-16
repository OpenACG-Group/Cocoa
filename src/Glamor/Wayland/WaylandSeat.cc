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

#include <sstream>

#include <wayland-client.h>

#include "Core/Journal.h"
#include "Core/Exception.h"
#include "Glamor/Wayland/WaylandSeat.h"
#include "Glamor/Wayland/WaylandDisplay.h"
#include "Glamor/Wayland/WaylandSurface.h"
#include "Glamor/Wayland/WaylandSeatPointerDevice.h"
#include "Glamor/Wayland/WaylandSeatKeyboardDevice.h"
GLAMOR_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Glamor.Wayland.Seat)

#define SEAT(ptr)   (reinterpret_cast<WaylandSeat*>(ptr))

namespace {

const wl_seat_listener g_seat_listener = {
    .capabilities = WaylandSeat::on_capabilities,
    .name = WaylandSeat::on_name
};

} // namespace anonymous

void WaylandSeat::on_capabilities(void *data, wl_seat *seat, uint32_t caps)
{
    auto *seat_object = SEAT(data);
    CHECK(seat_object);

    uint32_t seat_id = seat_object->registry_id_;

    bool has_caps = false;
    std::ostringstream named_caps;

    if (caps & WL_SEAT_CAPABILITY_KEYBOARD)
    {
        named_caps << "keyboard";
        has_caps = true;
        seat_object->keyboard_device_ = WaylandSeatKeyboardDevice::MakeFromKeyboardDevice(
                seat_object, wl_seat_get_keyboard(seat));
    }
    else if (seat_object->keyboard_device_)
    {
        seat_object->keyboard_device_.reset();
    }

    if (caps & WL_SEAT_CAPABILITY_POINTER)
    {
        named_caps << ((const char*)",pointer" + !has_caps);
        has_caps = true;
        seat_object->pointer_device_ = WaylandSeatPointerDevice::MakeFromPointerDevice(
                seat_object, wl_seat_get_pointer(seat));
    }
    else if (seat_object->pointer_device_)
    {
        seat_object->pointer_device_.reset();
    }

    if (caps & WL_SEAT_CAPABILITY_TOUCH)
    {
        named_caps << ((const char*)",touch" + !has_caps);
        seat_object->touch_device_ = wl_seat_get_touch(seat);
        // TODO(sora): handle touch devices.
    }
    else if (seat_object->touch_device_)
    {
        wl_touch_destroy(seat_object->touch_device_);
        seat_object->touch_device_ = nullptr;
    }

    QLOG(LOG_INFO, "Wayland seat {} capabilities updates: has {} devices",
         seat_id, named_caps.str());
}

void WaylandSeat::on_name(void *data, g_maybe_unused wl_seat *seat, const char *name)
{
    auto *seat_object = SEAT(data);

    uint32_t seat_id = seat_object->registry_id_;

    QLOG(LOG_INFO, "Wayland seat {} updates name \"{}\"", seat_id, name);
    seat_object->seat_name_ = name;
}

std::shared_ptr<WaylandSeat>
WaylandSeat::Make(const std::shared_ptr<WaylandDisplay>& display,
                  wl_seat *seat, uint32_t registry_id)
{
    CHECK(display && seat);

    auto seat_object = std::make_shared<WaylandSeat>(display, seat, registry_id);

    // Add listeners to wayland seat here. Callbacks will be fired
    // during the next roundtrip started by `WaylandDisplay::Connect`.
    // That is, listener callbacks will not be firstly fired in `Connect` function
    // instead of being fired in the event loop.
    wl_seat_add_listener(seat, &g_seat_listener, seat_object.get());

    return seat_object;
}

WaylandSeat::WaylandSeat(std::weak_ptr<WaylandDisplay> display,
                         wl_seat *seat,
                         uint32_t registry_id)
    : display_(std::move(display))
    , wl_seat_(seat)
    , registry_id_(registry_id)
    , keyboard_device_(nullptr)
    , pointer_device_(nullptr)
    , touch_device_(nullptr)
{
}

WaylandSeat::~WaylandSeat()
{
    pointer_device_.reset();

    CHECK(wl_seat_ && "Invalid seat pointer");
    wl_seat_destroy(wl_seat_);
}

std::shared_ptr<WaylandSurface>
WaylandSeat::FindSurfaceByNativeHandle(wl_surface *surface)
{
    // The userdata field of `surface` points to `WaylandRenderTarget` object with an uncertain type.
    // It may be `WaylandHWComposeRenderTarget` or `WaylandSHMRenderTarget`,
    // so getting the associated `Surface` object through the userdata field of `surface`
    // is completely unreliable. We just try to iterate and match here.
    for (const auto& window : display_.lock()->GetSurfacesList())
    {
        wl_surface *search_surface = window->Cast<WaylandSurface>()->GetWaylandSurface();
        if (search_surface != surface)
            continue;
        return window->As<WaylandSurface>();
    }

    return nullptr;
}

GLAMOR_NAMESPACE_END
