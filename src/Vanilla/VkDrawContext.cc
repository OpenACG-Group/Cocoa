#include <set>

#include "include/gpu/vk/GrVkBackendContext.h"
#include "include/gpu/vk/GrVkExtensions.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrBackendSemaphore.h"
#include "include/core/SkSurface.h"

#include "Core/Project.h"
#include "Core/Journal.h"
#include "Vanilla/VkDrawContext.h"
#include "Vanilla/Xcb/XcbDisplay.h"
#include "Vanilla/Xcb/XcbWindow.h"
VANILLA_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Vanilla)

Handle<DrawContext> DrawContext::MakeVulkan(Handle<Window> window, bool enableDebug)
{
    VkDrawContext::SurfaceCreatorPfn surfaceCreator;
    std::vector<const char*> instanceExt, deviceExt;

    switch (window->getDisplay()->backend())
    {
    case DisplayBackend::kXcb:
        instanceExt.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);
        instanceExt.emplace_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
        surfaceCreator = [window](VkInstance instance) -> std::tuple<VkResult, VkSurfaceKHR> {
            auto x11Window = std::dynamic_pointer_cast<XcbWindow>(window);
            auto x11Display = std::dynamic_pointer_cast<XcbDisplay>(window->getDisplay());
            VkXcbSurfaceCreateInfoKHR createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
            createInfo.flags = 0;
            createInfo.pNext = nullptr;
            createInfo.connection = x11Display->connection();
            createInfo.window = x11Window->window();
            VkSurfaceKHR surface;
            VkResult result = vkCreateXcbSurfaceKHR(instance, &createInfo, nullptr, &surface);
            return {result, surface};
        };
        break;
    }

    deviceExt.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    auto context = std::make_shared<VkDrawContext>(std::move(window), enableDebug,
                                                     instanceExt, deviceExt);
    context->initialize(surfaceCreator);
    return context;
}

namespace {

const char *vk_strerror(VkResult errorCode)
{
    switch (errorCode)
    {
#define STR(r) case VK_ ##r: return #r
    STR(NOT_READY);
    STR(TIMEOUT);
    STR(EVENT_SET);
    STR(EVENT_RESET);
    STR(INCOMPLETE);
    STR(ERROR_OUT_OF_HOST_MEMORY);
    STR(ERROR_OUT_OF_DEVICE_MEMORY);
    STR(ERROR_INITIALIZATION_FAILED);
    STR(ERROR_DEVICE_LOST);
    STR(ERROR_MEMORY_MAP_FAILED);
    STR(ERROR_LAYER_NOT_PRESENT);
    STR(ERROR_EXTENSION_NOT_PRESENT);
    STR(ERROR_FEATURE_NOT_PRESENT);
    STR(ERROR_INCOMPATIBLE_DRIVER);
    STR(ERROR_TOO_MANY_OBJECTS);
    STR(ERROR_FORMAT_NOT_SUPPORTED);
    STR(ERROR_SURFACE_LOST_KHR);
    STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
    STR(SUBOPTIMAL_KHR);
    STR(ERROR_OUT_OF_DATE_KHR);
    STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
    STR(ERROR_VALIDATION_FAILED_EXT);
    STR(ERROR_INVALID_SHADER_NV);
#undef STR
    default:
        return "UNKNOWN_ERROR";
    }
}

VkResult vk_dyld_create_debug_utils_messenger_ext(VkInstance instance,
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

void vk_dyld_destroy_debug_utils_messenger_ext(VkInstance instance,
                                               VkDebugUtilsMessengerEXT debugMessenger,
                                               const VkAllocationCallbacks *pAllocator)
{
    auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
    if (func != nullptr)
        func(instance, debugMessenger, pAllocator);
}

const char *vk_required_layers[] = {
        "VK_LAYER_KHRONOS_validation"
};

bool vk_has_required_layer()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> layers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, layers.data());

    for (const char *layerName : vk_required_layers)
    {
        bool found = false;
        for (const auto& property : layers)
        {
            if (!std::strcmp(property.layerName, layerName))
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            QLOG(LOG_WARNING, "Vulkan layer {} is required but not available", layerName);
            return false;
        }
        QLOG(LOG_INFO, "Found Vulkan layer {}", layerName);
    }
    return true;
}

