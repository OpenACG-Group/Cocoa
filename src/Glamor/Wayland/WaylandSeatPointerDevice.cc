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
#include <wayland-client.h>

#include "Core/Errors.h"
#include "Core/Journal.h"
#include "Core/Exception.h"
#include "Glamor/Wayland/WaylandCursor.h"
#include "Glamor/Wayland/WaylandDisplay.h"
#include "Glamor/Wayland/WaylandSeat.h"
#include "Glamor/Wayland/WaylandSeatPointerDevice.h"
#include "Glamor/Wayland/WaylandSurface.h"
GLAMOR_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Glamor.Wayland.WaylandSeatPointerDevice)

namespace {

#define LISTENER(ptr)   (reinterpret_cast<WaylandSeatPointerDevice*>(ptr))

const wl_pointer_listener g_pointer_listener = {
    .enter = WaylandSeatPointerDevice::on_enter,
    .leave = WaylandSeatPointerDevice::on_leave,
    .motion = WaylandSeatPointerDevice::on_motion,
    .button = WaylandSeatPointerDevice::on_button,
    .axis = WaylandSeatPointerDevice::on_axis,
    .frame = WaylandSeatPointerDevice::on_frame,
    .axis_source = WaylandSeatPointerDevice::on_axis_source,
    .axis_stop = WaylandSeatPointerDevice::on_axis_stop,
    .axis_discrete = WaylandSeatPointerDevice::on_axis_discrete,
    .axis_value120 = WaylandSeatPointerDevice::on_axis_value120
};

Shared<WaylandSurface> extract_surface_from_pointer(void *data, wl_pointer *pointer)
{
    CHECK(data);
    auto *listener = LISTENER(data);

    auto display = listener->GetSeat()->GetDisplay();
    return display->GetPointerEnteredSurface(pointer);
}

} // namespace anonymous

WaylandSeatPointerDevice::WaylandSeatPointerDevice(WaylandSeat *seat,
                                                   wl_pointer *pointer)
    : seat_(seat)
    , pointer_device_(pointer)
    , axis_values_{0, 0}
    , axis_highres_scroll_{0, 0}
    , axis_source_type_(AxisSourceType::kUnknown)
    , axis_scroll_type_(AxisScrollType::kNo_ScrollType)
{
    CHECK(seat_);
    CHECK(pointer_device_);
}

WaylandSeatPointerDevice::~WaylandSeatPointerDevice()
{
    CHECK(pointer_device_);
    wl_pointer_destroy(pointer_device_);
}

Unique<WaylandSeatPointerDevice>
WaylandSeatPointerDevice::MakeFromPointerDevice(WaylandSeat *seat, wl_pointer *pointer)
{
    CHECK(seat && pointer);
    
    auto device = std::make_unique<WaylandSeatPointerDevice>(seat, pointer);
    wl_pointer_add_listener(pointer, &g_pointer_listener, device.get());
    
    return device;
}

void WaylandSeatPointerDevice::ResetEventGroupStates()
{
    axis_source_type_ = AxisSourceType::kUnknown;
    axis_values_[kX] = 0;
    axis_values_[kY] = 0;
    axis_highres_scroll_[kX] = 0;
    axis_highres_scroll_[kY] = 0;
    axis_scroll_type_ = kNo_ScrollType;
}

void WaylandSeatPointerDevice::on_enter(void *data,
                                        wl_pointer *pointer,
                                        uint32_t serial,
                                        wl_surface *surface,
                                        g_maybe_unused wl_fixed_t surface_x,
                                        g_maybe_unused wl_fixed_t surface_y)
{
    auto *listener = LISTENER(data);
    CHECK(listener);

    auto surface_object = listener->seat_->FindSurfaceByNativeHandle(surface);
    if (!surface_object)
    {
        QLOG(LOG_ERROR, "Compositor notified us the pointer entered a surface"
                        " which is not in the surfaces list");
        return;
    }

    // Update the serial number first
    surface_object->SetPointerEntered(serial, pointer);

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
    PresentSignal info;
    info.EmplaceBack<bool>(true);
    surface_object->Emit(GLSI_SURFACE_POINTER_HOVERING, std::move(info));
}

void WaylandSeatPointerDevice::on_leave(void *data,
                                        g_maybe_unused wl_pointer *pointer,
                                        g_maybe_unused uint32_t serial,
                                        wl_surface *surface)
{
    auto *listener = LISTENER(data);
    CHECK(listener);

    auto surface_object = listener->seat_->FindSurfaceByNativeHandle(surface);
    if (!surface_object)
    {
        QLOG(LOG_ERROR, "Compositor notified us the pointer left a surface"
                        " which is not in the surfaces list");
        return;
    }

    surface_object->SetPointerEntered(0, nullptr);

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

    PresentSignal info;
    info.EmplaceBack<bool>(false);
    surface_object->Emit(GLSI_SURFACE_POINTER_HOVERING, std::move(info));
}

void WaylandSeatPointerDevice::on_motion(void *data,
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

    PresentSignal info;
    info.EmplaceBack<double>(wl_fixed_to_double(surface_x));
    info.EmplaceBack<double>(wl_fixed_to_double(surface_y));
    surface->Emit(GLSI_SURFACE_POINTER_MOTION, std::move(info));
}

