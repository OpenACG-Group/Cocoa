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

#ifndef COCOA_UTAU_HWDEVICECONTEXT_H
#define COCOA_UTAU_HWDEVICECONTEXT_H

#define FFWRAP_AVUTIL_USE_HWCONTEXT_VULKAN

#include <vulkan/vulkan_core.h>

#include "Glamor/HWComposeContext.h"
#include "Glamor/HWComposeDevice.h"
#include "Utau/Utau.h"
#include "Utau/ffwrappers/libavutil.h"
UTAU_NAMESPACE_BEGIN

class HWDeviceContext
{
public:
    class AttachedResource
    {
    public:
        enum Key
        {
            kVideoFrameConversion
        };

        virtual ~AttachedResource() = default;
    };

    static void AppendRequiredVkExtensions(gl::HWComposeContext *context,
                                           std::vector<std::string>& extra_device_ext);
    static void AppendRequiredQueueSpecs(std::vector<gl::HWComposeDevice::DeviceQueueSpecifier>& specs);
    static void EnableRequiredVkDeviceFeatures(const gl::HWComposeContext::DeviceFeatures& avail_features,
                                               VkPhysicalDeviceFeatures2& features,
                                               VkPhysicalDeviceVulkan13Features& v13feature,
                                               VkPhysicalDeviceSamplerYcbcrConversionFeatures& ycbcr_conv_feature);

    static HWDeviceContext *GetEmbedded(AVBufferRef *hwctx);
    static HWDeviceContext *GetEmbedded(AVHWDeviceContext *hwctx);

    explicit HWDeviceContext(std::shared_ptr<gl::HWComposeDevice> device, AVBufferRef *hwctx)
        : vk_device(std::move(device)), hwctx_(hwctx) {}
    ~HWDeviceContext() = default;

    g_nodiscard static AVBufferRef *MakeVulkan();
    g_nodiscard static AVBufferRef *MakeFromCompatibleGLDevice(
        const std::shared_ptr<gl::HWComposeDevice>& device,
        const VkPhysicalDeviceFeatures2& enabled_features);

    g_nodiscard AVBufferRef *GetAVContext() const;
    g_nodiscard AVPixelFormat GetDeviceFormat() const;

    bool FillHWFramesContextBackendSpecific(AVHWFramesContext *frames_ctx);

    void StoreAttachedResource(AttachedResource::Key key, std::unique_ptr<AttachedResource> resource);
    AttachedResource *LoadAttachedResource(AttachedResource::Key key) const;
    void DeleteAttachedResource(AttachedResource::Key key);

private:
    using AttachedResourceMap = std::map<AttachedResource::Key, std::unique_ptr<AttachedResource>>;

    std::shared_ptr<gl::HWComposeDevice> vk_device;
    AVBufferRef *hwctx_;
    AttachedResourceMap attached_resource_map_;
};

UTAU_NAMESPACE_END
#endif //COCOA_UTAU_HWDEVICECONTEXT_H
