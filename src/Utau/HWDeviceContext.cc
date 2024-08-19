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

#include <unistd.h>
#include <fcntl.h>

#include "Core/Journal.h"
#include "Core/Exception.h"
#include "Core/Errors.h"
#include "Utau/HWDeviceContext.h"
#include "Glamor/Glamor.h"
#include "Glamor/HWComposeContext.h"
#include "Glamor/HWComposeDevice.h"

#define FFWRAP_AVUTIL_USE_HWCONTEXT_VULKAN
#include "Utau/ffwrappers/libavutil.h"
#include "Utau/ffwrappers/libavcodec.h"
UTAU_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Utau.HWDeviceContext)

void HWDeviceContext::AppendRequiredQueueSpecs(std::vector<gl::HWComposeDevice::DeviceQueueSpecifier>& specs)
{
    using Selector = gl::HWComposeDevice::DeviceQueueSelector;
    specs.push_back({ Selector::kCompute, -1, {}, VK_NULL_HANDLE, true });
    specs.push_back({ Selector::kVideoDecode, -1, {}, VK_NULL_HANDLE, true });
}

void HWDeviceContext::AppendRequiredVkExtensions(gl::HWComposeContext *context,
                                                 std::vector<std::string>& extra_device_ext)
{
    extra_device_ext.emplace_back(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME);
    extra_device_ext.emplace_back(VK_KHR_VIDEO_QUEUE_EXTENSION_NAME);
    extra_device_ext.emplace_back(VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME);

    // These are optional extensions deciding what decoders we can use.
    for (const std::string& name : {VK_KHR_VIDEO_DECODE_H264_EXTENSION_NAME,
                                  VK_KHR_VIDEO_DECODE_H265_EXTENSION_NAME,
                                  VK_KHR_VIDEO_DECODE_AV1_EXTENSION_NAME,
                                  VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME})
    {
        if (context->HasDeviceExtension(name))
            extra_device_ext.emplace_back(name);
    }
}

void HWDeviceContext::EnableRequiredVkDeviceFeatures(const gl::HWComposeContext::DeviceFeatures& avail_features,
                                                     VkPhysicalDeviceFeatures2& features,
                                                     VkPhysicalDeviceVulkan13Features& v13feature,
                                                     VkPhysicalDeviceSamplerYcbcrConversionFeatures& ycbcr_conv_feature)
{
    features.features.shaderImageGatherExtended = avail_features.base.shaderImageGatherExtended;
    features.features.fragmentStoresAndAtomics = avail_features.base.fragmentStoresAndAtomics;
    features.features.shaderInt64 = avail_features.base.shaderInt64;
    features.features.vertexPipelineStoresAndAtomics = avail_features.base.vertexPipelineStoresAndAtomics;
    v13feature.synchronization2 = avail_features.v13.synchronization2;
    ycbcr_conv_feature.samplerYcbcrConversion = avail_features.ycbcr_conversion.samplerYcbcrConversion;
}

AVBufferRef *HWDeviceContext::MakeVulkan()
{
    auto hwcompose_ctx = gl::GlobalScope::Instance()->GetHWComposeContext();
    if (!hwcompose_ctx)
    {
        QLOG(LOG_ERROR, "Failed to initialize hardware acceleration: HWCompose context not available");
        return nullptr;
    }

    // Enable device features
    const auto& avail_features = hwcompose_ctx->GetVkPhysicalDeviceFeatures();
    VkPhysicalDeviceFeatures2 enable_features{};
    enable_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    VkPhysicalDeviceVulkan13Features v13feature{};
    v13feature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    VkPhysicalDeviceSamplerYcbcrConversionFeatures ycbcr_conv_feature{};
    ycbcr_conv_feature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES;

    // Chain them together
    enable_features.pNext = &v13feature;
    v13feature.pNext = &ycbcr_conv_feature;

    EnableRequiredVkDeviceFeatures(avail_features, enable_features, v13feature,
                                   ycbcr_conv_feature);

    // Select device extensions and queue specs
    std::vector<std::string> extra_device_ext;
    AppendRequiredVkExtensions(hwcompose_ctx.get(), extra_device_ext);

    std::vector<gl::HWComposeDevice::DeviceQueueSpecifier> queue_specs;
    AppendRequiredQueueSpecs(queue_specs);

    std::shared_ptr<gl::HWComposeDevice> device = gl::HWComposeDevice::Make(
        hwcompose_ctx, queue_specs, extra_device_ext, enable_features);
    if (!device)
    {
        QLOG(LOG_ERROR, "Failed to initialize hardware acceleration: failed to create device");
        return nullptr;
    }
    return MakeFromCompatibleGLDevice(device, enable_features);
}

