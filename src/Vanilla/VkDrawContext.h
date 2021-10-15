#ifndef COCOA_VKDRAWCONTEXT_H
#define COCOA_VKDRAWCONTEXT_H

#include <vector>
#include <tuple>
#include <functional>

#define VK_USE_PLATFORM_XCB_KHR 1
#include <vulkan/vulkan.h>

#include "include/core/SkSurface.h"
#include "Vanilla/Base.h"
#include "Vanilla/DrawContext.h"
VANILLA_NS_BEGIN

struct VaVkRenderTarget;
class VkDrawContext : public DrawContext
{
public:
    using SurfaceCreatorPfn = std::function<std::tuple<VkResult, VkSurfaceKHR>(VkInstance)>;

    VkDrawContext(Handle<Window> window, bool enableDebug,
                    std::vector<const char*> instanceExt,
                    std::vector<const char*> deviceExt);
    ~VkDrawContext() override;

    void initialize(const SurfaceCreatorPfn& surfaceCreator);

    va_nodiscard inline GrDirectContext *getDirectContext() const {
        return fDirectContext.get();
    }

    sk_sp<SkSurface> createBackendSurface(const SkImageInfo &info, SkBudgeted budgeted) override;

private:
    sk_sp<SkSurface> onBeginFrame(const SkRect& region) override;
    void onEndFrame(const SkRect& region) override;
    void onResize(int32_t width, int32_t height) override;

    int getMaxSampleCount();
    void createGpuInstance();
    void createGpuDevice(const SurfaceCreatorPfn& surfaceCreator);
    void createDirectContext();
    void checkSwapChain(int32_t width, int32_t height);
    void createRenderTargets();
    void destroyWholeSwapChain();


    bool                                fEnableDebug;
    std::vector<const char*>            fInstanceExtensions;
    std::vector<const char*>            fDeviceExtensions;
    VkPhysicalDeviceProperties          fPhysicalDeviceProps;
    VkInstance                          fInstance;
    VkDebugUtilsMessengerEXT            fDebugMessenger;
    VkSurfaceKHR                        fSurface;
    VkPhysicalDevice                    fPhysicalDevice;
    VkDevice                            fDevice;
    VkQueue                             fGraphicsQueue;
    VkQueue                             fPresentQueue;
    uint32_t                            fGraphicsQueueIndex;
    uint32_t                            fPresentQueueIndex;
    std::vector<Handle<VaVkRenderTarget>>
                                        fRenderTargets;
    std::vector<sk_sp<SkSurface>>       fRTSurfaces;
    VkExtent2D                          fImageExtent;
    VkFormat                            fImageFormat;
    VkSwapchainKHR                      fSwapChain;
    VkSharingMode                       fImageSharingMode;
    sk_sp<GrDirectContext>              fDirectContext;
    uint32_t                            fCurrentRT;
};

VANILLA_NS_END
#endif //COCOA_VKDRAWCONTEXT_H