bool vk_has_required_instance_extensions(const std::vector<const char*>& require)
{
    uint32_t count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> extensions(count);
    vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data());

    QLOG(LOG_INFO, "Vulkan instance has {} available extension(s)", count);

    for (const auto& req : require)
    {
        bool found = false;
        for (const auto& item : extensions)
            if (!std::strcmp(item.extensionName, req))
                found = true;
        if (!found)
        {
            QLOG(LOG_ERROR, "Vulkan extension {} not available", req);
            return false;
        }
    }
    return true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL
messenger_user_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                        [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT type,
                        const VkDebugUtilsMessengerCallbackDataEXT *pData,
                        [[maybe_unused]] void *pUserData)
{
    char const *psSeverity;
    switch (severity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        psSeverity = "%fg<cy,hl>VERBOSE%reset";
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        psSeverity = "%fg<gr,hl>INFO%reset";
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        psSeverity = "%fg<ye,hl>WARNING%reset";
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        psSeverity = "%fg<re,hl>ERROR%reset";
        break;
    default:
        psSeverity = "unknown";
        break;
    }

    QLOG(LOG_DEBUG, "%fg<hl>Vulkan%reset-{}: {}", psSeverity, pData->pMessage);
    return VK_FALSE;
}

void populate_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& info)
{
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    info.pfnUserCallback = messenger_user_callback;
    info.pNext = nullptr;
    info.flags = 0;
}

} // namespace anonymous

VkDrawContext::VkDrawContext(Handle<Window> window, bool enableDebug,
                                 std::vector<const char*> instanceExt,
                                 std::vector<const char*> deviceExt)
    : DrawContext(std::move(window), RasterizerType::kVulkan),
      fEnableDebug(enableDebug),
      fInstanceExtensions(std::move(instanceExt)),
      fDeviceExtensions(std::move(deviceExt)),
      fPhysicalDeviceProps{},
      fInstance(VK_NULL_HANDLE),
      fDebugMessenger(VK_NULL_HANDLE),
      fSurface(VK_NULL_HANDLE),
      fPhysicalDevice(VK_NULL_HANDLE),
      fDevice(VK_NULL_HANDLE),
      fGraphicsQueue(VK_NULL_HANDLE),
      fPresentQueue(VK_NULL_HANDLE),
      fGraphicsQueueIndex(0),
      fPresentQueueIndex(0),
      fImageFormat(VK_FORMAT_UNDEFINED),
      fSwapChain(VK_NULL_HANDLE),
      fImageSharingMode(VK_SHARING_MODE_EXCLUSIVE),
      fCurrentRT(0)
{
}

VkDrawContext::~VkDrawContext()
{
    if (fDevice != VK_NULL_HANDLE)
        vkDeviceWaitIdle(fDevice);

    destroyWholeSwapChain();
    if (fDirectContext != nullptr)
        fDirectContext = nullptr;
    if (fDevice != VK_NULL_HANDLE)
        vkDestroyDevice(fDevice, nullptr);
    if (fSurface != VK_NULL_HANDLE)
        vkDestroySurfaceKHR(fInstance, fSurface, nullptr);
    if (fEnableDebug && fDebugMessenger != VK_NULL_HANDLE)
        vk_dyld_destroy_debug_utils_messenger_ext(fInstance, fDebugMessenger, nullptr);
    if (fInstance != VK_NULL_HANDLE)
        vkDestroyInstance(fInstance, nullptr);
}

void VkDrawContext::initialize(const SurfaceCreatorPfn& surfaceCreator)
{
    uint32_t apiVersion;
    vkEnumerateInstanceVersion(&apiVersion);
    QLOG(LOG_INFO, "Vulkan Instance Version {}.{}.{}",
                   VK_VERSION_MAJOR(apiVersion),
                   VK_VERSION_MINOR(apiVersion),
                   VK_VERSION_PATCH(apiVersion));

    createGpuInstance();
    createGpuDevice(surfaceCreator);
    createDirectContext();

    checkSwapChain(this->getWindow()->width(), this->getWindow()->height());
    createRenderTargets();
}