AVBufferRef *
HWDeviceContext::MakeFromCompatibleGLDevice(const std::shared_ptr<gl::HWComposeDevice>& device,
                                            const VkPhysicalDeviceFeatures2& enable_features)
{
    QLOG(LOG_INFO, "Using Vulkan for hardware acceleration");
    auto hwcompose_ctx = device->GetHWComposeContext();

    // Create ffmpeg hwdevice context
    AVBufferRef *hwctx_buffer_ref = av_hwdevice_ctx_alloc(AV_HWDEVICE_TYPE_VULKAN);
    if (!hwctx_buffer_ref)
    {
        QLOG(LOG_ERROR, "Failed in av_hwdevice_ctx_alloc, unsupported device type?");
        return nullptr;
    }

    auto *hwctx = reinterpret_cast<AVHWDeviceContext*>(hwctx_buffer_ref->data);
    auto *vk_hwctx = static_cast<AVVulkanDeviceContext*>(hwctx->hwctx);

    vk_hwctx->get_proc_addr = vkGetInstanceProcAddr;
    vk_hwctx->inst = hwcompose_ctx->GetVkInstance();
    vk_hwctx->phys_dev = hwcompose_ctx->GetVkPhysicalDevice();
    vk_hwctx->act_dev = device->GetVkDevice();

    // Fill device features
    vk_hwctx->device_features = enable_features;

    // Fill enabled extensions
    const auto& enabled_inst_ext = hwcompose_ctx->GetInstanceEnabledExtensions();
    vk_hwctx->nb_enabled_inst_extensions = static_cast<int>(enabled_inst_ext.size());
    const char **inst_exts_chararr = static_cast<const char**>(
                                     av_malloc(sizeof(char*) * enabled_inst_ext.size()));
    for (int32_t i = 0; i < vk_hwctx->nb_enabled_inst_extensions; i++)
        inst_exts_chararr[i] = av_strdup(enabled_inst_ext[i].c_str());
    vk_hwctx->enabled_inst_extensions = inst_exts_chararr;

    const auto& enabled_device_ext = device->GetEnabledExtensions();
    vk_hwctx->nb_enabled_dev_extensions = static_cast<int>(enabled_device_ext.size());
    const char **device_exts_chararr = static_cast<const char **>(
                                       av_malloc(sizeof(char*) * enabled_device_ext.size()));
    for (int32_t i = 0; i < vk_hwctx->nb_enabled_dev_extensions; i++)
        device_exts_chararr[i] = av_strdup(enabled_device_ext[i].c_str());
    vk_hwctx->enabled_dev_extensions = device_exts_chararr;

    // Fill queue families
    using Selector = gl::HWComposeDevice::DeviceQueueSelector;
    if (device->GetDeviceQueueCount(Selector::kGraphicsWithPresent) > 0)
    {
        vk_hwctx->queue_family_index = device->GetDeviceQueue(
            Selector::kGraphicsWithPresent, 0)->family_index;
        vk_hwctx->nb_graphics_queues = device->GetDeviceQueueCount(Selector::kGraphicsWithPresent);
    }
    else if (device->GetDeviceQueueCount(Selector::kGraphics) > 0)
    {
        vk_hwctx->queue_family_index = device->GetDeviceQueue(
            Selector::kGraphics, 0)->family_index;
        vk_hwctx->nb_graphics_queues = device->GetDeviceQueueCount(Selector::kGraphics);
    }
    else
    {
        vk_hwctx->queue_family_index = -1;
        vk_hwctx->nb_graphics_queues = 0;
    }

    vk_hwctx->queue_family_comp_index = device->GetDeviceQueue(Selector::kCompute, 0)->family_index;
    vk_hwctx->nb_comp_queues = device->GetDeviceQueueCount(Selector::kCompute);

    vk_hwctx->queue_family_tx_index = vk_hwctx->queue_family_comp_index;
    vk_hwctx->nb_tx_queues = vk_hwctx->nb_comp_queues;

    vk_hwctx->queue_family_encode_index = -1;
    vk_hwctx->nb_encode_queues = 0;

    vk_hwctx->queue_family_decode_index = device->GetDeviceQueue(Selector::kVideoDecode, 0)->family_index;
    vk_hwctx->nb_decode_queues = device->GetDeviceQueueCount(Selector::kVideoDecode);

    int32_t ret = av_hwdevice_ctx_init(hwctx_buffer_ref);
    if (ret < 0)
    {
        av_buffer_unref(&hwctx_buffer_ref);
        QLOG(LOG_ERROR, "Failed to initialize libav hardware device context");
        return nullptr;
    }

    // Attach the embedded `HWDeviceContext`. The most important function of it
    // is to keep a reference to `gl::HWComposeDevice`.
    HWDeviceContext *embedded = new HWDeviceContext(device, hwctx_buffer_ref);
    hwctx->user_opaque = embedded;
    hwctx->free = +[](AVHWDeviceContext *hwctx) {
        delete static_cast<HWDeviceContext*>(hwctx->user_opaque);
    };

    return hwctx_buffer_ref;
}

HWDeviceContext *HWDeviceContext::GetEmbedded(AVBufferRef *hwctx)
{
    void *opaque = reinterpret_cast<AVHWDeviceContext*>(hwctx->data)->user_opaque;
    return static_cast<HWDeviceContext*>(opaque);
}

HWDeviceContext *HWDeviceContext::GetEmbedded(AVHWDeviceContext *hwctx)
{
    return static_cast<HWDeviceContext*>(hwctx->user_opaque);
}

AVBufferRef *HWDeviceContext::GetAVContext() const
{
    return hwctx_;
}

AVPixelFormat HWDeviceContext::GetDeviceFormat() const
{
    return AV_PIX_FMT_VULKAN;
}

bool HWDeviceContext::FillHWFramesContextBackendSpecific(AVHWFramesContext *frames_ctx)
{
    // AVVulkanFramesContext *vkctx = static_cast<AVVulkanFramesContext*>(frames_ctx->hwctx);
    return true;
}

void HWDeviceContext::StoreAttachedResource(AttachedResource::Key key, std::unique_ptr<AttachedResource> resource)
{
    attached_resource_map_[key] = std::move(resource);
}

HWDeviceContext::AttachedResource *HWDeviceContext::LoadAttachedResource(AttachedResource::Key key) const
{
    if (attached_resource_map_.contains(key))
        return attached_resource_map_.at(key).get();
    return nullptr;
}

void HWDeviceContext::DeleteAttachedResource(AttachedResource::Key key)
{
    attached_resource_map_.erase(key);
}

UTAU_NAMESPACE_END
