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

#ifndef COCOA_GLAMOR_MONITOR_H
#define COCOA_GLAMOR_MONITOR_H

#include "include/core/SkM44.h"

#include "Core/EnumClassBitfield.h"
#include "Glamor/Glamor.h"
#include "Glamor/PresentRemoteHandle.h"
GLAMOR_NAMESPACE_BEGIN

class Display;

#define GLAMOR_MONITOR_DEFAULT_MANUFACTURE      "OpenACG Group"
#define GLAMOR_MONITOR_DEFAULT_MODEL            "Glamor Visual"
#define GLAMOR_MONITOR_DEFAULT_CONNECTOR        "DEFAULT-0"
#define GLAMOR_MONITOR_DEFAULT_DESCRIPTION      "Default Wayland Monitor"

#define GLOP_MONITOR_REQUEST_PROPERTIES     1

#define GLSI_MONITOR_PROPERTIES_CHANGED     1
#define GLSI_MONITOR_DETACHED               2

enum class MonitorSubpixel
{
    kUnknown,
    kNone,
    kHorizontalRGB,
    kHorizontalBGR,
    kVerticalRGB,
    kVerticalBGR
};

enum class MonitorTransform
{
    kNormal,        // No transform
    kRotate90,      // 90 degrees counter-clockwise
    kRotate180,     // 180 degrees counter-clockwise
    kRotate270,     // 270 degrees counter-clockwise
    kFlipped,       // 180 degrees around a vertical axis
    kFlipped90,     // flip and rotate 90 degrees counter-clockwise
    kFlipped180,    // flip and rotate 180 degrees counter-clockwise
    kFlipped270,    // flip and rotate 270 degrees counter-clockwise
};

enum class MonitorMode : uint32_t
{
    kCurrent    = 0x01,  // indicates this is the current mode
    kPreferred  = 0x02   // indicates this is the preferred mode
};

class Monitor : public PresentRemoteHandle
{
public:
    struct PropertySet
    {
        SkIVector               logical_position;
        SkIVector               physical_metrics;
        MonitorSubpixel         subpixel;
        std::string             manufacture_name;
        std::string             model_name;
        MonitorTransform        transform;
        Bitfield<MonitorMode>   mode_flags;
        SkIVector               mode_size;
        int32_t                 refresh_rate_mhz;
        int32_t                 scale_factor;
        std::string             connector_name;
        std::string             description;
    };

    explicit Monitor(const Weak<Display>& display);
    ~Monitor() override = default;

    g_nodiscard g_inline Shared<Display> GetDisplay() const {
        return display_.lock();
    }

    g_nodiscard g_inline uint32_t GetUniqueId() const {
        return unique_id_;
    }

    g_async_api void RequestProperties();

    g_private_api PropertySet GetCurrentProperties() const;

private:
    Weak<Display>           display_;
    uint32_t                unique_id_;

protected:
    void NotifyPropertiesChanged();

    int32_t                 logical_x_;
    int32_t                 logical_y_;
    int32_t                 physical_width_;
    int32_t                 physical_height_;
    MonitorSubpixel         subpixel_;
    std::string             manufacture_name_;
    std::string             model_name_;
    MonitorTransform        transform_;
    Bitfield<MonitorMode>   mode_flags_;
    int32_t                 mode_width_;
    int32_t                 mode_height_;
    int32_t                 refresh_rate_mhz_;
    int32_t                 scale_factor_;
    std::string             connector_name_;
    std::string             description_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_MONITOR_H
