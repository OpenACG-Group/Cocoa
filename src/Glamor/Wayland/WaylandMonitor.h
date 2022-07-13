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
                                       wl_output *output);

    WaylandMonitor(const Weak<WaylandDisplay>& display, wl_output *output);
    ~WaylandMonitor() override = default;

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
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_WAYLAND_WAYLANDMONITOR_H