void VkDrawContext::createGpuInstance()
{
    /* Check the supporting of validation layer */
    if (fEnableDebug && !vk_has_required_layer())
    {
        QLOG(LOG_ERROR, "Vulkan debug is unavailable as validation layer can't be found");
        fEnableDebug = false;
        throw VanillaException(__func__, "Required layers unavailable");
    }
    if (fEnableDebug)
        fInstanceExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    if (!vk_has_required_instance_extensions(fInstanceExtensions))
        throw VanillaException(__func__, "Required instance extensions unavailable");

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    appInfo.pNext = nullptr,
    appInfo.pApplicationName = "Cocoa/Vanilla",
    appInfo.applicationVersion = VK_MAKE_VERSION(COCOA_MAJOR, COCOA_MINOR, 0),
    appInfo.pEngineName = "Cocoa/Vanilla",
    appInfo.engineVersion = VK_MAKE_VERSION(VANILLA_MAJOR_VERSION, VANILLA_MAJOR_VERSION, 0),
    appInfo.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = nullptr;
    instanceCreateInfo.enabledLayerCount = 0;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.enabledExtensionCount = fInstanceExtensions.size();
    instanceCreateInfo.ppEnabledExtensionNames = fInstanceExtensions.data();

    VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo{};
    if (fEnableDebug)
    {
        populate_messenger_create_info(messengerCreateInfo);
        instanceCreateInfo.enabledLayerCount = sizeof(vk_required_layers) / sizeof(char*);
        instanceCreateInfo.ppEnabledLayerNames = vk_required_layers;
        instanceCreateInfo.pNext = &messengerCreateInfo;
    }

    VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &fInstance);
    if (result != VK_SUCCESS)
    {
        QLOG(LOG_ERROR, "Failed to create Vulkan instance: {}", vk_strerror(result));
        throw VanillaException(__func__, "Failed to create Vulkan instance");
    }

    if (fEnableDebug)
    {
        result = vk_dyld_create_debug_utils_messenger_ext(fInstance, &messengerCreateInfo,
                                                          nullptr, &fDebugMessenger);
        if (result != VK_SUCCESS)
        {
            QLOG(LOG_ERROR, "Failed to create Vulkan messenger: {}", vk_strerror(result));
            throw VanillaException(__func__, "Failed to create Vulkan messenger");
        }
    }
}

