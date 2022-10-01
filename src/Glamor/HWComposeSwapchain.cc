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

#include <optional>

#include <vulkan/vulkan.h>

#include "include/core/SkSurface.h"
#include "include/core/SkColorSpace.h"
#include "include/gpu/vk/GrVkExtensions.h"
#include "include/gpu/vk/GrVkBackendContext.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrBackendSemaphore.h"

#include "Core/Errors.h"
#include "Core/Journal.h"
#include "Glamor/HWComposeContext.h"
#include "Glamor/HWComposeSwapchain.h"
GLAMOR_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Glamor.HWComposeSwapchain)

namespace {

template<typename T>
std::vector<T> vk_typed_enumerate(const std::function<void(uint32_t*, T*)>& enumerator)
{
    uint32_t count;
    enumerator(&count, nullptr);
    if (count == 0)
        return {};
    std::vector<T> elements(count);
    enumerator(&count, elements.data());
    return std::move(elements);
}

PFN_vkVoidFunction vk_skia_proc_getter(const char *sym, VkInstance instance, VkDevice device)
{
    if (device != VK_NULL_HANDLE)
        return vkGetDeviceProcAddr(device, sym);
    return vkGetInstanceProcAddr(instance, sym);
}

#define GPU_NO_QUEUE        (-1)

struct GpuQueueFamilyIndices
{
    int32_t graphics {GPU_NO_QUEUE};
    int32_t present  {GPU_NO_QUEUE};
};

GpuQueueFamilyIndices find_gpu_queue_families(const Shared<HWComposeContext>& ctx, VkSurfaceKHR surface)
{
    auto queues = vk_typed_enumerate<VkQueueFamilyProperties>([ctx](auto *c, auto *out) {
        vkGetPhysicalDeviceQueueFamilyProperties(ctx->GetVkPhysicalDevice(), c, out);
    });

    GpuQueueFamilyIndices result;
    for (int32_t i = 0; i < queues.size(); i++)
    {
        if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            result.graphics = i;
        VkBool32 hasPresent = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(ctx->GetVkPhysicalDevice(), i, surface, &hasPresent);
        if (hasPresent)
            result.present = i;

        if (result.present > 0 && result.graphics > 0)
            break;
    }

    return result;
}

VkDevice create_gpu_logical_device(const Shared<HWComposeContext>& ctx, VkSurfaceKHR surface,
                                   int32_t *graphicsQueueIdx, int32_t *presentQueueIndex)
{
    *graphicsQueueIdx = GPU_NO_QUEUE;
    *presentQueueIndex = GPU_NO_QUEUE;

    GpuQueueFamilyIndices indices = find_gpu_queue_families(ctx, surface);

    if (indices.graphics < 0 || indices.present < 0)
    {
        QLOG(LOG_ERROR, "Current physical device could not provide graphics/present queues");
        return VK_NULL_HANDLE;
    }

    float priority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfos[2] = {
        {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .queueFamilyIndex = static_cast<uint32_t>(indices.graphics),
            .queueCount = 1,
            .pQueuePriorities = &priority
        },
        {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .queueFamilyIndex = static_cast<uint32_t>(indices.present),
            .queueCount = 1,
            .pQueuePriorities = &priority
        }
    };

    std::vector<const char*> deviceExtensions(ctx->GetDeviceEnabledExtensions().size());
    for (int32_t i = 0; i < deviceExtensions.size(); i++)
        deviceExtensions[i] = ctx->GetDeviceEnabledExtensions()[i].c_str();

    VkPhysicalDeviceFeatures features{};

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceCreateInfo.pNext = nullptr;
    deviceCreateInfo.queueCreateInfoCount = sizeof(queueCreateInfos) / sizeof(VkDeviceQueueCreateInfo);
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos;
    deviceCreateInfo.pEnabledFeatures = &features;
    deviceCreateInfo.enabledExtensionCount = ctx->GetDeviceEnabledExtensions().size();
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
    // TODO: support validation layer on logical device
    deviceCreateInfo.enabledLayerCount = 0;
    deviceCreateInfo.ppEnabledLayerNames = nullptr;

    VkDevice result;
    if (vkCreateDevice(ctx->GetVkPhysicalDevice(), &deviceCreateInfo, nullptr, &result) != VK_SUCCESS)
    {
        QLOG(LOG_ERROR, "Failed to create a logical HWCompose device");
        return VK_NULL_HANDLE;
    }

    *graphicsQueueIdx = indices.graphics;
    *presentQueueIndex = indices.present;
    return result;
}

using SwapchainDetails = HWComposeSwapchain::SwapchainDetails;

void get_swapchain_details(VkPhysicalDevice phy, VkSurfaceKHR surface, SwapchainDetails& details)
{
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phy, surface, &details.caps);
    details.formats = vk_typed_enumerate<VkSurfaceFormatKHR>([phy, surface](auto *c, auto *d) {
        return vkGetPhysicalDeviceSurfaceFormatsKHR(phy, surface, c, d);
    });
    details.present_modes = vk_typed_enumerate<VkPresentModeKHR>([phy, surface](auto *c, auto *d) {
        return vkGetPhysicalDeviceSurfacePresentModesKHR(phy, surface, c, d);
    });
}

