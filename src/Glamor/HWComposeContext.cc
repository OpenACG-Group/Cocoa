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

#include <cstring>
#include <set>

#include "Core/Project.h"
#include "Core/Journal.h"
#include "Core/Errors.h"
#include "Core/Utils.h"
#include "Glamor/Glamor.h"
#include "Glamor/HWComposeContext.h"
#include "Glamor/HWComposeSwapchain.h"
GLAMOR_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Glamor.HWComposeContext)

namespace {

const char *g_validation_layers[] = {
    "VK_LAYER_KHRONOS_validation"
};

const char *g_device_extensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

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

bool has_validation_layer_support()
{
    auto props = vk_typed_enumerate<VkLayerProperties>(vkEnumerateInstanceLayerProperties);
    for (const char *required : g_validation_layers)
    {
        auto itr = std::find_if(props.begin(), props.end(), [required](const VkLayerProperties& p) {
            return (std::strcmp(required, p.layerName) == 0);
        });
        if (itr == props.end())
        {
            QLOG(LOG_DEBUG, "Could not find required validation layer {}", required);
            return false;
        }
        QLOG(LOG_DEBUG, "Found required validation layer {}", required);
    }
    return true;
}

std::vector<char const*> vk_select_required_instance_extensions(const HWComposeContext::Options& opt)
{
    std::vector<const char*> extensions;
    for (const std::string& str : opt.instance_extensions)
        extensions.push_back(str.c_str());
    if (opt.use_vkdbg)
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    return extensions;
}

struct DebugMessageSeverityMap {
    VkDebugUtilsMessageSeverityFlagBitsEXT severity;
    const char *prefix;
} g_debug_message_severity_map[] = {
    { VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,  "%fg<cy>(vulkan@verbose)%reset" },
    { VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT,     "%fg<gr>(vulkan@info)%reset"    },
    { VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,  "%fg<ye>(vulkan@warning)%reset" },
    { VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,    "%fg<re>(vulkan@error)%reset"   }
};

VkBool32 vk_debug_messenger_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                     g_maybe_unused VkDebugUtilsMessageTypeFlagsEXT type,
                                     const VkDebugUtilsMessengerCallbackDataEXT *data,
                                     g_maybe_unused void *pUserData)
{
    const char *prefix = "(vulkan@unknown)";
    for (auto& pair : g_debug_message_severity_map)
    {
        if (severity == pair.severity)
        {
            prefix = pair.prefix;
            break;
        }
    }

    for (const auto& view : utils::SplitString(data->pMessage, '\n'))
        QLOG(LOG_DEBUG, "{} {}", prefix, view);
    return VK_FALSE;
}

void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& info,
                                          const HWComposeContext::Options& options)
{
    using T = HWComposeContext::Options::VkDBGTypeFilter;
    using L = HWComposeContext::Options::VkDBGLevelFilter;

    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    info.messageSeverity = (options.vkdbg_level_filter & L::kVerbose) ?
                            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT : 0;
    info.messageSeverity |= (options.vkdbg_level_filter & L::kInfo) ?
                            VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT : 0;
    info.messageSeverity |= (options.vkdbg_level_filter & L::kWarning) ?
                            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT : 0;
    info.messageSeverity |= (options.vkdbg_level_filter & L::kError) ?
                            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT : 0;

    info.messageType = (options.vkdbg_type_filter & T::kGeneral) ?
                        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT : 0;
    info.messageType |= (options.vkdbg_type_filter & T::kPerformance) ?
                         VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT : 0;
    info.messageType |= (options.vkdbg_type_filter & T::kValidation) ?
                         VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT : 0;

    info.flags = 0;
    info.pNext = nullptr;
    info.pfnUserCallback = vk_debug_messenger_callback;
}

