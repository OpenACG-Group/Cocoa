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

#include <linux/input-event-codes.h>

#include <sstream>

#include <wayland-client.h>
#include <wayland-cursor.h>

#include "Core/Journal.h"
#include "Glamor/Wayland/WaylandSeat.h"
#include "Glamor/Wayland/WaylandDisplay.h"
#include "Glamor/Wayland/WaylandSurface.h"
#include "Glamor/Wayland/WaylandCursorTheme.h"
#include "Glamor/Wayland/WaylandSystemCursor.h"
GLAMOR_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Glamor.Wayland.Seat)

#define LISTENER(ptr)   (reinterpret_cast<WaylandSeatListener*>(ptr))

class WaylandSeatListener
{
public:
    explicit WaylandSeatListener(WaylandSeat *seat) : seat_(seat) {}
    ~WaylandSeatListener() = default;

    // Seat events
    static void on_capabilities(void *data, wl_seat *seat, uint32_t caps);
    static void on_name(void *data, wl_seat *seat, const char *name);

    // Pointer device events
    static void on_enter(void *data,
                         wl_pointer *pointer,
                         uint32_t serial,
                         wl_surface *surface,
                         wl_fixed_t surface_x,
                         wl_fixed_t surface_y);
    static void on_leave(void *data,
                         wl_pointer *pointer,
                         uint32_t serial,
                         wl_surface *surface);
    static void on_motion(void *data,
                          wl_pointer *pointer,
                          uint32_t time,
                          wl_fixed_t surface_x,
                          wl_fixed_t surface_y);
    static void on_button(void *data,
                          wl_pointer *pointer,
                          uint32_t serial,
                          uint32_t time,
                          uint32_t button,
                          uint32_t state);
    static void on_axis(void *data,
                        wl_pointer *pointer,
                        uint32_t time,
                        uint32_t axis,
                        wl_fixed_t value) {}
    static void on_frame(void *data, wl_pointer *pointer) {}
    static void on_axis_source(void *data,
                               wl_pointer *pointer,
                               uint32_t axis_source) {}
    static void on_axis_stop(void *data,
                             wl_pointer *pointer,
                             uint32_t time,
                             uint32_t axis) {}
    static void on_axis_discrete(void *data,
                                 wl_pointer *pointer,
                                 uint32_t axis,
                                 int32_t discrete);
    static void on_axis_value120(void *data,
                                 wl_pointer *pointer,
                                 uint32_t axis,
                                 int32_t value120) {}

    WaylandSeat     *seat_;
};

namespace {

const wl_seat_listener g_seat_listener = {
    .capabilities = WaylandSeatListener::on_capabilities,
    .name = WaylandSeatListener::on_name
};

const wl_pointer_listener g_pointer_listener = {
    .enter = WaylandSeatListener::on_enter,
    .leave = WaylandSeatListener::on_leave,
    .motion = WaylandSeatListener::on_motion,
    .button = WaylandSeatListener::on_button,
    .axis = WaylandSeatListener::on_axis,
    .frame = WaylandSeatListener::on_frame,
    .axis_source = WaylandSeatListener::on_axis_source,
    .axis_stop = WaylandSeatListener::on_axis_stop,
    .axis_discrete = WaylandSeatListener::on_axis_discrete,
    .axis_value120 = WaylandSeatListener::on_axis_value120
};

Shared<WaylandSurface> extract_surface_from_pointer(void *data, wl_pointer *pointer)
{
    CHECK(data);
    auto *listener = LISTENER(data);

    auto display = listener->seat_->GetDisplay();
    return display->GetPointerEnteredSurface(pointer);
}

} // namespace anonymous