namespace {

/* Returns a tuple of {supported(boolean), graphicsQueue, presentQueue} */
std::tuple<bool, uint32_t, uint32_t> vk_phy_check_queue_families(VkPhysicalDevice phy, VkSurfaceKHR surface)
{
    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(phy, &count, nullptr);
    if (count == 0)
        return {false, 0, 0};
    std::vector<VkQueueFamilyProperties> prop(count);
    vkGetPhysicalDeviceQueueFamilyProperties(phy, &count, prop.data());

    int32_t graphics = -1, present = -1;
    for (int32_t i = 0; i < count; i++)
    {
        if (prop[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            graphics = i;

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(phy, i, surface, &presentSupport);
        if (presentSupport)
            present = i;
        if (graphics >= 0 && present >= 0)
            break;
    }

    if (present >= 0 && graphics >= 0)
        return {true, graphics, present};
    return {false, 0, 0};
}

struct SwapChainDetails
{
    VkSurfaceCapabilitiesKHR caps{};
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

SwapChainDetails vk_phy_query_swap_chain_details(VkPhysicalDevice phy, VkSurfaceKHR surface)
{
    SwapChainDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phy, surface, &details.caps);

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(phy, surface, &formatCount, nullptr);
    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(phy, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(phy, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(phy, surface, &presentModeCount,
                                                  details.presentModes.data());
    }
    return details;
}

bool vk_phy_check_extensions(VkPhysicalDevice phy,
                             const std::vector<const char*>& required)
{
    uint32_t count = 0;
    vkEnumerateDeviceExtensionProperties(phy, nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> props(count);
    vkEnumerateDeviceExtensionProperties(phy, nullptr, &count, props.data());

    for (const auto& req : required)
    {
        bool found = false;
        for (auto ext : props)
            if (!std::strcmp(ext.extensionName, req))
            {
                found = true;
                break;
            }
        if (!found)
            return false;
    }

    return true;
}

VkPhysicalDevice vk_find_phy_device(VkInstance instance,
                                    VkSurfaceKHR surface,
                                    const std::vector<const char*>& requiredPhyExts,
                                    VkPhysicalDeviceProperties& phyProps,
                                    uint32_t& graphicsQueue,
                                    uint32_t& presentQueue)
{
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(instance, &count, nullptr);
    if (count == 0)
    {
        QLOG(LOG_ERROR, "None of physical devices are available");
        return VK_NULL_HANDLE;
    }

    std::vector<VkPhysicalDevice> phys(count);
    VkResult result = vkEnumeratePhysicalDevices(instance, &count, phys.data());
    if (result != VK_SUCCESS)
    {
        QLOG(LOG_ERROR, "Failed to enumerate physical devices: {}", vk_strerror(result));
        return VK_NULL_HANDLE;
    }

    QLOG(LOG_INFO, "Found available physical devices:");;
    int32_t chosen = -1;
    for (int32_t i = 0; i < count; i++)
    {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(phys[i], &properties);
        QLOG(LOG_INFO, "  [{}] {} [API Version {}.{}.{}]", i, properties.deviceName,
                       VK_VERSION_MAJOR(properties.apiVersion),
                       VK_VERSION_MINOR(properties.apiVersion),
                       VK_VERSION_PATCH(properties.apiVersion));
        auto queueDetails = vk_phy_check_queue_families(phys[i], surface);
        if (!std::get<0>(queueDetails))
            continue;
        if (!vk_phy_check_extensions(phys[i], requiredPhyExts))
            continue;

        SwapChainDetails details = vk_phy_query_swap_chain_details(phys[i], surface);
        if (details.formats.empty() || details.presentModes.empty())
            continue;

        graphicsQueue = std::get<1>(queueDetails);
        presentQueue = std::get<2>(queueDetails);
        chosen = i;
        break;
    }
    if (chosen >= 0)
    {
        QLOG(LOG_INFO, "  Device #{} is chosen", chosen);
        return phys[chosen];
    }
    return VK_NULL_HANDLE;
}

} // namespace anonymous

void VkDrawContext::createGpuDevice(const SurfaceCreatorPfn& surfaceCreator)
{
    auto creatorResult = surfaceCreator(fInstance);
    if (std::get<0>(creatorResult) != VK_SUCCESS)
    {
        QLOG(LOG_ERROR, "Failed to create Vulkan surface: {}",
                vk_strerror(std::get<0>(creatorResult)));
        throw VanillaException(__func__, "Failed to create Vulkan surface");
    }
    fSurface = std::get<1>(creatorResult);

    fPhysicalDevice = vk_find_phy_device(fInstance,
                                         fSurface,
                                         fDeviceExtensions,
                                         fPhysicalDeviceProps,
                                         fGraphicsQueueIndex,
                                         fPresentQueueIndex);
    if (fPhysicalDevice == VK_NULL_HANDLE)
        throw VanillaException(__func__, "Failed to find a proper physical device");

    /* Create logical device */
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    constexpr float_t queuePriority = 1.0f;
    std::set<uint32_t> uniqueIndexFamilies{fGraphicsQueueIndex, fPresentQueueIndex};
    for (uint32_t queueIdx : uniqueIndexFamilies)
    {
        VkDeviceQueueCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        createInfo.queueFamilyIndex = queueIdx;
        createInfo.queueCount = 1;
        createInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(createInfo);
    }

    VkPhysicalDeviceFeatures features{};
    vkGetPhysicalDeviceFeatures(fPhysicalDevice, &features);

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.pEnabledFeatures = &features;
    deviceCreateInfo.enabledExtensionCount = fDeviceExtensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = fDeviceExtensions.data();
    deviceCreateInfo.enabledLayerCount = 0;
    if (fEnableDebug)
    {
        deviceCreateInfo.enabledLayerCount = sizeof(vk_required_layers) / sizeof(char*);
        deviceCreateInfo.ppEnabledLayerNames = vk_required_layers;
    }

    VkResult result = vkCreateDevice(fPhysicalDevice, &deviceCreateInfo, nullptr, &fDevice);
    if (result != VK_SUCCESS)
    {
        QLOG(LOG_ERROR, "Failed to create a Vulkan logical device: {}",
                vk_strerror(result));
        throw VanillaException(__func__, "Failed to create a Vulkan logical device");
    }

    vkGetDeviceQueue(fDevice, fGraphicsQueueIndex, 0, &fGraphicsQueue);
    vkGetDeviceQueue(fDevice, fPresentQueueIndex, 0, &fPresentQueue);
}


namespace {
PFN_vkVoidFunction skia_vk_proc_getter(char const *sym, VkInstance instance, VkDevice device)
{
    if (device != VK_NULL_HANDLE)
        return vkGetDeviceProcAddr(device, sym);
    return vkGetInstanceProcAddr(instance, sym);
}
} // namespace anonymous

void VkDrawContext::createDirectContext()
{
    GrVkExtensions extensions;
    extensions.init(skia_vk_proc_getter, fInstance, fPhysicalDevice,
                    fInstanceExtensions.size(), fInstanceExtensions.data(),
                    fDeviceExtensions.size(), fDeviceExtensions.data());

    VkPhysicalDeviceFeatures2 features{};
    features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    vkGetPhysicalDeviceFeatures2(fPhysicalDevice, &features);

    GrVkBackendContext backend;
    backend.fInstance = fInstance;
    backend.fPhysicalDevice = fPhysicalDevice;
    backend.fDevice = fDevice;
    backend.fQueue = fGraphicsQueue;
    backend.fGraphicsQueueIndex = fGraphicsQueueIndex;
    backend.fMaxAPIVersion = VK_API_VERSION_1_1;
    backend.fVkExtensions = &extensions;
    backend.fGetProc = skia_vk_proc_getter;
    backend.fDeviceFeatures2 = &features;

    fDirectContext = GrDirectContext::MakeVulkan(backend);
    if (fDirectContext == nullptr)
        throw VanillaException(__func__, "Failed to create Skia GPU Context");
}

namespace {

VkFormat native_format_to_vk_format(SkColorType fmt)
{
    switch (fmt)
    {
    case SkColorType::kBGRA_8888_SkColorType:
        return VK_FORMAT_B8G8R8A8_UNORM;

    default:
        throw VanillaException(__func__, "Unsupported color format");
    }
}

VkSurfaceFormatKHR vk_choose_surface_format(SkColorType native,
                                            const SwapChainDetails& details)
{
    VkFormat target = native_format_to_vk_format(native);
    for (const auto& format : details.formats)
    {
        if (format.format == target &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return format;
    }

    return VkSurfaceFormatKHR{.format = VK_FORMAT_UNDEFINED};
}

VkPresentModeKHR vk_choose_presentation_mode(SwapChainDetails details)
{
    bool mailboxAvailable = false;
    bool fifoAvailable = false;
    for (VkPresentModeKHR mode : details.presentModes)
    {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
            mailboxAvailable = true;
        else if (mode == VK_PRESENT_MODE_FIFO_KHR)
            fifoAvailable = true;
    }

    if (mailboxAvailable)
        return VK_PRESENT_MODE_MAILBOX_KHR;
    if (fifoAvailable)
        return VK_PRESENT_MODE_FIFO_KHR;

    throw VanillaException(__func__, "Could not find a proper presentation mode");
}

} // namespace anonymous

void VkDrawContext::checkSwapChain(int32_t width, int32_t height)
{
    if (width <= 0 || height <= 0)
        throw VanillaException(__func__, "Invalid geometry size");

    SwapChainDetails details = vk_phy_query_swap_chain_details(fPhysicalDevice, fSurface);
#if 0
    if (width < details.caps.minImageExtent.width ||
        height < details.caps.minImageExtent.height)
        throw VanillaException(__func__, "Invalid geometry size");
    
    if (width > details.caps.maxImageExtent.width ||
        height > details.caps.maxImageExtent.height)
        throw VanillaException(__func__, "Invalid geometry size");
#endif

    fImageFormat = native_format_to_vk_format(this->getWindow()->format());

    /* Choose a proper surface format */
    VkSurfaceFormatKHR surfaceFormat = vk_choose_surface_format(this->getWindow()->format(),
                                                                details);
    if (surfaceFormat.format == VK_FORMAT_UNDEFINED)
    {
        QLOG(LOG_ERROR, "Failed to match a proper image format for Vulkan swapchain");
        throw VanillaException(__func__, "Failed to match a proper image format");
    }

    /* Choose a proper presentation mode */
    VkPresentModeKHR presentMode = vk_choose_presentation_mode(details);

    fImageExtent.width = width;
    fImageExtent.height = height;

    uint32_t imageCount = details.caps.minImageCount + 1;
    if (details.caps.maxImageCount > 0 && imageCount > details.caps.maxImageCount)
        imageCount = details.caps.maxImageCount;
    
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = fSurface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = fImageExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                            VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                            VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    
    uint32_t indices[2] = {fGraphicsQueueIndex, fPresentQueueIndex};
    if (fGraphicsQueueIndex == fPresentQueueIndex)
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = indices;
    }
    fImageSharingMode = createInfo.imageSharingMode;

    createInfo.preTransform = details.caps.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = fSwapChain;

    VkResult result = vkCreateSwapchainKHR(fDevice, &createInfo, nullptr, &fSwapChain);
    if (result != VK_SUCCESS)
    {
        QLOG(LOG_ERROR, "Failed to create swapchain: {}", vk_strerror(result));
        throw VanillaException(__func__, "Failed to create swapchain");
    }
}

struct VaVkRenderTarget
{
    explicit VaVkRenderTarget(VkDevice dev, VkSemaphore sem)
        : imageIdx(UINT32_MAX),
          device(dev),
          semaphore(sem),
          acquired(false) {}
    ~VaVkRenderTarget()
    {
        if (device != VK_NULL_HANDLE && semaphore != VK_NULL_HANDLE)
            vkDestroySemaphore(device, semaphore, nullptr);
    }

    uint32_t imageIdx;
    VkDevice device;
    VkSemaphore semaphore;
    bool acquired;
};

void VkDrawContext::createRenderTargets()
{
    uint32_t imageCount = 0;
    vkGetSwapchainImagesKHR(fDevice, fSwapChain, &imageCount, nullptr);
    if (imageCount == 0)
        throw VanillaException(__func__, "None of images are available in swapchain");
    std::vector<VkImage> images(imageCount);
    vkGetSwapchainImagesKHR(fDevice, fSwapChain, &imageCount, images.data());

    /* Create semaphores first */
    VkSemaphoreCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    fRenderTargets.resize(imageCount + 1);
    for (int32_t i = 0; i < imageCount + 1; i++)
    {
        VkSemaphore semaphore = VK_NULL_HANDLE;
        VkResult result = vkCreateSemaphore(fDevice, &createInfo, nullptr, &semaphore);
        if (result != VK_SUCCESS)
        {
            QLOG(LOG_ERROR, "Failed to create Vulkan semaphore: {}", vk_strerror(result));
            throw VanillaException(__func__, "Failed to create Vulkan semaphore");
        }
        fRenderTargets[i] = std::make_shared<VaVkRenderTarget>(fDevice, semaphore);
    }

    /* Then create surfaces */
    fRTSurfaces.resize(imageCount);
    for (int32_t i = 0; i < imageCount; i++)
    {
        GrVkImageInfo info;
        info.fImage = images[i];
        info.fImageLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        info.fFormat = fImageFormat;
        info.fImageUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        info.fLevelCount = 1;
        info.fSharingMode = fImageSharingMode;

        GrBackendRenderTarget target(fImageExtent.width, fImageExtent.height, info);
        fRTSurfaces[i] = SkSurface::MakeFromBackendRenderTarget(fDirectContext.get(),
                                                                target,
                                                                GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin,
                                                                this->getWindow()->format(),
                                                                SkColorSpace::MakeSRGB(),
                                                                nullptr);
        if (fRTSurfaces[i] == nullptr)
            throw VanillaException(__func__, "Failed to create Skia GPU surface");
    }

    fCurrentRT = 0;
}

void VkDrawContext::destroyWholeSwapChain()
{
    fRTSurfaces.clear();
    fRenderTargets.clear();
    if (fSwapChain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(fDevice, fSwapChain, nullptr);
        fSwapChain = VK_NULL_HANDLE;
    }
}

void VkDrawContext::onResize(int32_t width, int32_t height)
{
    if (fSwapChain != VK_NULL_HANDLE)
    {
        vkDeviceWaitIdle(fDevice);

        destroyWholeSwapChain();
        this->checkSwapChain(width, height);
        createRenderTargets();
    }
}

sk_sp<SkSurface> VkDrawContext::onBeginFrame(const SkRect& region)
{
    Handle<VaVkRenderTarget> rt = fRenderTargets[fCurrentRT];
    sk_sp<SkSurface> surface = nullptr;
    if (!rt->acquired)
    {
        VkSemaphoreCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkSemaphore semaphore;

        vkCreateSemaphore(fDevice, &createInfo, nullptr, &semaphore);
        VkResult result = vkAcquireNextImageKHR(fDevice, fSwapChain, UINT64_MAX, semaphore,
                                                VK_NULL_HANDLE, &rt->imageIdx);
        if (result != VK_SUCCESS)
        {
            vkDestroySemaphore(fDevice, semaphore, nullptr);
            if (result == VK_ERROR_OUT_OF_DATE_KHR)
            {
                /* The swapchain is out of date now, we need to recreate it.
                 * But the Window object should notify us to do this later, so we just
                 * return nullptr to skip this frame. */
                return nullptr;
            }
            QLOG(LOG_ERROR, "Failed to acquire next Vulkan frame: {}", vk_strerror(result));
            throw VanillaException(__func__, "Failed to acquire next Vulkan frame");
        }

        surface = fRTSurfaces[rt->imageIdx];
        GrBackendSemaphore backendSemaphore;
        backendSemaphore.initVulkan(semaphore);
        surface->wait(1, &backendSemaphore);
        rt->acquired = true;
    }
    else
        surface = fRTSurfaces[rt->imageIdx];
    return surface;
}

void VkDrawContext::onEndFrame(const SkRect& region)
{
    Handle<VaVkRenderTarget> rt = fRenderTargets[fCurrentRT];
    if (!rt->acquired)
        return;

    sk_sp<SkSurface> surface = fRTSurfaces[rt->imageIdx];

    GrBackendSemaphore renderFinishSemaphore;
    renderFinishSemaphore.initVulkan(rt->semaphore);
    GrFlushInfo flushInfo{
        .fNumSemaphores = 1,
        .fSignalSemaphores = &renderFinishSemaphore
    };

    GrBackendSurfaceMutableState state(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                       fGraphicsQueueIndex);
    if (surface->flush(flushInfo, &state) != GrSemaphoresSubmitted::kYes)
    {
        QLOG(LOG_ERROR, "Failed in submitting commands to GPU");
        return;
    }
    fDirectContext->submit();

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &rt->semaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &fSwapChain;
    presentInfo.pImageIndices = &rt->imageIdx;

    vkQueuePresentKHR(fPresentQueue, &presentInfo);
    rt->acquired = false;

    fCurrentRT = (fCurrentRT + 1) % fRenderTargets.size();
}

int VkDrawContext::getMaxSampleCount()
{
    VkPhysicalDeviceProperties prop{};
    vkGetPhysicalDeviceProperties(fPhysicalDevice, &prop);

    VkSampleCountFlags c = std::min(prop.limits.framebufferColorSampleCounts,
                                    prop.limits.framebufferDepthSampleCounts);
    if (c & VK_SAMPLE_COUNT_64_BIT)
        return 64;
    else if (c & VK_SAMPLE_COUNT_32_BIT)
        return 32;
    else if (c & VK_SAMPLE_COUNT_16_BIT)
        return 16;
    else if (c & VK_SAMPLE_COUNT_8_BIT)
        return 8;
    else if (c & VK_SAMPLE_COUNT_4_BIT)
        return 4;
    else if (c & VK_SAMPLE_COUNT_2_BIT)
        return 2;
    return 1;
}

sk_sp<SkSurface> VkDrawContext::createBackendSurface(const SkImageInfo& info, SkBudgeted budgeted)
{
    sk_sp<SkSurface> surface = SkSurface::MakeRenderTarget(fDirectContext.get(),
                                                           budgeted,
                                                           info,
                                                           getMaxSampleCount(),
                                                           GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin,
                                                           nullptr);
    return surface;
}

VANILLA_NS_END
