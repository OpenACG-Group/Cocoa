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
#include <sstream>

#include <vulkan/vulkan.h>

#include "include/core/SkSurface.h"
#include "include/core/SkColorSpace.h"
#include "include/gpu/vk/GrVkExtensions.h"
#include "include/gpu/vk/GrVkBackendContext.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrBackendSemaphore.h"
#include "include/gpu/GrBackendSurfaceMutableState.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/GrBackendSurface.h"

#include "fmt/format.h"

#include "Core/Errors.h"
#include "Core/Journal.h"
#include "Core/TraceEvent.h"
#include "Glamor/HWComposeContext.h"
#include "Glamor/HWComposeSwapchain.h"
#include "Glamor/HWComposeDevice.h"
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

void populate_gr_vk_extensions(GrVkExtensions& ext,
                               const Shared<HWComposeContext>& ctx,
                               const Unique<HWComposeDevice>& device)
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

} // namespace anonymous

Unique<HWComposeSwapchain>
HWComposeSwapchain::Make(const Shared<HWComposeContext>& context,
                         VkSurfaceFactory& factory,
                         int32_t width, int32_t height,
                         SkPixelGeometry pixel_geometry)
{
    const ContextOptions& gl_options = GlobalScope::Ref().GetOptions();
    if (gl_options.GetDisableHWComposePresent())
    {
        QLOG(LOG_ERROR, "HWCompose presentation was disabled by global options");
        return nullptr;
    }

    auto ret = std::make_unique<HWComposeSwapchain>();
    ret->context_ = context;
    ret->pixel_geometry_ = pixel_geometry;

    // Create a present `VkSurfaceKHR` first, using the factory class
    // provided by caller.
    ret->vk_surface_ = factory.Create(context);
    if (ret->vk_surface_ == VK_NULL_HANDLE)
        return nullptr;

    // Select an appropriate color format and present mode,
    // check image dimensions.
    VkPhysicalDevice physical_device = context->GetVkPhysicalDevice();
    get_swapchain_details(physical_device, ret->vk_surface_, ret->details_);
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

    // Create HWCompose logical device
    using Selector = HWComposeDevice::DeviceQueueSelector;
    ret->device_ = HWComposeDevice::Make(context, {
        { Selector::kGraphics, 1, {1.0f}, VK_NULL_HANDLE },
        { Selector::kPresent,  1, {1.0f}, ret->vk_surface_ }
    }, {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    });
    if (!ret->device_)
        return nullptr;

    auto graphics_queue = ret->device_->GetDeviceQueue(
            HWComposeDevice::DeviceQueueSelector::kGraphics, 0);
    auto present_queue = ret->device_->GetDeviceQueue(
            HWComposeDevice::DeviceQueueSelector::kPresent, 0);
    CHECK(graphics_queue.has_value() && present_queue.has_value());
    ret->device_graphics_queue_family_ = graphics_queue->family_index;
    ret->device_present_queue_family_ = present_queue->family_index;
    ret->device_present_queue_ = present_queue->queue;

    // Create Skia GPU direct context
    GrVkExtensions extensions;
    populate_gr_vk_extensions(extensions, context, ret->device_);

    VkPhysicalDeviceFeatures2 features{};
    features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    vkGetPhysicalDeviceFeatures2(context->GetVkPhysicalDevice(), &features);

    GrVkBackendContext backend{};
    backend.fInstance = context->GetVkInstance();
    backend.fPhysicalDevice = physical_device;
    backend.fDevice = ret->device_->GetVkDevice();
    backend.fQueue = graphics_queue->queue;
    backend.fGraphicsQueueIndex = graphics_queue->family_index;
    backend.fMaxAPIVersion = VK_API_VERSION_1_2;
    backend.fVkExtensions = &extensions;
    backend.fGetProc = vk_skia_proc_getter;
    backend.fDeviceFeatures2 = &features;

    sk_sp<GrDirectContext> direct_ctx = GrDirectContext::MakeVulkan(backend);
    if (!direct_ctx)
    {
        QLOG(LOG_ERROR, "Failed to create Skia GPU direct context with Vulkan backend");
        return nullptr;
    }

    ret->TakeOverSkiaGpuContext(std::move(direct_ctx));

    /* Create swapchain */
    if (!ret->CreateOrRecreateSwapchain(width, height))
        return nullptr;
    if (!ret->CreateGpuBuffers())
        return nullptr;
    return ret;
}