std::optional<VkFormat> select_appropriate_format(const SwapchainDetails& details)
{
    for (const VkSurfaceFormatKHR& format : details.formats)
    {
        if (format.colorSpace != VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            continue;

        if (format.format == VK_FORMAT_B8G8R8A8_UNORM)
            return format.format;
    }
    return {};
}

std::optional<VkPresentModeKHR> select_appropriate_present_mode(const SwapchainDetails& details)
{
    bool hasFifoMode = false;
    bool hasMailboxMode = false;
    for (const VkPresentModeKHR& mode : details.present_modes)
    {
        if (mode == VK_PRESENT_MODE_FIFO_KHR)
            hasFifoMode = true;
        else if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
            hasMailboxMode = true;
    }

    // Both of these two present modes have vertical-synchronization support
    if (hasMailboxMode)
        return VK_PRESENT_MODE_MAILBOX_KHR;
    if (hasFifoMode)
        return VK_PRESENT_MODE_FIFO_KHR;
    return {};
}

bool check_dimensions_from_capabilities(const SwapchainDetails& details, int32_t w, int32_t h)
{
    const VkExtent2D& l = details.caps.minImageExtent;
    const VkExtent2D& r = details.caps.maxImageExtent;
    return (w >= l.width && h >= l.width && w <= r.width && h <= r.height);
}

bool check_surface_capabilities(const SwapchainDetails& details)
{
    const auto& caps = details.caps;
    if (!(caps.supportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT))
    {
        QLOG(LOG_ERROR, "Vulkan surface does not support IMAGE_COLOR_ATTACHMENT usage");
        return false;
    }

    if (!(caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT))
    {
        QLOG(LOG_ERROR, "Vulkan surface does not support IMAGE_TRANSFER_SRC usage");
        return false;
    }

    if (!(caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT))
    {
        QLOG(LOG_ERROR, "Vulkan surface does not support IMAGE_TRANSFER_DST usage");
        return false;
    }

    return true;
}

void populate_gr_vk_extensions(GrVkExtensions& ext, const Shared<HWComposeContext>& ctx)
{
    std::vector<const char *> instanceExt, deviceExt;
    for (const auto& str : ctx->GetInstanceEnabledExtensions())
        instanceExt.push_back(str.c_str());
    for (const auto& str : ctx->GetDeviceEnabledExtensions())
        deviceExt.push_back(str.c_str());
    ext.init(vk_skia_proc_getter, ctx->GetVkInstance(), ctx->GetVkPhysicalDevice(),
             instanceExt.size(), instanceExt.data(), deviceExt.size(), deviceExt.data());
}

} // namespace anonymous

