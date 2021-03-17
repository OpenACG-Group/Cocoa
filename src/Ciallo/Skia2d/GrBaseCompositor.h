#ifndef __BASE_COMPOSITOR_H__
#define __BASE_COMPOSITOR_H__

#include <string>
#include <memory>
#include <map>
#include <mutex>

#include "include/core/SkSurface.h"
#include "include/core/SkImage.h"
#include "include/core/SkRect.h"
#include "include/gpu/GrDirectContext.h"

#include "Ciallo/Ciallo.h"
#include "Ciallo/Drawable.h"
#include "Ciallo/Skia2d/GrBaseRenderLayer.h"

CIALLO_BEGIN_NS

class GrBaseCompositor
{
public:
    enum class CompositeDevice
    {
        kCpuDevice,
        kGpuVulkan,
    };

    enum class CompositeDriverSpecDeviceType
    {
        kDirectCpu,
        kVulkan_Other,
        kVulkan_IntergratedGpu,
        kVulkan_DiscreteGpu,
        kVulkan_VirtualGpu,
        kVulkan_Cpu
    };

    struct CompositeBackendInfo
    {
        CompositeDevice                 fDeviceType;
        CompositeDriverSpecDeviceType   fDriverSpecDeviceType;
        uint32_t                        fDriverVersion;
        uint32_t                        fAPIVersion;
        uint32_t                        fDeviceVendor;
        std::string                     fDeviceName;
        std::vector<std::string>        fGpuExtensions;
    };

    struct LayerBinder
    {
        GrBaseRenderLayer *fHandle = nullptr;
        SkIRect          fSubmittedClip;
        sk_sp<SkImage>   fSubmittedImage = nullptr;
        int64_t          fDroppedFrames = 0;
    };

    using RenderLayerID = uint32_t;
    using RenderLayerIteator = std::map<int, RenderLayerID>::iterator;

    static std::unique_ptr<GrBaseCompositor> MakeVulkan(Drawable *drawable,
                                                        const std::vector<std::string>& requiredVkInstanceExts = {},
                                                        const std::vector<std::string>& requiredVkDeviceExts = {},
                                                        bool debugMode = false);

    static std::unique_ptr<GrBaseCompositor> MakeRaster(Drawable *drawable);

    virtual ~GrBaseCompositor();

    inline CompositeDevice getDeviceType() const
                                            { return fBackendInfo.fDeviceType; }
    inline CompositeDriverSpecDeviceType getDetailedDeviceType() const
                                            { return fBackendInfo.fDriverSpecDeviceType; }

    inline uint32_t getDriverVersion() const       { return fBackendInfo.fDriverVersion; }
    inline uint32_t getAPIVersion() const          { return fBackendInfo.fAPIVersion; }
    inline uint32_t getDeviceVendor() const        { return fBackendInfo.fDeviceVendor; }
    inline std::string getDeviceName() const       { return fBackendInfo.fDeviceName; }

    inline const std::vector<std::string>& getGpuExtensions() const
                                            { return fBackendInfo.fGpuExtensions; }

    inline int32_t width()  const { return fWidth; }
    inline int32_t height() const { return fHeight; }

    void submit(GrBaseRenderLayer *who, const sk_sp<SkImage>& result, const SkIRect& clipRect);

    /**
     * @brief Presents current frame.
     * 
     * Rasterizer submits the rendered texture to its own compositor,
     * which generates a new frame. But the new frame is not actually
     * composited. Call present() to composite them onto the window.
     * Note that even if the rasterizer submits textures multiple times,
     * they will not be displayed until present is called. The content of
     * the final frame is the last texture submitted.
     */
    void present();

    /**
     * @brief Creates a new render layer with a rasterizer.
     * @param width: Width of new layer.
     * @param height: Height of new layer.
     * @param left: The coordinate of the upper-left corner of the rectangle
     *            where the layer is located.
     * @param top: See above.
     * @param zindex: The stacking order of layer. The lower layer has
     *                lower value.
    */
    std::shared_ptr<GrBaseRenderLayer> overlay(int32_t width,
                                               int32_t height,
                                               int32_t left,
                                               int32_t top,
                                               int zindex);

    /* Swaps two layers by their Z-index value */
    void swapRenderLayers(int a, int b);

    RenderLayerIteator begin();
    RenderLayerIteator end();

    inline Drawable *drawable() const
    {
        return fDrawable;
    }

protected:
    GrBaseCompositor(CompositeDevice device,
                     Drawable *drawable);

    void setDriverSpecDeviceTypeInfo(CompositeDriverSpecDeviceType type);
    void setDeviceInfo(uint32_t driverVersion,
                       uint32_t APIVersion,
                       uint32_t vendor,
                       const std::string& device);
    void appendGpuExtensionsInfo(const std::string& ext);

    virtual SkSurface *onTargetSurface() = 0;
    virtual void onPresent() = 0;
    virtual GrBaseRenderLayer *onCreateRenderLayer(int32_t width,
                                                   int32_t height,
                                                   int32_t left,
                                                   int32_t top,
                                                   int zindex) = 0;

    /* Call this before inherit class destructing */
    void Dispose();

private:
    int32_t                         fWidth;
    int32_t                         fHeight;
    CompositeBackendInfo            fBackendInfo;
    std::map<int, RenderLayerID>    fLayerIDMap;
    std::vector<LayerBinder>        fLayers;
    Drawable                       *fDrawable;
};

CIALLO_END_NS

#endif