void WaylandSeatListener::on_capabilities(void *data, wl_seat *seat, uint32_t caps)
{
    auto *listener = LISTENER(data);
    uint32_t seat_id = listener->seat_->registry_id_;

    bool has_caps;
    std::ostringstream named_caps;

    if (caps & WL_SEAT_CAPABILITY_KEYBOARD)
    {
        named_caps << "keyboard";
        has_caps = true;
        listener->seat_->keyboard_device_ = wl_seat_get_keyboard(seat);
        // TODO(sora): handle keyboard devices.
    }
    else if (listener->seat_->keyboard_device_)
    {
        wl_keyboard_destroy(listener->seat_->keyboard_device_);
        listener->seat_->keyboard_device_ = nullptr;
    }

    if (caps & WL_SEAT_CAPABILITY_POINTER)
    {
        named_caps << ((const char*)",pointer" + !has_caps);
        has_caps = true;
        listener->seat_->pointer_device_ = wl_seat_get_pointer(seat);
        wl_pointer_add_listener(listener->seat_->pointer_device_,
                                &g_pointer_listener, data);
    }
    else if (listener->seat_->pointer_device_)
    {
        wl_pointer_destroy(listener->seat_->pointer_device_);
        listener->seat_->pointer_device_ = nullptr;
    }

    if (caps & WL_SEAT_CAPABILITY_TOUCH)
    {
        named_caps << ((const char*)",touch" + !has_caps);
        listener->seat_->touch_device_ = wl_seat_get_touch(seat);
        // TODO(sora): handle touch devices.
    }
    else if (listener->seat_->touch_device_)
    {
        wl_touch_destroy(listener->seat_->touch_device_);
        listener->seat_->touch_device_ = nullptr;
    }

    QLOG(LOG_INFO, "Wayland seat {} capabilities updates: has {} devices",
         seat_id, named_caps.str());
}

void WaylandSeatListener::on_name(void *data, g_maybe_unused wl_seat *seat, const char *name)
{
    auto *listener = LISTENER(data);
    uint32_t seat_id = listener->seat_->registry_id_;

    QLOG(LOG_INFO, "Wayland seat {} updates name \"{}\"", seat_id, name);
    listener->seat_->seat_name_ = name;
}

void WaylandSeatListener::on_enter(void *data, wl_pointer *pointer, uint32_t serial,
                                   wl_surface *surface,
                                   g_maybe_unused wl_fixed_t surface_x,
                                   g_maybe_unused wl_fixed_t surface_y)
{
    auto *listener = LISTENER(data);
    auto display = listener->seat_->display_.lock();
    CHECK(display);

    auto surface_object = listener->seat_->FindSurfaceByNativeHandle(surface);
    if (!surface_object)
    {
        QLOG(LOG_ERROR, "Compositor notified us the pointer entered a surface"
                        " which is not in the surfaces list");
        return;
    }

    // Update the serial number first
    surface_object->SetLatestPointerEnterEventSerial(serial);
    surface_object->SetEnteredPointerDevice(pointer);

    // Set an appropriate cursor which is associated with the surface
    auto cursor_base = surface_object->GetAttachedCursor();
    if (cursor_base)
    {
        auto cursor = cursor_base->As<WaylandCursor>();
        wl_pointer_set_cursor(pointer,
                              serial,
                              cursor->GetCursorSurface(),
                              cursor->GetHotspotVector().x(),
                              cursor->GetHotspotVector().y());

        cursor->TryStartAnimation();
    }
    else
    {
        QLOG(LOG_WARNING, "No cursor was associated with the surface");
    }

    // At last, notify the corresponding surface the event.
    // This invocation emits a `GLSI_SURFACE_HOVERED` signal to user.
    RenderClientEmitterInfo info;
    info.EmplaceBack<bool>(true);
    surface_object->Emit(GLSI_SURFACE_POINTER_HOVERING, std::move(info));
}

void WaylandSeatListener::on_leave(void *data,
                                   g_maybe_unused wl_pointer *pointer,
                                   g_maybe_unused uint32_t serial,
                                   wl_surface *surface)
{
    auto *listener = LISTENER(data);
    auto display = listener->seat_->display_.lock();
    CHECK(display);

    auto surface_object = listener->seat_->FindSurfaceByNativeHandle(surface);
    if (!surface_object)
    {
        QLOG(LOG_ERROR, "Compositor notified us the pointer left a surface"
                        " which is not in the surfaces list");
        return;
    }

    surface_object->SetEnteredPointerDevice(nullptr);

    // Stop the cursor's animation
    auto cursor_base = surface_object->GetAttachedCursor();
    if (cursor_base)
    {
        cursor_base->As<WaylandCursor>()->TryAbortAnimation();
    }
    else
    {
        QLOG(LOG_WARNING, "No cursor was associated with the surface");
    }

    RenderClientEmitterInfo info;
    info.EmplaceBack<bool>(false);
    surface_object->Emit(GLSI_SURFACE_POINTER_HOVERING, std::move(info));
}

