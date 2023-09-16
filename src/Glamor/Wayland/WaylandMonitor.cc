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

#include <wayland-client.h>

#include "fmt/format.h"

#include "Core/Journal.h"
#include "Glamor/Glamor.h"
#include "Glamor/Wayland/WaylandMonitor.h"
#include "Glamor/Wayland/WaylandDisplay.h"
GLAMOR_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Glamor.Wayland.Monitor)

namespace {

struct {
    wl_output_subpixel subpixel;
    const char *name;
    MonitorSubpixel typed_enum;
} g_subpixel_name_map[] = {
    {WL_OUTPUT_SUBPIXEL_UNKNOWN, "Unknown", MonitorSubpixel::kUnknown},
    {WL_OUTPUT_SUBPIXEL_NONE, "None", MonitorSubpixel::kNone},
    {WL_OUTPUT_SUBPIXEL_HORIZONTAL_RGB, "Horizontal RGB", MonitorSubpixel::kHorizontalRGB},
    {WL_OUTPUT_SUBPIXEL_HORIZONTAL_BGR, "Horizontal BGR", MonitorSubpixel::kHorizontalBGR},
    {WL_OUTPUT_SUBPIXEL_VERTICAL_RGB, "Vertical RGB", MonitorSubpixel::kVerticalRGB},
    {WL_OUTPUT_SUBPIXEL_VERTICAL_BGR, "Vertical BGR", MonitorSubpixel::kVerticalBGR}
};

struct {
    wl_output_transform transform;
    const char *name;
    MonitorTransform typed_enum;
} g_transform_name_map[] = {
    {WL_OUTPUT_TRANSFORM_NORMAL, "Normal", MonitorTransform::kNormal},
    {WL_OUTPUT_TRANSFORM_90, "90 deg", MonitorTransform::kRotate90},
    {WL_OUTPUT_TRANSFORM_180, "180 deg", MonitorTransform::kRotate180},
    {WL_OUTPUT_TRANSFORM_270, "270 deg", MonitorTransform::kRotate270},
    {WL_OUTPUT_TRANSFORM_FLIPPED, "Flipped", MonitorTransform::kFlipped},
    {WL_OUTPUT_TRANSFORM_FLIPPED_90, "Flipped, 90 deg", MonitorTransform::kFlipped90},
    {WL_OUTPUT_TRANSFORM_FLIPPED_180, "Flipped, 180 deg", MonitorTransform::kFlipped180},
    {WL_OUTPUT_TRANSFORM_FLIPPED_270, "Flipped, 270 deg", MonitorTransform::kFlipped270}
};

const wl_output_listener g_output_listener = {
    .geometry = WaylandMonitor::OutputEventGeometry,
    .mode = WaylandMonitor::OutputEventMode,
    .done = WaylandMonitor::OutputEventDone,
    .scale = WaylandMonitor::OutputEventScale,
    .name = WaylandMonitor::OutputEventName,
    .description = WaylandMonitor::OutputEventDescription
};

} // namespace anonymous

void WaylandMonitor::OutputEventGeometry(void *data,
                                         wl_output *output,
                                         int32_t x,
                                         int32_t y,
                                         int32_t physical_width,
                                         int32_t physical_height,
                                         int32_t subpixel,
                                         const char *make,
                                         const char *model,
                                         int32_t transform)
{
    WaylandMonitor *monitor = WaylandMonitor::BareCast(data);

    MonitorSubpixel typed_subpixel = MonitorSubpixel::kUnknown;
    MonitorTransform typed_transform = MonitorTransform::kNormal;

    for (const auto& pair : g_subpixel_name_map)
    {
        if (pair.subpixel == subpixel)
        {
            typed_subpixel = pair.typed_enum;
            break;
        }
    }

    for (const auto& pair : g_transform_name_map)
    {
        if (pair.transform == transform)
        {
            typed_transform = pair.typed_enum;
            break;
        }
    }

    monitor->logical_x_ = x;
    monitor->logical_y_ = y;
    monitor->physical_width_ = physical_width;
    monitor->physical_height_ = physical_height;
    monitor->subpixel_ = typed_subpixel;
    monitor->manufacture_name_ = make;
    monitor->model_name_ = model;
    monitor->transform_ = typed_transform;
}

void WaylandMonitor::OutputEventMode(void *data,
                                     wl_output *output,
                                     uint32_t flags,
                                     int32_t width,
                                     int32_t height,
                                     int32_t refresh)
{
    WaylandMonitor *monitor = WaylandMonitor::BareCast(data);

    monitor->refresh_rate_mhz_ = refresh;
    monitor->mode_width_ = width;
    monitor->mode_height_ = height;

    monitor->mode_flags_.clear();
    if (flags & WL_OUTPUT_MODE_CURRENT)
        monitor->mode_flags_ |= MonitorMode::kCurrent;
    if (flags & WL_OUTPUT_MODE_PREFERRED)
        monitor->mode_flags_ |= MonitorMode::kPreferred;
}

void WaylandMonitor::OutputEventDone(void *data, wl_output *output)
{
    WaylandMonitor *monitor = WaylandMonitor::BareCast(data);
    monitor->NotifyPropertiesChanged();
}

void WaylandMonitor::OutputEventScale(void *data,
                                      wl_output *output,
                                      int32_t factor)
{
    WaylandMonitor *monitor = WaylandMonitor::BareCast(data);
    monitor->scale_factor_ = factor;
}

void WaylandMonitor::OutputEventName(void *data,
                                     wl_output *output,
                                     const char *name)
{
    WaylandMonitor *monitor = WaylandMonitor::BareCast(data);
    monitor->connector_name_ = name;
}

void WaylandMonitor::OutputEventDescription(void *data,
                                            wl_output *output,
                                            const char *description)
{
    WaylandMonitor *monitor = WaylandMonitor::BareCast(data);
    monitor->description_ = description;
}

std::shared_ptr<WaylandMonitor>
WaylandMonitor::Make(const std::shared_ptr<WaylandDisplay>& display,
                     wl_output *output, uint32_t registry_id)
{
    auto monitor = std::make_shared<WaylandMonitor>(display, output, registry_id);

    // callback functions will not be called immediately, but they will be called
    //  in the next roundtrip which is performed in `WaylandDisplay::Connect`.
    wl_output_add_listener(output, &g_output_listener, monitor.get());
    return monitor;
}

WaylandMonitor::WaylandMonitor(const std::weak_ptr<WaylandDisplay>& display,
                               wl_output *output, uint32_t registry_id)
    : Monitor(display)
    , wl_display_(display.lock()->GetWaylandDisplay())
    , wl_output_(output)
    , output_registry_id_(registry_id)
{
}

GLAMOR_NAMESPACE_END
