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

#include "include/core/SkImage.h"
#include "include/core/SkColorSpace.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/ganesh/vk/GrVkBackendSurface.h"
#include "include/gpu/ganesh/vk/GrVkDirectContext.h"
#include "include/gpu/vk/GrVkBackendContext.h"

#include "Glamor/SkiaGpuContextOwner.h"
#include "Glamor/HWComposeDevice.h"
#include "Glamor/HWComposeContext.h"
GLAMOR_NAMESPACE_BEGIN

SkiaGpuContextOwner::SkiaGpuContextOwner()
    : direct_context_(nullptr)
    , device_support_memory_sharing_(false)
    , device_support_semaphore_sharing_(false)
    , pfn_vkGetSemaphoreFdKHR_(nullptr)
    , pfn_vkImportSemaphoreFdKHR_(nullptr)
    , pfn_vkGetMemoryFdKHR_(nullptr)
{
}

namespace {

PFN_vkVoidFunction vk_skia_proc_getter(const char *sym, VkInstance instance, VkDevice device)
{
    if (device != VK_NULL_HANDLE)
        return vkGetDeviceProcAddr(device, sym);
    return vkGetInstanceProcAddr(instance, sym);
}

void populate_gr_vk_extensions(GrVkExtensions& ext, HWComposeContext& ctx, HWComposeDevice& device)
{
    std::vector<const char *> instance_ext;
    instance_ext.reserve(ctx.GetInstanceEnabledExtensions().size());
    for (const auto& str : ctx.GetInstanceEnabledExtensions())
        instance_ext.push_back(str.c_str());

    std::vector<const char *> device_ext;
    device_ext.reserve(device.GetEnabledExtensions().size());
    for (const auto& str : device.GetEnabledExtensions())
        device_ext.push_back(str.c_str());

    ext.init(vk_skia_proc_getter,
             ctx.GetVkInstance(),
             ctx.GetVkPhysicalDevice(),
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

bool SkiaGpuContextOwner::InitializeSkiaGpuContext(const SkiaGpuContextCreateInfo& create_info)
{
    // Never initialize the context twice.
    if (direct_context_)
        return false;

    if (!create_info.hw_context || !create_info.hw_device)
        return false;

    auto device = create_info.hw_device;
    auto queue = device->GetDeviceQueue(
            HWComposeDevice::DeviceQueueSelector::kGraphics, create_info.graphics_queue_index);
    if (!queue)
        return false;

    // Populate extensions and device features info
    GrVkExtensions extensions;
    populate_gr_vk_extensions(extensions, *create_info.hw_context, *device);
    VkPhysicalDevice physical_device = create_info.hw_context->GetVkPhysicalDevice();
    VkPhysicalDeviceFeatures2 features{};
    features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    vkGetPhysicalDeviceFeatures2(physical_device, &features);

    // Create Vulkan memory allocator.
    sk_sp<VulkanAMDAllocatorImpl> vk_allocator = device->CreateAllocator(false, &extensions);
    if (!vk_allocator)
        return false;

    GrVkBackendContext backend{};
    backend.fInstance = create_info.hw_context->GetVkInstance();
    backend.fPhysicalDevice = physical_device;
    backend.fDevice = device->GetVkDevice();
    backend.fQueue = queue->queue;
    backend.fGraphicsQueueIndex = queue->family_index;
    backend.fMaxAPIVersion = VK_API_VERSION_1_2;
    backend.fVkExtensions = &extensions;
    backend.fGetProc = vk_skia_proc_getter;
    backend.fDeviceFeatures2 = &features;
    backend.fMemoryAllocator = vk_allocator;

    GrContextOptions context_options;
    // Keep a reference of `HWComposeDevice` for the `GrDirectContext`
    // so that the device resources will not be destroyed before
    // `GrDirectContext` is deleted.
    context_options.fContextDeleteContext = new GrContextDeviceClosure{device};
    context_options.fContextDeleteProc = [](void *userdata) {
        CHECK(userdata);
        delete reinterpret_cast<GrContextDeviceClosure*>(userdata);
    };

    sk_sp<GrDirectContext> ctx = GrDirectContexts::MakeVulkan(backend, context_options);
    if (!ctx)
        return false;

    // Fill fields
    hw_device_ = create_info.hw_device;
    vk_allocator_ = vk_allocator;
    direct_context_ = ctx;

    device_support_memory_sharing_ =
            extensions.hasExtension(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME, 1) &&
            extensions.hasExtension(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME, 1);

    device_support_semaphore_sharing_ =
            extensions.hasExtension(VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME, 1) &&
            extensions.hasExtension(VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME, 1);

#define GET_PFN(name) reinterpret_cast<PFN_##name>(vkGetDeviceProcAddr(device->GetVkDevice(), #name))

    if (device_support_semaphore_sharing_)
    {
        pfn_vkGetSemaphoreFdKHR_ = GET_PFN(vkGetSemaphoreFdKHR);
        pfn_vkImportSemaphoreFdKHR_ = GET_PFN(vkImportSemaphoreFdKHR);
        CHECK(pfn_vkGetSemaphoreFdKHR_ && pfn_vkImportSemaphoreFdKHR_);
    }

    if (device_support_memory_sharing_)
    {
        pfn_vkGetMemoryFdKHR_ = GET_PFN(vkGetMemoryFdKHR);
        CHECK(pfn_vkGetMemoryFdKHR_);
    }

#undef GET_PFN

    return true;
}

void SkiaGpuContextOwner::DisposeSkiaGpuContext()
{
    if (!direct_context_)
        return;
    direct_context_ = nullptr;
    vk_allocator_.reset();
    hw_device_.reset();
}

VkDevice SkiaGpuContextOwner::GetVkDevice()
{
    if (!direct_context_)
        return VK_NULL_HANDLE;
    return hw_device_->GetVkDevice();
}

VkDeviceMemory SkiaGpuContextOwner::ImportDeviceMemoryFromFd(int32_t fd,
                                                             uint32_t memory_type_index,
                                                             VkDeviceSize size)
{
    if (!device_support_memory_sharing_ || !direct_context_)
        return VK_NULL_HANDLE;
    VkDevice device = hw_device_->GetVkDevice();

    VkImportMemoryFdInfoKHR import_mem_info{};
    import_mem_info.sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR;
    import_mem_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
    import_mem_info.fd = fd;
    import_mem_info.pNext = nullptr;

    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.pNext = &import_mem_info;
    alloc_info.memoryTypeIndex = memory_type_index;
    alloc_info.allocationSize = size;

    VkDeviceMemory device_memory;
    VkResult result = vkAllocateMemory(device, &alloc_info, nullptr, &device_memory);
    if (result != VK_SUCCESS)
        return VK_NULL_HANDLE;

    return device_memory;
}

int32_t SkiaGpuContextOwner::ExportDeviceMemoryFd(VkDeviceMemory memory)
{
    if (!device_support_memory_sharing_ || !direct_context_)
        return -1;
    VkDevice device = hw_device_->GetVkDevice();

    VkMemoryGetFdInfoKHR info{};
    info.sType = VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR;
    info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
    info.memory = memory;
    info.pNext = nullptr;

    int32_t fd;
    VkResult result = pfn_vkGetMemoryFdKHR_(device, &info, &fd);
    if (result != VK_SUCCESS)
        return -1;

    return fd;
}

VkSemaphore SkiaGpuContextOwner::ImportSemaphoreFromFd(int32_t fd)
{
    if (!device_support_semaphore_sharing_ || !direct_context_)
        return VK_NULL_HANDLE;

    VkDevice device = hw_device_->GetVkDevice();

    VkSemaphore semaphore;
    VkSemaphoreCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkResult result = vkCreateSemaphore(device, &create_info, nullptr, &semaphore);
    if (result != VK_SUCCESS)
        return VK_NULL_HANDLE;

    VkImportSemaphoreFdInfoKHR import_fd_info{};
    import_fd_info.sType = VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_FD_INFO_KHR;
    import_fd_info.fd = fd;
    import_fd_info.handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT;
    import_fd_info.semaphore = semaphore;
    result = pfn_vkImportSemaphoreFdKHR_(device, &import_fd_info);
    if (result != VK_SUCCESS)
    {
        vkDestroySemaphore(device, semaphore, nullptr);
        return VK_NULL_HANDLE;
    }
    return semaphore;
}

int32_t SkiaGpuContextOwner::ExportSemaphoreFd(VkSemaphore semaphore)
{
    if (!device_support_semaphore_sharing_ || !direct_context_)
        return -1;

    VkDevice device = hw_device_->GetVkDevice();

    VkSemaphoreGetFdInfoKHR get_fd_info{};
    get_fd_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_GET_FD_INFO_KHR;
    get_fd_info.handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT;
    get_fd_info.semaphore = semaphore;
    int32_t fd;
    VkResult result = pfn_vkGetSemaphoreFdKHR_(device, &get_fd_info, &fd);
    if (result != VK_SUCCESS)
        return -1;

    return fd;
}

std::optional<SkiaGpuContextOwner::ExportedSkSurfaceInfo>
SkiaGpuContextOwner::ExportSkSurface(const sk_sp<SkSurface>& surface)
{
    if (!surface || !device_support_memory_sharing_ || !direct_context_)
        return {};

    if (surface->recordingContext() != direct_context_.get())
        return {};

    SkImageInfo sk_image_info = surface->imageInfo();

    GrBackendRenderTarget RT = SkSurfaces::GetBackendRenderTarget(
            surface.get(),
            SkSurface::BackendHandleAccess::kFlushRead);
    if (!RT.isValid())
        return {};

    GrVkImageInfo vk_image_info;
    if (!GrBackendRenderTargets::GetVkImageInfo(RT, &vk_image_info))
        return {};

    int32_t fd = ExportDeviceMemoryFd(vk_image_info.fAlloc.fMemory);
    if (fd < 0)
        return {};

    VmaAllocationInfo alloc_info;
    vmaGetAllocationInfo(vk_allocator_->GetAllocator(),
                         reinterpret_cast<VmaAllocation>(vk_image_info.fAlloc.fBackendMemory),
                         &alloc_info);
    
    ExportedSkSurfaceInfo exported_info{};
    exported_info.fd = fd;
    exported_info.memory_type_index = alloc_info.memoryType;
    exported_info.size = alloc_info.size;
    exported_info.offset = alloc_info.offset;
    exported_info.width = RT.width();
    exported_info.height = RT.height();
    exported_info.image_tilling = vk_image_info.fImageTiling;
    exported_info.image_format = vk_image_info.fFormat;
    exported_info.sample_count = vk_image_info.fSampleCount;
    exported_info.level_count = vk_image_info.fLevelCount;
    exported_info.sk_color_type = sk_image_info.colorType();

    return exported_info;
}

namespace {

struct ImportedSurfaceReleaseContext
{
    VkDevice device;
    VkImage image;
    VkDeviceMemory device_memory;

    static void Callback(void *data)
    {
        auto *closure = reinterpret_cast<ImportedSurfaceReleaseContext*>(data);
        vkDestroyImage(closure->device, closure->image, nullptr);
        vkFreeMemory(closure->device, closure->device_memory, nullptr);
        delete closure;
    }
};

} // namespace anonymous

sk_sp<SkSurface>
SkiaGpuContextOwner::ImportSkSurface(const ExportedSkSurfaceInfo& exported_info)
{
    static std::unordered_map<uint32_t, VkSampleCountFlagBits> vk_sample_count_map = {
            { 1, VK_SAMPLE_COUNT_1_BIT },
            { 2, VK_SAMPLE_COUNT_2_BIT },
            { 4, VK_SAMPLE_COUNT_4_BIT },
            { 8, VK_SAMPLE_COUNT_8_BIT },
            { 16, VK_SAMPLE_COUNT_16_BIT },
            { 32, VK_SAMPLE_COUNT_32_BIT },
            { 64, VK_SAMPLE_COUNT_64_BIT }
    };
    
    if (vk_sample_count_map.count(exported_info.sample_count) == 0)
        return nullptr;
    VkSampleCountFlagBits vk_samples = vk_sample_count_map[exported_info.sample_count];

    VkDeviceMemory device_memory = ImportDeviceMemoryFromFd(
            exported_info.fd, exported_info.memory_type_index, exported_info.size);
    if (device_memory == VK_NULL_HANDLE)
        return nullptr;

    bool is_linear = exported_info.image_tilling == VK_IMAGE_TILING_LINEAR;
    VkImageLayout initial_layout = is_linear ? VK_IMAGE_LAYOUT_PREINITIALIZED
                                             : VK_IMAGE_LAYOUT_UNDEFINED;

    VkDevice device = GetVkDevice();

    VkImageCreateInfo image_create_info{};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.flags = 0;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = exported_info.image_format;
    image_create_info.extent = { exported_info.width, exported_info.height, 1 };
    image_create_info.mipLevels = exported_info.level_count;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = vk_samples;
    image_create_info.tiling = exported_info.image_tilling;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
                              | VK_IMAGE_USAGE_TRANSFER_SRC_BIT
                              | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.queueFamilyIndexCount = 0;
    image_create_info.pQueueFamilyIndices = nullptr;
    image_create_info.initialLayout = initial_layout;

    VkExternalMemoryImageCreateInfo external_mem_info{};
    external_mem_info.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
    external_mem_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

    image_create_info.pNext = &external_mem_info;

    VkImage vk_image;
    VkResult result = vkCreateImage(device, &image_create_info, nullptr, &vk_image);
    if (result != VK_SUCCESS)
    {
        vkFreeMemory(device, device_memory, nullptr);
        return nullptr;
    }

    result = vkBindImageMemory(device, vk_image, device_memory, exported_info.offset);
    if (result != VK_SUCCESS)
    {
        vkFreeMemory(device, device_memory, nullptr);
        return nullptr;
    }

    GrVkImageInfo vk_image_info{};
    vk_image_info.fImage = vk_image;
    vk_image_info.fImageTiling = exported_info.image_tilling;
    vk_image_info.fImageLayout = initial_layout;
    vk_image_info.fFormat = exported_info.image_format;
    vk_image_info.fImageUsageFlags = image_create_info.usage;
    vk_image_info.fSampleCount = exported_info.sample_count;
    vk_image_info.fLevelCount = exported_info.level_count;
    vk_image_info.fCurrentQueueFamily = VK_QUEUE_FAMILY_EXTERNAL;
    vk_image_info.fProtected = skgpu::Protected::kNo;
    vk_image_info.fSharingMode = VK_SHARING_MODE_EXCLUSIVE;

    GrBackendRenderTarget RT = GrBackendRenderTargets::MakeVk(
            static_cast<int>(exported_info.width), static_cast<int>(exported_info.height), vk_image_info);
    sk_sp<SkSurface> surface = SkSurfaces::WrapBackendRenderTarget(
            direct_context_.get(),
            RT,
            GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin,
            exported_info.sk_color_type,
            nullptr,
            nullptr,
            ImportedSurfaceReleaseContext::Callback,
            new ImportedSurfaceReleaseContext{ device, vk_image, device_memory }
    );

    if (!surface)
    {
        vkDestroyImage(device, vk_image, nullptr);
        vkFreeMemory(device, device_memory, nullptr);
        return nullptr;
    }

    return surface;
}

void SkiaGpuContextOwner::Trace(Tracer *tracer) noexcept
{
    if (!direct_context_)
        return;

    tracer->TraceResource("GrDirectContext",
                          TRACKABLE_TYPE_CLASS_OBJECT,
                          TRACKABLE_DEVICE_GPU,
                          TRACKABLE_OWNERSHIP_SHARED,
                          TraceIdFromPointer(direct_context_.get()));
}

GLAMOR_NAMESPACE_END
