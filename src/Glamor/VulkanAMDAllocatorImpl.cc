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

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnullability-completeness"
#pragma clang diagnostic ignored "-Wunused-variable"
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
#pragma clang diagnostic pop

#include "Glamor/VulkanAMDAllocatorImpl.h"
GLAMOR_NAMESPACE_BEGIN

sk_sp<VulkanAMDAllocatorImpl> VulkanAMDAllocatorImpl::Make(VkInstance instance,
                                                           VkPhysicalDevice physical_device,
                                                           VkDevice device,
                                                           uint32_t vk_api_version,
                                                           bool external_sync,
                                                           const skgpu::VulkanExtensions *extensions,
                                                           bool force_coherent_host_visible_mem)
{
    VmaAllocatorCreateInfo create_info{};

    if (external_sync)
        create_info.flags |= VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT;

    if (vk_api_version >= VK_MAKE_VERSION(1, 1, 0) ||
        (extensions->hasExtension(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME, 1) &&
         extensions->hasExtension(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME, 1)))
    {
        create_info.flags |= VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;
    }

    create_info.physicalDevice = physical_device;
    create_info.device = device;
    // 4MB was picked for the size here by following Skia's default value.
    // It seems to be a good compromise of not wasting unused allocated space and not making
    // too many small allocations. The AMD allocator will start making blocks at 1/8 the max
    // size and builds up block size as needed before capping at the max set here.
    // For more details, see the `VulkanAMDMemoryAllocator::Make` function implemented in
    // `//third_party/skia/src/gpu/vk/VulkanAMDMemoryAllocator.cpp`.
    create_info.preferredLargeHeapBlockSize = 4 * 1024 * 1024;
    create_info.pAllocationCallbacks = nullptr;
    create_info.pDeviceMemoryCallbacks = nullptr;
    create_info.pHeapSizeLimit = nullptr;
    create_info.pVulkanFunctions = nullptr;
    create_info.instance = instance;
    create_info.vulkanApiVersion = std::min(vk_api_version, VK_MAKE_VERSION(1, 2, 0));

    VmaAllocator allocator;
    vmaCreateAllocator(&create_info, &allocator);

    return sk_make_sp<VulkanAMDAllocatorImpl>(allocator, force_coherent_host_visible_mem);
}

VulkanAMDAllocatorImpl::VulkanAMDAllocatorImpl(VmaAllocator allocator,
                                               bool force_coherent_host_visible_mem)
    : vma_allocator_(allocator)
    , force_coherent_host_visible_mem_(force_coherent_host_visible_mem)
{
}

VulkanAMDAllocatorImpl::~VulkanAMDAllocatorImpl()
{
    vmaDestroyAllocator(vma_allocator_);
    vma_allocator_ = VK_NULL_HANDLE;
}

VkResult VulkanAMDAllocatorImpl::allocateImageMemory(VkImage image, uint32_t flags, BackendMemory *memory)
{
    CHECK(memory);

    VmaAllocationCreateInfo info{};
    info.flags = 0;
    info.usage = VMA_MEMORY_USAGE_UNKNOWN;
    info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    info.preferredFlags = 0;
    info.memoryTypeBits = 0;
    info.pool = VK_NULL_HANDLE;
    info.pUserData = nullptr;

    if (flags & kDedicatedAllocation_AllocationPropertyFlag)
        info.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    if (flags & kLazyAllocation_AllocationPropertyFlag)
        info.requiredFlags |= VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
    if (flags & kProtected_AllocationPropertyFlag)
        info.requiredFlags |= VK_MEMORY_PROPERTY_PROTECTED_BIT;

    VmaAllocation alloc;
    VkResult result = vmaAllocateMemoryForImage(vma_allocator_, image, &info, &alloc, nullptr);
    if (result == VK_SUCCESS)
        *memory = reinterpret_cast<BackendMemory>(alloc);

    return result;
}

