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

#ifndef COCOA_GLAMOR_WAYLAND_WAYLANDMONITOR_H
#define COCOA_GLAMOR_WAYLAND_WAYLANDMONITOR_H

#include <wayland-client.h>

#include "Glamor/Glamor.h"
#include "Glamor/Monitor.h"
GLAMOR_NAMESPACE_BEGIN

class WaylandDisplay;

class WaylandMonitor : public Monitor
{
public:
    g_nodiscard g_inline static WaylandMonitor *BareCast(void *ptr) {
        CHECK(ptr && "Invalid pointer of WaylandMonitor");
        return reinterpret_cast<WaylandMonitor*>(ptr);
    }

    static Shared<WaylandMonitor> Make(const Shared<WaylandDisplay>& display,
                                       wl_output *output,
                                       uint32_t registry_id);

    WaylandMonitor(const Weak<WaylandDisplay>& display, wl_output *output, uint32_t registry_id);
    ~WaylandMonitor() override = default;

    g_nodiscard g_inline uint32_t GetOutputRegistryId() const {
        return output_registry_id_;
    }

    g_nodiscard g_inline wl_output *GetWaylandOutput() const {
        return wl_output_;
    }

    static void OutputEventGeometry(void *data,
                                    wl_output *output,
                                    int32_t x,
                                    int32_t y,
                                    int32_t physical_width,
                                    int32_t physical_height,
                                    int32_t subpixel,
                                    const char *make,
                                    const char *model,
                                    int32_t transform);

    static void OutputEventMode(void *data,
                                wl_output *output,
                                uint32_t flags,
                                int32_t width,
                                int32_t height,
                                int32_t refresh);

    static void OutputEventDone(void *data, wl_output *output);

    static void OutputEventScale(void *data,
                                 wl_output *output,
                                 int32_t factor);

    static void OutputEventName(void *data,
                                wl_output *output,
                                const char *name);

    static void OutputEventDescription(void *data,
                                       wl_output *output,
                                       const char *description);

private:
    wl_display          *wl_display_;
    wl_output           *wl_output_;
    uint32_t             output_registry_id_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_WAYLAND_WAYLANDMONITOR_H
