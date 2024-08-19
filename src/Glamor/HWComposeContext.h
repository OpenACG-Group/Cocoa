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

#ifndef COCOA_GLAMOR_HWCOMPOSECONTEXT_H
#define COCOA_GLAMOR_HWCOMPOSECONTEXT_H

#include <algorithm>

#include <vulkan/vulkan.h>

#include "Core/EnumClassBitfield.h"
#include "Glamor/GraphicsResourcesTrackable.h"
GLAMOR_NAMESPACE_BEGIN

class HWComposeSwapchain;

/**
 * `HWComposeContext` is a wrapper of a Vulkan instance and a physical
 * device (GPU or other supported devices). It represents an instance
 * of HWCompose, and multiple `HWComposeDevice` could be created from
 * a `HWComposeContext`.
 */
class HWComposeContext : public GraphicsResourcesTrackable
{
public:
    struct Options
    {
        enum VkDBGLevelFilter
        {
            kVerbose    = (1 << 1),
            kInfo       = (1 << 2),
            kWarning    = (1 << 3),
            kError      = (1 << 4)
        };

        enum VkDBGTypeFilter
        {
            kGeneral        = (1 << 1),
            kPerformance    = (1 << 2),
            kValidation     = (1 << 3)
        };

        bool                        use_vkdbg{false};
        Bitfield<VkDBGLevelFilter>  vkdbg_level_filter;
        Bitfield<VkDBGTypeFilter>   vkdbg_type_filter;
        std::string                 application_name;
        int32_t                     application_version_major;
        int32_t                     application_version_minor;
        int32_t                     application_version_patch;
        std::vector<std::string>    instance_extensions;
        std::vector<std::string>    device_extensions;
        // Physical device that supports the more optional extensions has higher priority
        // for physical device selection.
        std::vector<std::string>    optional_device_extensions;
        std::string                 device_name_hint;
    };

    struct DeviceFeatures
    {
        VkPhysicalDeviceFeatures base;
        VkPhysicalDeviceVulkan11Features v11;
        VkPhysicalDeviceVulkan12Features v12;
        VkPhysicalDeviceVulkan13Features v13;
        VkPhysicalDeviceSamplerYcbcrConversionFeatures ycbcr_conversion;
    };

    static std::shared_ptr<HWComposeContext> MakeVulkan(const Options& options);

    HWComposeContext();
    ~HWComposeContext() override;

    g_nodiscard VkInstance GetVkInstance() const {
        return vk_instance_;
    }

    g_nodiscard VkPhysicalDevice GetVkPhysicalDevice() const {
        return vk_physical_device_;
    }

    g_nodiscard const VkPhysicalDeviceProperties& GetVkPhysicalDeviceProperties() const {
        return vk_physical_device_properties_;
    }

    g_nodiscard const DeviceFeatures& GetVkPhysicalDeviceFeatures() const {
        return vk_physical_device_features_;
    }

    g_nodiscard const std::vector<std::string>& GetDeviceExtensions() const {
        return device_extensions_;
    }

    g_nodiscard const std::vector<std::string>& GetInstanceEnabledExtensions() const {
        return instance_enabled_extensions_;
    }

    g_nodiscard bool HasDeviceExtension(const std::string& name) const
    {
        return std::find(device_extensions_.begin(), device_extensions_.end(), name)
            != device_extensions_.end();
    }

    g_nodiscard bool HasInstanceExtension(const std::string& name) const {
        return std::find(instance_enabled_extensions_.begin(), instance_enabled_extensions_.end(), name)
            != instance_enabled_extensions_.end();
    }

    void Trace(GraphicsResourcesTrackable::Tracer *tracer) noexcept override;

private:
    VkInstance                      vk_instance_;
    VkDebugUtilsMessengerEXT        vk_debug_messenger_;
    VkPhysicalDevice                vk_physical_device_;
    VkPhysicalDeviceProperties      vk_physical_device_properties_;
    DeviceFeatures                  vk_physical_device_features_;
    std::vector<std::string>        device_extensions_;
    std::vector<std::string>        instance_enabled_extensions_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_HWCOMPOSECONTEXT_H
