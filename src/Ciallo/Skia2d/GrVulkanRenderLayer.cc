#include "Ciallo/Skia2d/GrVulkanRenderLayer.h"

CIALLO_BEGIN_NS

GrVulkanRenderLayer::GrVulkanRenderLayer(GrBaseCompositor *compositor,
                                         GrDirectContext *ctx,
                                         int32_t x,
                                         int32_t y,
                                         int32_t z,
                                         const SkImageInfo &imageInfo)
    : GrBaseRenderLayer(compositor, x, y, z,
                        imageInfo.width(), imageInfo.height()),
      fDirectContext(ctx),
      fImageInfo(imageInfo)
{
}

SkCanvas *GrVulkanRenderLayer::onCreateCanvas()
{
    if (fSurface == nullptr)
        createSurface();
    return fSurface->getCanvas();
}

sk_sp<SkImage> GrVulkanRenderLayer::onLayerResult()
{
    return fSurface->makeImageSnapshot();
}

void GrVulkanRenderLayer::createSurface()
{
    fSurface = SkSurface::MakeRenderTarget(fDirectContext,
                                           SkBudgeted::kNo,
                                           fImageInfo,
                                           4,
                                           GrSurfaceOrigin::kBottomLeft_GrSurfaceOrigin,
                                           nullptr);
    
    RUNTIME_EXCEPTION_ASSERT(fSurface != nullptr);
}

CIALLO_END_NS