Shared<HWComposeSwapchain> HWComposeSwapchain::Make(const Shared<HWComposeContext>& context,
                                                    VkSurfaceKHR surface,
                                                    int32_t width, int32_t height)
{
    auto ret = std::make_shared<HWComposeSwapchain>(context, surface);
    ret->vk_surface_ = surface;

    /* Select an appropriate color format and present mode, check image dimensions */
    get_swapchain_details(context->GetVkPhysicalDevice(), surface, ret->details_);
    if (!check_surface_capabilities(ret->details_))
        return nullptr;

    if (auto maybe = select_appropriate_format(ret->details_))
        ret->vk_image_format_ = *maybe;
    else
    {
        QLOG(LOG_ERROR, "Could not find an appropriate color format supported by Vulkan surface");
        return nullptr;
    }

    if (auto maybe = select_appropriate_present_mode(ret->details_))
        ret->vk_present_mode_ = *maybe;
    else
    {
        QLOG(LOG_ERROR, "Could not find an appropriate present mode supported by Vulkan surface");
        return nullptr;
    }

    if (!check_dimensions_from_capabilities(ret->details_, width, height))
    {
        QLOG(LOG_ERROR, "Invalid surface dimensions");
        return nullptr;
    }
    ret->vk_images_count_ = ret->details_.caps.minImageCount + 1;
    if (ret->details_.caps.maxImageCount > 0 &&
        ret->vk_images_count_ > ret->details_.caps.maxImageCount)
        ret->vk_images_count_ = ret->details_.caps.maxImageCount;

    /* Create logical device */
    ret->vk_device_ = create_gpu_logical_device(context, surface, &ret->graphics_queue_index_,
                                                &ret->present_queue_index_);
    if (ret->vk_device_ == VK_NULL_HANDLE || ret->graphics_queue_index_ < 0)
        return nullptr;
    vkGetDeviceQueue(ret->vk_device_, ret->graphics_queue_index_, 0, &ret->vk_graphics_queue_);
    vkGetDeviceQueue(ret->vk_device_, ret->present_queue_index_, 0, &ret->vk_present_queue_);

    GrVkExtensions extensions;
    populate_gr_vk_extensions(extensions, context);

    VkPhysicalDeviceFeatures2 features{};
    features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    vkGetPhysicalDeviceFeatures2(context->GetVkPhysicalDevice(), &features);

    GrVkBackendContext backend{};
    backend.fInstance = context->GetVkInstance();
    backend.fPhysicalDevice = context->GetVkPhysicalDevice();
    backend.fDevice = ret->vk_device_;
    backend.fQueue = ret->vk_graphics_queue_;
    backend.fMaxAPIVersion = VK_API_VERSION_1_2;
    backend.fVkExtensions = &extensions;
    backend.fGetProc = vk_skia_proc_getter;
    backend.fDeviceFeatures2 = &features;

    ret->skia_direct_context_ = GrDirectContext::MakeVulkan(backend);
    if (!ret->skia_direct_context_)
    {
        QLOG(LOG_ERROR, "Failed to create Skia GPU direct context with Vulkan backend");
        return nullptr;
    }

    // Allows resource cache <= 1GB
    ret->skia_direct_context_->setResourceCacheLimit(1024 * 1024UL);

    /* Create swapchain */
    if (!ret->CreateOrRecreateSwapchain(width, height))
        return nullptr;
    if (!ret->CreateGpuBuffers())
        return nullptr;
    return ret;
}

HWComposeSwapchain::HWComposeSwapchain(Shared<HWComposeContext> ctx, VkSurfaceKHR surface)
    : context_(std::move(ctx))
    , vk_surface_(surface)
    , details_{}
    , vk_present_mode_(VK_PRESENT_MODE_MAX_ENUM_KHR)
    , vk_device_(VK_NULL_HANDLE)
    , vk_graphics_queue_(VK_NULL_HANDLE)
    , vk_present_queue_(VK_NULL_HANDLE)
    , graphics_queue_index_(GPU_NO_QUEUE)
    , present_queue_index_(GPU_NO_QUEUE)
    , vk_swapchain_(VK_NULL_HANDLE)
    , vk_images_count_(0)
    , vk_image_format_(VK_FORMAT_UNDEFINED)
    , vk_swapchain_extent_{0, 0}
    , vk_images_sharing_mode_(VK_SHARING_MODE_MAX_ENUM)
    , skia_direct_context_(nullptr)
    , current_buffer_idx_(0)
{
}

HWComposeSwapchain::~HWComposeSwapchain()
{
    ReleaseEntireSwapchain();

    CHECK(skia_direct_context_->unique());
    skia_direct_context_.reset();

    if (vk_surface_ != VK_NULL_HANDLE)
        vkDestroySurfaceKHR(context_->GetVkInstance(), vk_surface_, nullptr);
    if (vk_device_ != VK_NULL_HANDLE)
        vkDestroyDevice(vk_device_, nullptr);
}

bool HWComposeSwapchain::CreateOrRecreateSwapchain(int32_t width, int32_t height)
{
    if (!check_dimensions_from_capabilities(details_, width, height))
    {
        QLOG(LOG_ERROR, "Invalid surface dimensions");
        return false;
    }

    vk_swapchain_extent_.width = static_cast<uint32_t>(width);
    vk_swapchain_extent_.height = static_cast<uint32_t>(height);

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = vk_surface_;
    createInfo.minImageCount = vk_images_count_;
    createInfo.imageFormat = vk_image_format_;
    createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    createInfo.imageExtent = vk_swapchain_extent_;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
                            | VK_IMAGE_USAGE_TRANSFER_SRC_BIT
                            | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    uint32_t queueFamilyIndices[] = {
        static_cast<uint32_t>(graphics_queue_index_),
        static_cast<uint32_t>(present_queue_index_)
    };

    if (graphics_queue_index_ != present_queue_index_)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 1;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    vk_images_sharing_mode_ = createInfo.imageSharingMode;

    createInfo.preTransform = details_.caps.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = vk_present_mode_;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = vk_swapchain_;

    if (vkCreateSwapchainKHR(vk_device_, &createInfo, nullptr, &vk_swapchain_) != VK_SUCCESS)
    {
        QLOG(LOG_ERROR, "Failed to create Vulkan swapchain");
        return false;
    }
    return true;
}

