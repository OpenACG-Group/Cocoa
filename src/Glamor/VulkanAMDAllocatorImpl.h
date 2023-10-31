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

#ifndef COCOA_GLAMOR_VULKANAMDALLOCATORIMPL_H
#define COCOA_GLAMOR_VULKANAMDALLOCATORIMPL_H

#include <vulkan/vulkan_core.h>

#include "vk_mem_alloc.h"
#include "include/gpu/vk/VulkanMemoryAllocator.h"
#include "include/gpu/vk/VulkanExtensions.h"

#include "Glamor/Glamor.h"
GLAMOR_NAMESPACE_BEGIN

class VulkanAMDAllocatorImpl : public skgpu::VulkanMemoryAllocator
{
public:
    using BackendMemory = skgpu::VulkanBackendMemory;

    static sk_sp<VulkanAMDAllocatorImpl> Make(VkInstance instance,
                                              VkPhysicalDevice physical_device,
                                              VkDevice device,
                                              uint32_t vk_api_version,
                                              bool external_sync,
                                              const skgpu::VulkanExtensions *extensions,
                                              bool force_coherent_host_visible_mem);

    VulkanAMDAllocatorImpl(VmaAllocator allocator, bool force_coherent_host_visible_mem);
    ~VulkanAMDAllocatorImpl() override;

    g_nodiscard VmaAllocator GetAllocator() {
        return vma_allocator_;
    }

    VkResult allocateImageMemory(VkImage image, uint32_t flags, BackendMemory *memory) override;

    VkResult allocateBufferMemory(VkBuffer buffer, BufferUsage usage,
                                  uint32_t flags, BackendMemory *memory) override;

    void freeMemory(const BackendMemory& memory) override;

    void getAllocInfo(const BackendMemory& memory, skgpu::VulkanAlloc *alloc) const override;

    VkResult mapMemory(const BackendMemory& memory, void **data) override;

    void unmapMemory(const BackendMemory& memory) override;

    VkResult flushMemory(const BackendMemory& memory, VkDeviceSize offset, VkDeviceSize size) override;

    VkResult invalidateMemory(const BackendMemory& memory, VkDeviceSize offset, VkDeviceSize size) override;

    std::pair<uint64_t, uint64_t> totalAllocatedAndUsedMemory() const override;

private:
    VmaAllocator vma_allocator_;
    bool         force_coherent_host_visible_mem_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_VULKANAMDALLOCATORIMPL_H
