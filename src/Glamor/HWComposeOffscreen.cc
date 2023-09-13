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

namespace {

PFN_vkVoidFunction vk_skia_proc_getter(const char *sym,
                                       VkInstance instance,
                                       VkDevice device)
{
    if (device != VK_NULL_HANDLE)
        return vkGetDeviceProcAddr(device, sym);
    return vkGetInstanceProcAddr(instance, sym);
}

void populate_gr_vk_extensions(GrVkExtensions& ext,
                               const std::shared_ptr<HWComposeContext>& ctx,
                               const std::shared_ptr<HWComposeDevice>& device)
{
    std::vector<const char *> instance_ext;
    instance_ext.reserve(ctx->GetInstanceEnabledExtensions().size());
    for (const auto& str : ctx->GetInstanceEnabledExtensions())
        instance_ext.push_back(str.c_str());

    std::vector<const char *> device_ext;
    device_ext.reserve(device->GetEnabledExtensions().size());
    for (const auto& str : device->GetEnabledExtensions())
        device_ext.push_back(str.c_str());

    ext.init(vk_skia_proc_getter,
             ctx->GetVkInstance(),
             ctx->GetVkPhysicalDevice(),
             instance_ext.size(),
             instance_ext.data(),
             device_ext.size(),
             device_ext.data());
}

struct GrContextDeviceClosure
{
    std::shared_ptr<HWComposeDevice> device;
};

} // namespace anonymous

Unique<HWComposeOffscreen>
HWComposeOffscreen::Make(const Shared<HWComposeContext> &context)
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

    // Create Skia GPU direct context
    GrVkExtensions extensions;
    populate_gr_vk_extensions(extensions, context, device);

    VkPhysicalDevice physical_device = context->GetVkPhysicalDevice();

    VkPhysicalDeviceFeatures2 features{};
    features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    vkGetPhysicalDeviceFeatures2(physical_device, &features);

    using Selector = HWComposeDevice::DeviceQueueSelector;
    auto graphics_queue = device->GetDeviceQueue(Selector::kGraphics, 0);
    CHECK(graphics_queue);

    GrVkBackendContext backend{};
    backend.fInstance = context->GetVkInstance();
    backend.fPhysicalDevice = physical_device;
    backend.fDevice = device->GetVkDevice();
    backend.fQueue = graphics_queue->queue;
    backend.fGraphicsQueueIndex = graphics_queue->family_index;
    backend.fMaxAPIVersion = VK_API_VERSION_1_2;
    backend.fVkExtensions = &extensions;
    backend.fGetProc = vk_skia_proc_getter;
    backend.fDeviceFeatures2 = &features;

    GrContextOptions context_options;
    // Keep a reference of `HWComposeDevice` for the `GrDirectContext`
    // so that the device resources will not be destroyed before
    // `GrDirectContext` is deleted.
    context_options.fContextDeleteContext = new GrContextDeviceClosure{device};
    context_options.fContextDeleteProc = [](void *userdata) {
        CHECK(userdata);
        delete reinterpret_cast<GrContextDeviceClosure*>(userdata);
    };

    sk_sp<GrDirectContext> direct_ctx = GrDirectContext::MakeVulkan(
            backend, context_options);
    if (!direct_ctx)
    {
        QLOG(LOG_ERROR, "Failed to create GrDirectContext from device");
        return nullptr;
    }

    return std::make_unique<HWComposeOffscreen>(
            std::move(device), std::move(direct_ctx));
}

HWComposeOffscreen::HWComposeOffscreen(Shared<HWComposeDevice> device,
                                       sk_sp<GrDirectContext> direct_context)
    : device_(std::move(device))
{
    TakeOverSkiaGpuContext(std::move(direct_context));
}

HWComposeDevice *HWComposeOffscreen::OnGetHWComposeDevice()
{
    CHECK(device_ && "Disposed GPU context");
    return device_.get();
}

GLAMOR_NAMESPACE_END