HWComposeSwapchain::GpuBufferInfo::GpuBufferInfo()
    : device(VK_NULL_HANDLE)
    , buffer_index(-1)
    , semaphore(VK_NULL_HANDLE)
    , acquired(false)
{
}

HWComposeSwapchain::GpuBufferInfo::~GpuBufferInfo()
{
    if (semaphore != VK_NULL_HANDLE)
        vkDestroySemaphore(device, semaphore, nullptr);
}

bool HWComposeSwapchain::CreateGpuBuffers()
{
    vkGetSwapchainImagesKHR(vk_device_, vk_swapchain_, &vk_images_count_, nullptr);
    if (vk_images_count_ == 0)
    {
        QLOG(LOG_ERROR, "No available images in swapchain");
        return false;
    }
    std::vector<VkImage> images(vk_images_count_);
    vkGetSwapchainImagesKHR(vk_device_, vk_swapchain_, &vk_images_count_, images.data());

    skia_surfaces_.clear();
    gpu_buffers_.clear();
    gpu_buffers_.resize(vk_images_count_ + 1);

    VkSemaphoreCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO;
    for (int32_t i = 0; i < vk_images_count_ + 1; i++)
    {
        VkResult result = vkCreateSemaphore(vk_device_, &info, nullptr, &gpu_buffers_[i].semaphore);
        if (result != VK_SUCCESS)
        {
            QLOG(LOG_ERROR, "Failed to create semaphore for GPU buffers");
            gpu_buffers_.clear();
            return false;
        }
        gpu_buffers_[i].device = vk_device_;
    }

    skia_surfaces_.resize(vk_images_count_);
    for (int32_t i = 0; i < vk_images_count_; i++)
    {
        GrVkImageInfo imageInfo{};
        imageInfo.fImage = images[i];
        imageInfo.fImageLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        imageInfo.fFormat = vk_image_format_;
        imageInfo.fImageUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
                                   | VK_IMAGE_USAGE_TRANSFER_SRC_BIT
                                   | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        imageInfo.fLevelCount = 1;
        imageInfo.fSharingMode = vk_images_sharing_mode_;

        GrBackendRenderTarget target(static_cast<int32_t>(vk_swapchain_extent_.width),
                                     static_cast<int32_t>(vk_swapchain_extent_.height), imageInfo);
        skia_surfaces_[i] = SkSurface::MakeFromBackendRenderTarget(skia_direct_context_.get(),
                                                                   target,
                                                                   GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin,
                                                                   SkColorType::kBGRA_8888_SkColorType,
                                                                   SkColorSpace::MakeSRGB(),
                                                                   nullptr);

        if (!skia_surfaces_[i])
        {
            QLOG(LOG_ERROR, "Failed to create Skia GPU surfaces from render target");
            skia_surfaces_.clear();
            gpu_buffers_.clear();
            return false;
        }
    }

    current_buffer_idx_ = 0;
    return true;
}

void HWComposeSwapchain::ReleaseEntireSwapchain()
{
    for (const sk_sp<SkSurface>& surface : skia_surfaces_)
        CHECK(surface->unique());

    skia_surfaces_.clear();
    gpu_buffers_.clear();
    if (vk_swapchain_)
    {
        vkDestroySwapchainKHR(vk_device_, vk_swapchain_, nullptr);
        vk_swapchain_ = VK_NULL_HANDLE;
    }
}

bool HWComposeSwapchain::Resize(int32_t width, int32_t height)
{
    if (!vk_swapchain_)
        return false;

    vkDeviceWaitIdle(vk_device_);
    skia_surfaces_.clear();
    gpu_buffers_.clear();

    if (!CreateOrRecreateSwapchain(width, height))
        return false;
    if (!CreateGpuBuffers())
        return false;
    return true;
}

