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

#include "include/core/SkSurface.h"
#include "include/core/SkColorSpace.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/vk/GrVkExtensions.h"
#include "include/gpu/vk/GrVkBackendContext.h"
#include "include/gpu/GrDirectContext.h"

#include "Core/Journal.h"
#include "Glamor/HWComposeContext.h"
#include "Glamor/HWComposeOffscreen.h"
#include "Glamor/HWComposeDevice.h"
GLAMOR_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Glamor.HWComposeOffscreen)

std::unique_ptr<HWComposeOffscreen>
HWComposeOffscreen::Make(const std::shared_ptr<HWComposeContext> &context)
{
    HWComposeDevice::DeviceQueueSpecifier device_graphics_queue_spec{
        .selector = HWComposeDevice::DeviceQueueSelector::kGraphics,
        .count = 1,
        .priorities = {1.0f},
        .present_surface = VK_NULL_HANDLE
    };
    std::shared_ptr<HWComposeDevice> device =
            HWComposeDevice::Make(context, {device_graphics_queue_spec}, {});

    if (!device)
        return nullptr;

    auto offscreen = std::make_unique<HWComposeOffscreen>();

    bool success = offscreen->InitializeSkiaGpuContext(SkiaGpuContextCreateInfo{
        .hw_context = context,
        .hw_device = device,
        .graphics_queue_index = 0
    });
    if (!success)
    {
        QLOG(LOG_ERROR, "Failed to create Skia GPU context");
        return nullptr;
    }

    return offscreen;
}

HWComposeOffscreen::HWComposeOffscreen() = default;

GLAMOR_NAMESPACE_END
