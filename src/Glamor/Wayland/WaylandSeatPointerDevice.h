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

#ifndef COCOA_GLAMOR_WAYLAND_WAYLANDSEATPOINTERDEVICE_H
#define COCOA_GLAMOR_WAYLAND_WAYLANDSEATPOINTERDEVICE_H

#include <wayland-client.h>

#include "Glamor/Glamor.h"
GLAMOR_NAMESPACE_BEGIN

class WaylandSeat;

class WaylandSeatPointerDevice
{
public:
    enum AxisValueSelector
    {
        kX,
        kY
    };

    enum AxisScrollType
    {
        kNo_ScrollType,
        kHighres_ScrollType,
        kNormal_ScrollType
    };

    explicit WaylandSeatPointerDevice(WaylandSeat *seat,
                                      wl_pointer *pointer);

    ~WaylandSeatPointerDevice();

    static std::unique_ptr<WaylandSeatPointerDevice>
    MakeFromPointerDevice(WaylandSeat *seat, wl_pointer *pointer);

    g_nodiscard g_inline WaylandSeat *GetSeat() const {
        return seat_;
    }

    void ResetEventGroupStates();

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
                        wl_fixed_t value);
    static void on_frame(void *data, wl_pointer *pointer);
    static void on_axis_source(void *data,
                               wl_pointer *pointer,
                               uint32_t axis_source);
    static void on_axis_stop(void *data,
                             wl_pointer *pointer,
                             uint32_t time,
                             uint32_t axis);
    static void on_axis_discrete(void *data,
                                 wl_pointer *pointer,
                                 uint32_t axis,
                                 int32_t discrete);
    static void on_axis_value120(void *data,
                                 wl_pointer *pointer,
                                 uint32_t axis,
                                 int32_t value120);

private:
    WaylandSeat     *seat_;
    wl_pointer      *pointer_device_;

    double           axis_values_[2];
    int32_t          axis_highres_scroll_[2];
    AxisSourceType   axis_source_type_;
    AxisScrollType   axis_scroll_type_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_WAYLAND_WAYLANDSEATPOINTERDEVICE_H