SkSurface *HWComposeSwapchain::NextFrame()
{
    GpuBufferInfo& buffer = gpu_buffers_[current_buffer_idx_];
    sk_sp<SkSurface> surface;

    if (!buffer.acquired)
    {
        VkSemaphoreCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO;
        VkSemaphore semaphore;
        if (vkCreateSemaphore(vk_device_, &createInfo, nullptr, &semaphore) != VK_SUCCESS)
        {
            QLOG(LOG_ERROR, "Failed to create a semaphore to wait for next frame");
            return nullptr;
        }

        uint32_t imageIndex;
        if (vkAcquireNextImageKHR(vk_device_, vk_swapchain_, UINT64_MAX, semaphore,
                                  VK_NULL_HANDLE, &imageIndex) != VK_SUCCESS)
        {
            vkDestroySemaphore(vk_device_, semaphore, nullptr);
            QLOG(LOG_ERROR, "Failed to acquire the next image for the next frame");
            return nullptr;
        }

        buffer.buffer_index = static_cast<int32_t>(imageIndex);
        surface = skia_surfaces_[buffer.buffer_index];

        GrBackendSemaphore backendSemaphore;
        backendSemaphore.initVulkan(semaphore);
        surface->wait(1, &backendSemaphore);
        buffer.acquired = true;
    }
    else
    {
        surface = skia_surfaces_[current_buffer_idx_];
    }
    return surface.get();
}

void HWComposeSwapchain::SubmitFrame()
{
    GpuBufferInfo& buffer = gpu_buffers_[current_buffer_idx_];
    if (!buffer.acquired)
    {
        QLOG(LOG_WARNING, "Submitting a frame which is not acquired");
        return;
    }

    sk_sp<SkSurface> surface = skia_surfaces_[buffer.buffer_index];

    GrBackendSemaphore finishSemaphore;
    finishSemaphore.initVulkan(buffer.semaphore);

    GrFlushInfo flushInfo{
        .fNumSemaphores = 1,
        .fSignalSemaphores = &finishSemaphore
    };

    GrBackendSurfaceMutableState state(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, graphics_queue_index_);

    if (surface->flush(flushInfo, &state) != GrSemaphoresSubmitted::kYes)
    {
        QLOG(LOG_WARNING, "Failed in submitting current frame");
        return;
    }
    skia_direct_context_->submit();

    VkPresentInfoKHR presentInfo{};
    uint32_t imageIndex = buffer.buffer_index;
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &buffer.semaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &vk_swapchain_;
    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(vk_present_queue_, &presentInfo);
    buffer.acquired = false;

    current_buffer_idx_ = (current_buffer_idx_ + 1) % gpu_buffers_.size();
}

SkColorType HWComposeSwapchain::GetImageFormat() const
{
    CHECK(!skia_surfaces_.empty());
    return skia_surfaces_[0]->imageInfo().colorType();
}

SkAlphaType HWComposeSwapchain::GetImageAlphaFormat() const
{
    CHECK(!skia_surfaces_.empty());
    return skia_surfaces_[0]->imageInfo().alphaType();
}

void HWComposeSwapchain::Trace(GraphicsResourcesTrackable::Tracer *tracer) noexcept
{
    // `HWComposeContext` has been traced by `RenderClient`.

    tracer->TraceResource("Vulkan surface",
                          TRACKABLE_TYPE_HANDLE,
                          TRACKABLE_DEVICE_GPU,
                          TRACKABLE_OWNERSHIP_STRICT_OWNED,
                          TraceIdFromPointer(vk_surface_));

    tracer->TraceResource("Vulkan logical device",
                          TRACKABLE_TYPE_HANDLE,
                          TRACKABLE_DEVICE_GPU,
                          TRACKABLE_OWNERSHIP_STRICT_OWNED,
                          TraceIdFromPointer(vk_device_));

    tracer->TraceResource("Vulkan graphics queue",
                          TRACKABLE_TYPE_HANDLE,
                          TRACKABLE_DEVICE_GPU,
                          TRACKABLE_OWNERSHIP_STRICT_OWNED,
                          TraceIdFromPointer(vk_graphics_queue_));

    tracer->TraceResource("Vulkan present queue",
                          TRACKABLE_TYPE_HANDLE,
                          TRACKABLE_DEVICE_GPU,
                          TRACKABLE_OWNERSHIP_STRICT_OWNED,
                          TraceIdFromPointer(vk_present_queue_));

    tracer->TraceResource("Vulkan swapchain",
                          TRACKABLE_TYPE_HANDLE,
                          TRACKABLE_DEVICE_GPU,
                          TRACKABLE_OWNERSHIP_STRICT_OWNED,
                          TraceIdFromPointer(vk_swapchain_));

    int32_t index = 0;
    for (const auto& surface : skia_surfaces_)
    {
        tracer->TraceResource(fmt::format("SkSurface#{}", index++),
                              TRACKABLE_TYPE_REPRESENT,
                              TRACKABLE_DEVICE_GPU,
                              TRACKABLE_OWNERSHIP_STRICT_OWNED,
                              TraceIdFromPointer(surface.get()));
    }
}

GLAMOR_NAMESPACE_END
