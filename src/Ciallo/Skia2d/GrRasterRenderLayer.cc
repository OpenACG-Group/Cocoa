#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkImageInfo.h"

#include "Ciallo/Ciallo.h"
#include "Ciallo/Skia2d/GrRasterRenderLayer.h"
CIALLO_BEGIN_NS

GrRasterRenderLayer::GrRasterRenderLayer(GrBaseCompositor *compositor,
                                         int32_t x,
                                         int32_t y,
                                         int32_t z,
                                         int32_t w,
                                         int32_t h,
                                         const SkImageInfo &imageInfo)
    : GrBaseRenderLayer(compositor, x, y, z, w, h),
      fImageInfo(imageInfo),
      fSurface(nullptr)
{
}

sk_sp<SkImage> GrRasterRenderLayer::onLayerResult()
{
    RUNTIME_EXCEPTION_ASSERT(fSurface != nullptr);
    return fSurface->makeImageSnapshot();
}

SkCanvas *GrRasterRenderLayer::onCreateCanvas()
{
    if (fSurface != nullptr)
        return fSurface->getCanvas();
    fSurface = SkSurface::MakeRaster(fImageInfo, nullptr);
    if (fSurface == nullptr)
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("Failed to create SkSurface to render to")
                .make<RuntimeException>();
    }

    return fSurface->getCanvas();
}

CIALLO_END_NS