VkInstance vk_create_instance_from_options(const HWComposeContext::Options& options)
{
    QLOG(LOG_INFO, "Creating HWCompose context [Vulkan backend]");

    uint32_t version;
    vkEnumerateInstanceVersion(&version);
    QLOG(LOG_INFO, "Available Vulkan instance API version: %fg<bl>{}.{}.{}%reset",
         VK_API_VERSION_MAJOR(version), VK_API_VERSION_MINOR(version), VK_API_VERSION_PATCH(version));

    if (version < VK_API_VERSION_1_2)
    {
        QLOG(LOG_ERROR, "Unsupported Vulkan instance API version");
        return nullptr;
    }

    if (options.use_vkdbg && !has_validation_layer_support())
    {
        QLOG(LOG_ERROR, "Failed finding available validation layers");
        return VK_NULL_HANDLE;
    }

    VkApplicationInfo applicationInfo{};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = options.application_name.c_str();
    applicationInfo.applicationVersion = VK_MAKE_VERSION(options.application_version_major,
                                                         options.application_version_minor,
                                                         options.application_version_patch);
    applicationInfo.pEngineName = "org.OpenACG.Cocoa";
    applicationInfo.engineVersion = VK_MAKE_VERSION(COCOA_MAJOR,
                                                    COCOA_MINOR,
                                                    COCOA_PATCH);
    applicationInfo.apiVersion = VK_API_VERSION_1_2;
    applicationInfo.pNext = nullptr;

    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;
    instanceCreateInfo.pNext = nullptr;

    auto extensions = vk_select_required_instance_extensions(options);
    instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

    QLOG(LOG_INFO, "Enabled extensions of Vulkan instance:");
    for (const char *name : extensions)
        QLOG(LOG_INFO, "  %italic<>%fg<bl>{}%reset", name);

    VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo{};
    if (options.use_vkdbg)
    {
        constexpr size_t count = sizeof(g_validation_layers) / sizeof(g_validation_layers[0]);
        instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(count);
        instanceCreateInfo.ppEnabledLayerNames = g_validation_layers;

        populate_debug_messenger_create_info(messengerCreateInfo, options);
        instanceCreateInfo.pNext = reinterpret_cast<VkDebugUtilsMessengerCreateInfoEXT *>(&messengerCreateInfo);
    }
    else
    {
        instanceCreateInfo.enabledLayerCount = 0;
        instanceCreateInfo.pNext = nullptr;
    }

    if (instanceCreateInfo.enabledLayerCount > 0)
    {
        QLOG(LOG_INFO, "Enabled layers of Vulkan instance:");
        for (int32_t i = 0; i < instanceCreateInfo.enabledLayerCount; i++)
            QLOG(LOG_INFO, "  %italic<>%fg<bl>{}%reset", instanceCreateInfo.ppEnabledLayerNames[i]);
    }

    VkInstance instance;
    if (vkCreateInstance(&instanceCreateInfo, nullptr, &instance) != VK_SUCCESS)
    {
        // TODO: detailed error information by VkResult
        QLOG(LOG_ERROR, "Failed to create a Vulkan instance");
        return VK_NULL_HANDLE;
    }

    return instance;
}

VkResult _vkCreateDebugUtilsMessengerEXT(VkInstance instance,
                                         const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                         const VkAllocationCallbacks *pAllocator,
                                         VkDebugUtilsMessengerEXT *pDebugMessenger)
{
    auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
    if (func != nullptr)
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    else
        return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void _vkDestroyDebugUtilsMessengerEXT(VkInstance instance,
                                      VkDebugUtilsMessengerEXT debugMessenger,
                                      const VkAllocationCallbacks *pAllocator)
{
    auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
    if (func != nullptr)
        func(instance, debugMessenger, pAllocator);
}

VkDebugUtilsMessengerEXT vk_create_debug_messenger(VkInstance instance, const HWComposeContext::Options& options)
{
    VkDebugUtilsMessengerEXT ret;
    VkDebugUtilsMessengerCreateInfoEXT info{};

    populate_debug_messenger_create_info(info, options);

    if (_vkCreateDebugUtilsMessengerEXT(instance, &info, nullptr, &ret) != VK_SUCCESS)
    {
        QLOG(LOG_ERROR, "Failed creating Vulkan debug messenger");
        return VK_NULL_HANDLE;
    }
    return ret;
}

VkPhysicalDevice vk_pick_physical_device(VkInstance instance, const HWComposeContext::Options& options)
{
    auto phys = vk_typed_enumerate<VkPhysicalDevice>([instance](uint32_t *c, VkPhysicalDevice *out) {
        vkEnumeratePhysicalDevices(instance, c, out);
    });

    if (phys.empty())
    {
        QLOG(LOG_ERROR, "No available physical devices can be used");
        return VK_NULL_HANDLE;
    }

    QLOG(LOG_INFO, "Available physical devices:");
    uint32_t idx = 0;
    for (VkPhysicalDevice device : phys)
    {
        VkPhysicalDeviceProperties prop{};
        vkGetPhysicalDeviceProperties(device, &prop);
        QLOG(LOG_INFO, "  [{}] {}", idx++, prop.deviceName);
    }

    for (VkPhysicalDevice device : phys)
    {
        auto props = vk_typed_enumerate<VkExtensionProperties>(
                [device](uint32_t *c, VkExtensionProperties *p) {
                    return vkEnumerateDeviceExtensionProperties(device, nullptr, c, p);
                });

        bool satisfied = true;

        for (const char *required : g_device_extensions)
        {
            auto r = std::find_if(props.begin(), props.end(), [required](const VkExtensionProperties& p) {
                return (std::strcmp(required, p.extensionName) == 0);
            });
            if (r == props.end())
            {
                satisfied = false;
                break;
            }
        }

        if (!satisfied)
            continue;

        for (const std::string& required : options.device_extensions)
        {
            auto r = std::find_if(props.begin(), props.end(), [required](const VkExtensionProperties& p) {
                return (required == p.extensionName);
            });
            if (r == props.end())
            {
                satisfied = false;
                break;
            }
        }

        if (satisfied)
            return device;
    }

    QLOG(LOG_ERROR, "No suitable physical devices were found");
    return VK_NULL_HANDLE;
}

} // namespace anonymous