void WaylandSeatListener::on_motion(void *data,
                                    wl_pointer *pointer,
                                    g_maybe_unused uint32_t time,
                                    wl_fixed_t surface_x,
                                    wl_fixed_t surface_y)
{
    CHECK(data && pointer);

    Shared<WaylandSurface> surface = extract_surface_from_pointer(data, pointer);
    if (!surface)
    {
        QLOG(LOG_ERROR, "Compositor notified us a motion event of a pointer "
                        "which is not hovering on any surfaces");
        return;
    }

    RenderClientEmitterInfo info;
    info.EmplaceBack<double>(wl_fixed_to_double(surface_x));
    info.EmplaceBack<double>(wl_fixed_to_double(surface_y));
    surface->Emit(GLSI_SURFACE_POINTER_MOTION, std::move(info));
}

void WaylandSeatListener::on_button(void *data,
                                    wl_pointer *pointer,
                                    g_maybe_unused uint32_t serial,
                                    g_maybe_unused uint32_t time,
                                    uint32_t button,
                                    uint32_t state)
{
    static std::map<uint32_t, PointerButton> button_map = {
        { BTN_LEFT,    PointerButton::kLeft    },
        { BTN_RIGHT,   PointerButton::kRight   },
        { BTN_MIDDLE,  PointerButton::kMiddle  },
        { BTN_SIDE,    PointerButton::kSide    },
        { BTN_FORWARD, PointerButton::kForward },
        { BTN_BACK,    PointerButton::kBack    },
        { BTN_EXTRA,   PointerButton::kExtra   },
        { BTN_TASK,    PointerButton::kTask    }
    };

    CHECK(data && pointer);

    if (button_map.count(button) == 0)
    {
        QLOG(LOG_WARNING, "Unrecognized button of pointer device: 0x{:x}", button);
        return;
    }

    Shared<WaylandSurface> surface = extract_surface_from_pointer(data, pointer);
    if (!surface)
    {
        QLOG(LOG_ERROR, "Compositor notified us a button event of a pointer "
                        "which is not hovering on any surfaces");
        return;
    }

    bool pressed = WL_POINTER_BUTTON_STATE_PRESSED == state;

    RenderClientEmitterInfo info;
    info.EmplaceBack<PointerButton>(button_map[button]);
    info.EmplaceBack<bool>(pressed);
    surface->Emit(GLSI_SURFACE_POINTER_BUTTON, std::move(info));
}

void WaylandSeatListener::on_axis_discrete(g_maybe_unused void *data,
                                           g_maybe_unused wl_pointer *pointer,
                                           g_maybe_unused uint32_t axis,
                                           g_maybe_unused int32_t discrete)
{
    // Deprecated event (since version 8).
}

Shared<WaylandSeat> WaylandSeat::Make(const std::shared_ptr<WaylandDisplay>& display,
                                      wl_seat *seat,
                                      uint32_t registry_id)
{
    CHECK(display && seat);

    auto seat_object = std::make_shared<WaylandSeat>(display, seat, registry_id);
    CHECK(seat_object);

    // Add listeners to wayland seat here. Callbacks will be fired
    // during the next roundtrip started by `WaylandDisplay::Connect`.
    // That is, listener callbacks will not be firstly fired in `Connect` function
    // instead of being fired in the event loop.
    wl_seat_add_listener(seat, &g_seat_listener, seat_object->GetListener().get());

    return seat_object;
}

WaylandSeat::WaylandSeat(std::weak_ptr<WaylandDisplay> display,
                         wl_seat *seat,
                         uint32_t registry_id)
    : listener_(std::make_unique<WaylandSeatListener>(this))
    , display_(std::move(display))
    , wl_seat_(seat)
    , registry_id_(registry_id)
    , keyboard_device_(nullptr)
    , pointer_device_(nullptr)
    , touch_device_(nullptr)
{
}

WaylandSeat::~WaylandSeat()
{
    CHECK(wl_seat_ && "Invalid seat pointer");
    wl_seat_destroy(wl_seat_);
}

Shared<WaylandSurface> WaylandSeat::FindSurfaceByNativeHandle(wl_surface *surface)
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