HWComposeSwapchain::HWComposeSwapchain()
    : context_(nullptr)
    , device_(nullptr)
    , pixel_geometry_(SkPixelGeometry::kUnknown_SkPixelGeometry)
    , device_graphics_queue_family_(0)
    , device_present_queue_family_(0)
    , device_present_queue_(VK_NULL_HANDLE)
    , vk_surface_(VK_NULL_HANDLE)
    , details_{}
    , vk_present_mode_(VK_PRESENT_MODE_MAX_ENUM_KHR)
    , vk_swapchain_(VK_NULL_HANDLE)
    , vk_images_count_(0)
    , vk_image_format_(VK_FORMAT_UNDEFINED)
    , vk_swapchain_extent_{0, 0}
    , vk_images_sharing_mode_(VK_SHARING_MODE_MAX_ENUM)
    , current_buffer_idx_(0)
{
}

HWComposeSwapchain::~HWComposeSwapchain()
{
    ReleaseEntireSwapchain();
    DisposeSkiaGpuContext();
    if (vk_surface_ != VK_NULL_HANDLE)
        vkDestroySurfaceKHR(context_->GetVkInstance(), vk_surface_, nullptr);
    device_.reset();
}

HWComposeDevice *HWComposeSwapchain::OnGetHWComposeDevice()
{
    return device_.get();
}

bool HWComposeSwapchain::CreateOrRecreateSwapchain(int32_t width, int32_t height)
{
    if (!check_dimensions_from_capabilities(details_, width, height))
    {
        QLOG(LOG_ERROR, "Invalid surface dimensions");
        return false;
    }

    if (vk_swapchain_)
    {
        vkDestroySwapchainKHR(device_->GetVkDevice(), vk_swapchain_, nullptr);
        vk_swapchain_ = VK_NULL_HANDLE;
    }

    vk_swapchain_extent_.width = static_cast<uint32_t>(width);
    vk_swapchain_extent_.height = static_cast<uint32_t>(height);

    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = vk_surface_;
    create_info.minImageCount = vk_images_count_;
    create_info.imageFormat = vk_image_format_;
    create_info.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    create_info.imageExtent = vk_swapchain_extent_;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
                             | VK_IMAGE_USAGE_TRANSFER_SRC_BIT
                             | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    uint32_t queue_family_indices[] = {
        device_graphics_queue_family_,
        device_present_queue_family_
    };

    if (device_graphics_queue_family_ != device_present_queue_family_)
    {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family_indices;
    }
    else
    {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 1;
        create_info.pQueueFamilyIndices = queue_family_indices;
    }
    vk_images_sharing_mode_ = create_info.imageSharingMode;

    create_info.preTransform = details_.caps.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = vk_present_mode_;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device_->GetVkDevice(), &create_info,
                             nullptr, &vk_swapchain_) != VK_SUCCESS)
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
    VkDevice vk_device = device_->GetVkDevice();

    vkGetSwapchainImagesKHR(vk_device, vk_swapchain_, &vk_images_count_, nullptr);
    if (vk_images_count_ == 0)
    {
        QLOG(LOG_ERROR, "No available images in swapchain");
        return false;
    }
    std::vector<VkImage> images(vk_images_count_);
    vkGetSwapchainImagesKHR(vk_device, vk_swapchain_, &vk_images_count_, images.data());

    skia_surfaces_.clear();
    gpu_buffers_.clear();
    gpu_buffers_.resize(vk_images_count_ + 1);

    VkSemaphoreCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO;
    for (int32_t i = 0; i < vk_images_count_ + 1; i++)
    {
        VkResult result = vkCreateSemaphore(vk_device, &info, nullptr, &gpu_buffers_[i].semaphore);
        if (result != VK_SUCCESS)
        {
            QLOG(LOG_ERROR, "Failed to create semaphore for GPU buffers");
            gpu_buffers_.clear();
            return false;
        }
        gpu_buffers_[i].device = vk_device;
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

        SkSurfaceProps surface_props(0, pixel_geometry_);
        skia_surfaces_[i] = SkSurfaces::WrapBackendRenderTarget(
                GetSkiaGpuContext(),
                target,
                GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin,
                SkColorType::kBGRA_8888_SkColorType,
                SkColorSpace::MakeSRGB(),
                &surface_props
        );

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
        vkDestroySwapchainKHR(device_->GetVkDevice(), vk_swapchain_, nullptr);
        vk_swapchain_ = VK_NULL_HANDLE;
    }
}