VkResult VulkanAMDAllocatorImpl::allocateBufferMemory(VkBuffer buffer, BufferUsage usage,
                                                      uint32_t flags, BackendMemory *memory)
{
    CHECK(memory);

    VmaAllocationCreateInfo info{};
    info.flags = 0;
    info.usage = VMA_MEMORY_USAGE_UNKNOWN;
    info.memoryTypeBits = 0;
    info.pool = VK_NULL_HANDLE;
    info.pUserData = nullptr;

    switch (usage)
    {
    case BufferUsage::kGpuOnly:
        info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        info.preferredFlags = 0;
        break;
    case BufferUsage::kCpuWritesGpuReads:
        info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                           | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        info.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        break;
    case BufferUsage::kTransfersFromCpuToGpu:
        info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                             | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        info.preferredFlags = 0;
        break;
    case BufferUsage::kTransfersFromGpuToCpu:
        info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        info.preferredFlags = VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
        break;
    }

    if (force_coherent_host_visible_mem_ &&
        (info.requiredFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
    {
        info.requiredFlags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    }

    if (flags & kDedicatedAllocation_AllocationPropertyFlag)
        info.requiredFlags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    if ((flags & kLazyAllocation_AllocationPropertyFlag) && usage == BufferUsage::kGpuOnly)
        info.preferredFlags |= VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;

    if (flags & kPersistentlyMapped_AllocationPropertyFlag)
    {
        CHECK(usage != BufferUsage::kGpuOnly);
        info.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
    }

    VmaAllocation alloc;
    VkResult result = vmaAllocateMemoryForBuffer(vma_allocator_, buffer, &info, &alloc, nullptr);
    if (result == VK_SUCCESS)
        *memory = reinterpret_cast<BackendMemory>(alloc);

    return result;
}

void VulkanAMDAllocatorImpl::freeMemory(const BackendMemory& memory)
{
    const auto alloc = reinterpret_cast<VmaAllocation>(memory);
    vmaFreeMemory(vma_allocator_, alloc);
}

void VulkanAMDAllocatorImpl::getAllocInfo(const BackendMemory& memory,
                                          skgpu::VulkanAlloc *alloc) const
{
    CHECK(alloc);

    const auto vma_alloc = reinterpret_cast<VmaAllocation>(memory);
    VmaAllocationInfo vma_alloc_info{};
    vmaGetAllocationInfo(vma_allocator_, vma_alloc, &vma_alloc_info);

    VkMemoryPropertyFlags mem_flags;
    vmaGetMemoryTypeProperties(vma_allocator_, vma_alloc_info.memoryType, &mem_flags);

    uint32_t flags = 0;
    if (mem_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        flags |= skgpu::VulkanAlloc::kMappable_Flag;
    if (!SkToBool(mem_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
        flags |= skgpu::VulkanAlloc::kNoncoherent_Flag;
    if (mem_flags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)
        flags |= skgpu::VulkanAlloc::kLazilyAllocated_Flag;

    alloc->fMemory = vma_alloc_info.deviceMemory;
    alloc->fOffset = vma_alloc_info.offset;
    alloc->fSize = vma_alloc_info.size;
    alloc->fFlags = flags;
    alloc->fBackendMemory = memory;
}

VkResult VulkanAMDAllocatorImpl::mapMemory(const BackendMemory& memory, void **data)
{
    const auto vma_alloc = reinterpret_cast<VmaAllocation>(memory);
    return vmaMapMemory(vma_allocator_, vma_alloc, data);
}

void VulkanAMDAllocatorImpl::unmapMemory(const BackendMemory& memory)
{
    const auto vma_alloc = reinterpret_cast<VmaAllocation>(memory);
    vmaUnmapMemory(vma_allocator_, vma_alloc);
}

VkResult
VulkanAMDAllocatorImpl::flushMemory(const BackendMemory& memory, VkDeviceSize offset, VkDeviceSize size)
{
    const auto vma_alloc = reinterpret_cast<VmaAllocation>(memory);
    return vmaFlushAllocation(vma_allocator_, vma_alloc, offset, size);
}

VkResult
VulkanAMDAllocatorImpl::invalidateMemory(const BackendMemory& memory, VkDeviceSize offset, VkDeviceSize size)
{
    const auto vma_alloc = reinterpret_cast<VmaAllocation>(memory);
    return vmaInvalidateAllocation(vma_allocator_, vma_alloc, offset, size);
}

std::pair<uint64_t, uint64_t> VulkanAMDAllocatorImpl::totalAllocatedAndUsedMemory() const
{
    VmaTotalStatistics stats{};
    vmaCalculateStatistics(vma_allocator_, &stats);
    return {stats.total.statistics.blockBytes, stats.total.statistics.allocationBytes};
}

GLAMOR_NAMESPACE_END
