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

#ifndef COCOA_GLAMOR_HWCOMPOSEDEVICE_H
#define COCOA_GLAMOR_HWCOMPOSEDEVICE_H

#include <vector>
#include <string>
#include <optional>

#include <vulkan/vulkan.h>

#include "Glamor/Glamor.h"
#include "Glamor/GraphicsResourcesTrackable.h"
GLAMOR_NAMESPACE_BEGIN

class HWComposeContext;

/**
 * Represents a logical GPU device (created from physical device),
 * and could be used to create Skia's GPU context for rendering.
 *
 * `HWComposeDevice` can be used to create either a `HWComposeOffscreen`
 * object (for offscreen rendering) or a `HWComposeSwapchain` object
 * (for onscreen rendering). Both of them will create a Skia GPU
 * context from the device, which can be used for rendering.
 *
 * Multiple logical devices can be created from a physical device.
 * (That is, they are created from the same `HWComposeContext`).
 * In that case, those logical devices are in the same device group,
 * and they could share device memory objects (like texture-backed
 * `SkImage` objects) without copying memory. That is implemented via
 * exporting memory objects from a device first and then importing
 * the memory objects into another device. They also can share sync
 * primitives (fences, semaphores, ...), which is implemented via
 * the same way to memory sharing.
 *
 * For example, consider the hierarchy of devices:
 * \code
 * HWComposeContext [P] ------+-----> HWComposeDevice [A]
 *                            |
 *                            +-----> HWComposeDevice [B]
 *                            |
 *                            +-----> HWComposeDevice [C]
 *
 * HWComposeContext [Q] ------------> HWComposeDevice [D]
 * \endcode
 *
 * Logical device A, B, C can share memory and sync primitives
 * between each other, as they are created from the same physical
 * device (`HWComposeContext`) P. However, if P,Q use different
 * physical devices, it is impossible to share sync primitives
 * between A and D, and it is also impossible to share memory objects
 * without the help of CPU memory (memory of device A must be transferred
 * from device A to CPU memory first, and then be transferred from CPU
 * memory to device B).
 */
class HWComposeDevice : public GraphicsResourcesTrackable
{
public:
    enum class DeviceQueueSelector
    {
        // Requires `VK_QUEUE_GRAPHICS_BIT`. Queue will be used for
        // graphics rendering pipeline.
        kGraphics,

        // Requires `VkSurfaceKHR` support. Queue will be used for
        // presenting frames on a certain window.
        kPresent

        // TODO(sora): Compute queue may be supported in the future
    };

    struct DeviceQueueSpecifier
    {
        DeviceQueueSelector selector;
        int32_t count;
        std::vector<float> priorities;

        // When `selector` is `kPresent`, this is used to determine whether a VkQueue
        // has present support. Otherwise, caller should leave this field `VK_NULL_HANDLE`.
        VkSurfaceKHR present_surface;
    };

    struct DeviceQueue
    {
        VkQueue queue;
        int32_t family_index;
    };
    using QueueMultiMap = std::unordered_map<DeviceQueueSelector, std::vector<DeviceQueue>>;

    static std::unique_ptr<HWComposeDevice> Make(
            const std::shared_ptr<HWComposeContext>& context,
            const std::vector<DeviceQueueSpecifier>& queue_specs,
            const std::vector<std::string>& extra_device_ext);

    HWComposeDevice(std::shared_ptr<HWComposeContext> context,
                    std::vector<std::string> enabled_extensions,
                    VkDevice vk_device,
                    QueueMultiMap device_queue_multimap);
    ~HWComposeDevice() override;

    g_nodiscard g_inline VkDevice GetVkDevice() const {
        return vk_device_;
    }

    g_nodiscard g_inline std::shared_ptr<HWComposeContext> GetHWComposeContext() const {
        return context_;
    }

    g_nodiscard g_inline const std::vector<std::string>& GetEnabledExtensions() const {
        return enabled_extensions_;
    }

    g_nodiscard std::optional<DeviceQueue> GetDeviceQueue(DeviceQueueSelector selector,
                                                          int32_t index);

    void Trace(Tracer *tracer) noexcept override;

private:
    std::shared_ptr<HWComposeContext>   context_;
    std::vector<std::string>            enabled_extensions_;
    VkDevice                            vk_device_;
    QueueMultiMap                       device_queue_multimap_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_HWCOMPOSEDEVICE_H