bool HWComposeSwapchain::Resize(int32_t width, int32_t height)
{
    if (!vk_swapchain_)
        return false;

    vkDeviceWaitIdle(device_->GetVkDevice());
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
    TRACE_EVENT("rendering", "HWComposeSwapchain::NextFrame");

    GpuBufferInfo& buffer = gpu_buffers_[current_buffer_idx_];
    sk_sp<SkSurface> surface;
    VkDevice vk_device = device_->GetVkDevice();

    if (!buffer.acquired)
    {
        VkSemaphoreCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO;
        VkSemaphore semaphore;
        if (vkCreateSemaphore(vk_device, &create_info, nullptr, &semaphore) != VK_SUCCESS)
        {
            QLOG(LOG_ERROR, "Failed to create a semaphore to wait for next frame");
            return nullptr;
        }

        uint32_t image_index;
        if (vkAcquireNextImageKHR(vk_device, vk_swapchain_, UINT64_MAX, semaphore,
                                  VK_NULL_HANDLE, &image_index) != VK_SUCCESS)
        {
            vkDestroySemaphore(vk_device, semaphore, nullptr);
            QLOG(LOG_ERROR, "Failed to acquire the next image for the next frame");
            return nullptr;
        }

        buffer.buffer_index = static_cast<int32_t>(image_index);
        surface = skia_surfaces_[buffer.buffer_index];

        GrBackendSemaphore backend_semaphore;
        backend_semaphore.initVulkan(semaphore);
        surface->wait(1, &backend_semaphore);
        buffer.acquired = true;
    }
    else
    {
        surface = skia_surfaces_[current_buffer_idx_];
    }
    return surface.get();
}

GrSemaphoresSubmitted
HWComposeSwapchain::SubmitFrame(const std::vector<GrBackendSemaphore>& signal_semaphores)
{
    TRACE_EVENT("rendering", "HWComposeSwapchain::SubmitFrame");

    GpuBufferInfo& buffer = gpu_buffers_[current_buffer_idx_];
    if (!buffer.acquired)
    {
        QLOG(LOG_WARNING, "Submitting a frame which is not acquired");
        return GrSemaphoresSubmitted::kNo;
    }

    sk_sp<SkSurface> surface = skia_surfaces_[buffer.buffer_index];

    std::vector<GrBackendSemaphore> total_semaphores;
    total_semaphores.reserve(1 + signal_semaphores.size());
    total_semaphores.emplace_back();
    total_semaphores.back().initVulkan(buffer.semaphore);
    for (const GrBackendSemaphore& sem : signal_semaphores)
        total_semaphores.push_back(sem);

    GrFlushInfo surface_flush_info{
        .fNumSemaphores = total_semaphores.size(),
        .fSignalSemaphores = total_semaphores.data()
    };

    GrDirectContext *direct_ctx = GetSkiaGpuContext();
    GrBackendSurfaceMutableState state(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                       device_present_queue_family_);

    if (direct_ctx->flush(surface, surface_flush_info, &state) != GrSemaphoresSubmitted::kYes)
    {
        QLOG(LOG_ERROR, "Failed to flush current surface contents to GPU recording context");
        return GrSemaphoresSubmitted::kNo;
    }
    if (!direct_ctx->submit())
    {
        QLOG(LOG_ERROR, "Failed to submit drawing operations to GPU");
        return GrSemaphoresSubmitted::kNo;
    }

    return GrSemaphoresSubmitted::kYes;
}

void HWComposeSwapchain::PresentFrame()
{
    TRACE_EVENT("rendering", "HWComposeSwapchain::PresentFrame");

    GpuBufferInfo& buffer = gpu_buffers_[current_buffer_idx_];
    if (!buffer.acquired)
    {
        QLOG(LOG_WARNING, "Submitting a frame which is not acquired");
        return;
    }

    VkPresentInfoKHR present_info{};
    uint32_t image_index = buffer.buffer_index;
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &buffer.semaphore;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &vk_swapchain_;
    present_info.pImageIndices = &image_index;

    vkQueuePresentKHR(device_present_queue_, &present_info);
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

std::string HWComposeSwapchain::GetBufferStateDescriptor()
{
    if (skia_surfaces_.empty())
        return "<empty>";

    std::ostringstream oss;

    oss << fmt::format("[hwcompose_context={}:swapchain={}]",
                       fmt::ptr(context_.get()), fmt::ptr(this));

    int32_t index = 0;
    for (const auto& surface : skia_surfaces_)
    {
        if (index > 0)
            oss << '|';

        oss << fmt::format("#{}:surface={}:size={}x{}:recording_context={}:{}",
                           index,
                           fmt::ptr(surface.get()),
                           surface->width(),
                           surface->height(),
                           fmt::ptr(surface->recordingContext()),
                           current_buffer_idx_ == index ? "drawing" : "free");

        index++;
    }

    return oss.str();
}

void HWComposeSwapchain::Trace(GraphicsResourcesTrackable::Tracer *tracer) noexcept
{
    SkiaGpuContextOwner::Trace(tracer);

    tracer->TraceMember("HWComposeDevice", device_.get());

    tracer->TraceResource("VkSurfaceKHR",
                          TRACKABLE_TYPE_HANDLE,
                          TRACKABLE_DEVICE_GPU,
                          TRACKABLE_OWNERSHIP_STRICT_OWNED,
                          TraceIdFromPointer(vk_surface_));

    tracer->TraceResource("VkSwapchainKHR",
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