std::shared_ptr<HWComposeContext> HWComposeContext::MakeVulkan(const Options& options)
{
    auto context = std::make_shared<HWComposeContext>();
    CHECK(context);

    context->vk_instance_ = vk_create_instance_from_options(options);
    if (context->vk_instance_ == VK_NULL_HANDLE)
        return nullptr;

    if (options.use_vkdbg)
    {
        context->vk_debug_messenger_ = vk_create_debug_messenger(context->vk_instance_, options);
        if (context->vk_debug_messenger_ == VK_NULL_HANDLE)
            return nullptr;
    }

    context->vk_physical_device_ = vk_pick_physical_device(context->vk_instance_, options);
    if (context->vk_physical_device_ == VK_NULL_HANDLE)
        return nullptr;

    std::set<std::string> deviceExtSet;
    for (const char *str : g_device_extensions)
        deviceExtSet.emplace(str);
    for (const std::string& str : options.device_extensions)
        deviceExtSet.emplace(str);

    for (const std::string& str : deviceExtSet)
        context->device_enabled_extensions_.push_back(str);

    QLOG(LOG_INFO, "Enabled extensions of Vulkan device:");
    for (const auto& name : deviceExtSet)
        QLOG(LOG_INFO, "  %italic<>%fg<bl>{}%reset", name);

    for (const char *str : vk_select_required_instance_extensions(options))
        context->instance_enabled_extensions_.emplace_back(str);

    vkGetPhysicalDeviceProperties(context->vk_physical_device_,
                                  &context->vk_physical_device_properties_);

    QLOG(LOG_INFO, "Using physical device: %fg<cy,hl>{}%reset",
         context->vk_physical_device_properties_.deviceName);

    return context;
}

HWComposeContext::HWComposeContext()
    : vk_instance_(VK_NULL_HANDLE)
    , vk_debug_messenger_(VK_NULL_HANDLE)
    , vk_physical_device_(VK_NULL_HANDLE)
    , vk_physical_device_properties_{}
{
}

HWComposeContext::~HWComposeContext()
{
    if (vk_debug_messenger_ != VK_NULL_HANDLE)
        _vkDestroyDebugUtilsMessengerEXT(vk_instance_, vk_debug_messenger_, nullptr);
    if (vk_instance_ != VK_NULL_HANDLE)
        vkDestroyInstance(vk_instance_, nullptr);
}

void HWComposeContext::Trace(GraphicsResourcesTrackable::Tracer *tracer) noexcept
{
    tracer->TraceResource("VkInstance",
                          TRACKABLE_TYPE_HANDLE,
                          TRACKABLE_DEVICE_CPU,
                          TRACKABLE_OWNERSHIP_STRICT_OWNED,
                          TraceIdFromPointer(vk_instance_));

    tracer->TraceResource("VkPhysicalDevice",
                          TRACKABLE_TYPE_HANDLE,
                          TRACKABLE_DEVICE_GPU,
                          TRACKABLE_OWNERSHIP_STRICT_OWNED,
                          TraceIdFromPointer(vk_physical_device_));

    if (vk_debug_messenger_)
    {
        tracer->TraceResource("VkDebugUtilsMessengerEXT",
                              TRACKABLE_TYPE_HANDLE,
                              TRACKABLE_DEVICE_CPU,
                              TRACKABLE_OWNERSHIP_STRICT_OWNED,
                              TraceIdFromPointer(vk_debug_messenger_));
    }
}

GLAMOR_NAMESPACE_END
