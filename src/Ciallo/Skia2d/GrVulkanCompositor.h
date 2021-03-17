#ifndef __GPU_COMPOSITOR_H__
#define __GPU_COMPOSITOR_H__

#define SK_VULKAN

#include <memory>
#include <cstdint>

#include <vulkan/vulkan.h>

#include "include/core/SkImage.h"
#include "include/core/SkSurface.h"
#include "include/core/SkRect.h"
#include "include/gpu/GrDirectContext.h"

#include "Core/Exception.h"
#include "Ciallo/Skia2d/GrBaseCompositor.h"

CIALLO_BEGIN_NS

struct GpuSwapchainDetails
{
    VkSurfaceCapabilitiesKHR        fCaps;
    std::vector<VkSurfaceFormatKHR> fFormats;
    std::vector<VkPresentModeKHR>   fPresentModes;
};

struct GpuQueueFamilyIndices
{
    std::optional<uint32_t>     fGraphics;
    std::optional<uint32_t>     fPresent;
    bool isComplete() const
    { return fGraphics.has_value() && fPresent.has_value(); }
};

struct GpuBufferInfo
{
    uint32_t        fImageIndex;
    VkSemaphore     fRenderSemaphore;
    bool            fAcquired = false;
};

class GrVulkanCompositor : public GrBaseCompositor
{
    friend class GrBaseCompositor;

public:
    GrVulkanCompositor(Drawable *drawable,
                       const std::vector<std::string>& requiredVkInstanceExts,
                       const std::vector<std::string>& requiredVkDeviceExts,
                       bool debug);
    ~GrVulkanCompositor() override;

private:
    void onPresent() override;
    SkSurface *onTargetSurface() override;
    GrBaseRenderLayer *onCreateRenderLayer(int32_t width,
                                         int32_t height,
                                         int32_t px,
                                         int32_t py,
                                         int zindex) override;

private:
    static PFN_vkVoidFunction skia_vulkan_funcptr(const char *sym, VkInstance instance, VkDevice device);

    void initObject();
    void finalizeObject();

    void makeGpuInstance();
    void makeGpuSurface();
    void pickGpuPhysicalDevice();
    void makeGpuLogicalDevice();
    void makeGpuSwapchain();
    void makeGpuBuffers();

    void makeDirectContext();
    void makeSurfaces();

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& fmts);
    static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& modes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& caps);
    GpuSwapchainDetails querySwapchainDetails(VkPhysicalDevice device);
    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    GpuQueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    static bool checkValidationLayerSupport();
    std::vector<const char*> chooseRequiredInstanceExtensions();

    static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& info);
    void makeDebugMessenger();

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                        VkDebugUtilsMessageTypeFlagsEXT type,
                                                        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                        void *pUserData);

    void nextFrame();
    SkSurface *currentSkSurface();

private:
    std::vector<std::string>    fVkRequiredInstanceExts;
    std::vector<std::string>    fVkRequiredDeviceExts;

    VkPhysicalDeviceProperties  fVkPhysicalDeviceProps;

    VkInstance                  fVkInstance;
    bool                        fHasDebug;
    VkDebugUtilsMessengerEXT    fVkDebugMessenger;
    VkSurfaceKHR                fVkSurface;
    VkPhysicalDevice            fVkPhysicalDevice;
    VkDevice                    fVkDevice;
    VkQueue                     fVkGraphicsQueue;
    VkQueue                     fVkPresentQueue;
    uint32_t                    fVkGraphicsQueueIndex;
    VkSwapchainKHR              fVkSwapchain;
    std::vector<VkImage>        fVkSwapchainImages;
    VkFormat                    fVkImageFormat;
    VkExtent2D                  fVkSwapchainExtent;
    VkSharingMode               fVkImagesSharingMode;
    uint32_t                    fImagesCount;

    GrDirectContext            *fDirectContext;
    std::vector<SkSurface*>     fSurfaces;
    std::vector<GpuBufferInfo>  fBuffers;
    uint64_t                    fCurrentBackBuffer;
};

CIALLO_END_NS

#endif // __GPU_COMPOSITOR_H__