void WaylandSeatPointerDevice::on_button(void *data,
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

    PresentSignal info;
    info.EmplaceBack<PointerButton>(button_map[button]);
    info.EmplaceBack<bool>(pressed);
    surface->Emit(GLSI_SURFACE_POINTER_BUTTON, std::move(info));
}

void WaylandSeatPointerDevice::on_axis_discrete(g_maybe_unused void *data,
                                                g_maybe_unused wl_pointer *pointer,
                                                g_maybe_unused uint32_t axis,
                                                g_maybe_unused int32_t discrete)
{
    // Deprecated event (since version 8).
}

void WaylandSeatPointerDevice::on_axis(void *data,
                                       g_maybe_unused wl_pointer *pointer,
                                       g_maybe_unused uint32_t time,
                                       uint32_t axis,
                                       wl_fixed_t value)
{
    auto *listener = LISTENER(data);
    CHECK(listener);

    listener->axis_scroll_type_ = kNormal_ScrollType;

    auto dv = wl_fixed_to_double(value);
    if (axis == WL_POINTER_AXIS_VERTICAL_SCROLL)
        listener->axis_values_[kY] += dv;
    else if (axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL)
        listener->axis_values_[kX] += dv;
    else
        MARK_UNREACHABLE("Unexpected axis enumeration");
}

void WaylandSeatPointerDevice::on_axis_source(void *data,
                                              g_maybe_unused wl_pointer *pointer,
                                              uint32_t axis_source)
{
    auto *listener = LISTENER(data);
    CHECK(listener);

    switch (axis_source)
    {
    case WL_POINTER_AXIS_SOURCE_WHEEL:
        listener->axis_source_type_ = AxisSourceType::kWheel;
        break;

    case WL_POINTER_AXIS_SOURCE_WHEEL_TILT:
        listener->axis_source_type_ = AxisSourceType::kWheelTilt;
        break;

    case WL_POINTER_AXIS_SOURCE_FINGER:
        listener->axis_source_type_ = AxisSourceType::kFinger;
        break;

    case WL_POINTER_AXIS_SOURCE_CONTINUOUS:
        listener->axis_source_type_ = AxisSourceType::kContinuous;
        break;

    default:
        MARK_UNREACHABLE("Invalid axis source type enumeration");
    }
}

void WaylandSeatPointerDevice::on_axis_stop(g_maybe_unused void *data,
                                            g_maybe_unused wl_pointer *pointer,
                                            g_maybe_unused uint32_t time,
                                            g_maybe_unused uint32_t axis)
{
    // TODO(sora): Handle this event.
}

void WaylandSeatPointerDevice::on_axis_value120(void *data,
                                                g_maybe_unused wl_pointer *pointer,
                                                uint32_t axis,
                                                int32_t value120)
{
    auto *listener = LISTENER(data);
    CHECK(listener);

    listener->axis_scroll_type_ = kHighres_ScrollType;

    switch (axis)
    {
    case WL_POINTER_AXIS_HORIZONTAL_SCROLL:
        listener->axis_highres_scroll_[kX] += value120;
        break;
    case WL_POINTER_AXIS_VERTICAL_SCROLL:
        listener->axis_highres_scroll_[kY] += value120;
        break;
    default:
        MARK_UNREACHABLE("Unexpected enumeration value");
    }
}

void WaylandSeatPointerDevice::on_frame(void *data, wl_pointer *pointer)
{
    auto *listener = LISTENER(data);
    CHECK(listener);

    ScopeExitAutoInvoker reset([listener]() {
        listener->ResetEventGroupStates();
    });

    if (listener->axis_scroll_type_ != kNo_ScrollType)
    {
        auto surface = extract_surface_from_pointer(data, pointer);
        if (!surface)
        {
            QLOG(LOG_ERROR, "Compositor notified us an axis event of a pointer "
                            "which is not hovering on any surfaces");
            return;
        }

        // FIXME(sora): Is it possible that the normal scroll and high-resolution scroll
        //              happen in the same event group (or "frame" in Wayland's spec)?
        //              If so, we must handle it properly.
        if (listener->axis_scroll_type_ == kNormal_ScrollType)
        {
            PresentSignal info;
            info.EmplaceBack<AxisSourceType>(listener->axis_source_type_);
            info.EmplaceBack<double>(listener->axis_values_[kX]);
            info.EmplaceBack<double>(listener->axis_values_[kY]);
            surface->Emit(GLSI_SURFACE_POINTER_AXIS, std::move(info));
        }
        else if (listener->axis_scroll_type_ == kHighres_ScrollType)
        {
            PresentSignal info;
            info.EmplaceBack<AxisSourceType>(listener->axis_source_type_);
            info.EmplaceBack<int32_t>(listener->axis_highres_scroll_[kX]);
            info.EmplaceBack<int32_t>(listener->axis_highres_scroll_[kY]);
            surface->Emit(GLSI_SURFACE_POINTER_HIGHRES_SCROLL, std::move(info));
        }
        else
        {
            MARK_UNREACHABLE("Unexpected enumeration value");
        }
    }
}

GLAMOR_NAMESPACE_END
