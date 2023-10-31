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

#ifndef COCOA_GLAMOR_SKIAGPUCONTEXTOWNER_H
#define COCOA_GLAMOR_SKIAGPUCONTEXTOWNER_H

#include <list>
#include <vulkan/vulkan_core.h>

#include "include/gpu/GrDirectContext.h"

#include "Glamor/Glamor.h"
#include "Glamor/GraphicsResourcesTrackable.h"
#include "Glamor/VulkanAMDAllocatorImpl.h"
GLAMOR_NAMESPACE_BEGIN

class HWComposeDevice;

struct SkiaGpuContextCreateInfo
{
    std::shared_ptr<HWComposeContext> hw_context;
    std::shared_ptr<HWComposeDevice> hw_device;

    // Queue index is defined by `HWComposeDevice::DeviceQueueSpecifier` when
    // `HWComposeDevice` is created.
    int32_t graphics_queue_index;
};

class SkiaGpuContextOwner : public GraphicsResourcesTrackable
{
public:
    SkiaGpuContextOwner();
    ~SkiaGpuContextOwner() override = default;

    g_nodiscard GrDirectContext *GetSkiaGpuContext() const {
        return direct_context_.get();
    }

    g_nodiscard sk_sp<VulkanAMDAllocatorImpl> GetAllocator() const {
        return vk_allocator_;
    }

    int32_t ExportSemaphoreFd(VkSemaphore semaphore);
    VkSemaphore ImportSemaphoreFromFd(int32_t fd);

    int32_t ExportDeviceMemoryFd(VkDeviceMemory memory);
    VkDeviceMemory ImportDeviceMemoryFromFd(int32_t fd, uint32_t memory_type_index, VkDeviceSize size);

    struct ExportedSkSurfaceInfo
    {
        int32_t fd;
        uint32_t memory_type_index;
        uint64_t size;
        uint64_t offset;
        uint32_t width;
        uint32_t height;
        VkImageTiling image_tilling;
        VkFormat image_format;
        uint32_t sample_count;
        uint32_t level_count;
        SkColorType sk_color_type;
    };

    std::optional<ExportedSkSurfaceInfo> ExportSkSurface(const sk_sp<SkSurface>& surface);
    sk_sp<SkSurface> ImportSkSurface(const ExportedSkSurfaceInfo& exported_info);

    VkDevice GetVkDevice();

    g_nodiscard std::shared_ptr<HWComposeDevice> GetDevice() const {
        return hw_device_;
    }

    void Trace(Tracer *tracer) noexcept override;

protected:
    bool InitializeSkiaGpuContext(const SkiaGpuContextCreateInfo& create_info);

    void DisposeSkiaGpuContext();

private:
    std::shared_ptr<HWComposeDevice> hw_device_;
    sk_sp<GrDirectContext>          direct_context_;
    sk_sp<VulkanAMDAllocatorImpl>   vk_allocator_;
    bool                            device_support_memory_sharing_;
    bool                            device_support_semaphore_sharing_;

    PFN_vkGetSemaphoreFdKHR         pfn_vkGetSemaphoreFdKHR_;
    PFN_vkImportSemaphoreFdKHR      pfn_vkImportSemaphoreFdKHR_;
    PFN_vkGetMemoryFdKHR            pfn_vkGetMemoryFdKHR_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_SKIAGPUCONTEXTOWNER_H
