#include <memory>

#include "include/core/SkImageInfo.h"
#include "include/core/SkSurface.h"

#include "Core/Exception.h"
#include "Ciallo/Ciallo.h"
#include "Ciallo/Skia2d/GrBaseCompositor.h"
#include "Ciallo/Skia2d/GrRasterCompositor.h"
#include "Ciallo/Skia2d/GrRasterRenderLayer.h"
CIALLO_BEGIN_NS

#define NULL_XID    0

std::unique_ptr<GrBaseCompositor> GrBaseCompositor::MakeRaster(Drawable *drawable)
{
    auto ret = std::make_unique<GrRasterCompositor>(drawable);
    ret->createSurface();
    return ret;
}

GrRasterCompositor::GrRasterCompositor(Drawable *drawable)
    : GrBaseCompositor(CompositeDevice::kCpuDevice,
                       drawable),
      fBitmapSurface(nullptr)
{
    setDriverSpecDeviceTypeInfo(CompositeDriverSpecDeviceType::kDirectCpu);
    setDeviceInfo(CIALLO_ROMAN_CPU_DRIVER_VERSION,
                  CIALLO_ROMAN_CPU_API_VERSION,
                  CIALLO_ROMAN_CPU_VENDOR,
                  CIALLO_ROMAN_CPU_DEVICE_NAME);
}

GrRasterCompositor::~GrRasterCompositor()
{
    this->Dispose();
}

void GrRasterCompositor::createSurface()
{
    auto format = SkColorInfoFromImageFormat(drawable()->format());

    SkISize size = SkISize::Make(this->width(), this->height());
    SkImageInfo imageInfo = SkImageInfo::Make(size, std::get<0>(format), SkAlphaType::kPremul_SkAlphaType);
    fBitmapSurface = SkSurface::MakeRaster(imageInfo, imageInfo.minRowBytes(), nullptr);
    if (fBitmapSurface == nullptr)
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("Failed to create SkSurface to render to")
                .make<RuntimeException>();
    }
}

SkSurface *GrRasterCompositor::onTargetSurface()
{
    RUNTIME_EXCEPTION_ASSERT(fBitmapSurface != nullptr);
    return fBitmapSurface.get();
}

void GrRasterCompositor::onPresent()
{
    SkPixmap pixmap;
    if (!fBitmapSurface->peekPixels(&pixmap))
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("Failed to reading pixels from composition buffer")
                .make<RuntimeException>();
    }

    SkIRect clip = SkIRect::MakeXYWH(0, 0, this->width(), this->height());

    drawable()->writePixmap(pixmap, clip);
}

GrBaseRenderLayer *GrRasterCompositor::onCreateRenderLayer(int32_t width, int32_t height,
                                                           int32_t left, int32_t top,
                                                           int zindex)
{
    auto format = SkColorInfoFromImageFormat(drawable()->format());

    SkISize size = SkISize::Make(width, height);
    SkImageInfo imageInfo = SkImageInfo::Make(size, std::get<0>(format), SkAlphaType::kPremul_SkAlphaType);
    return new GrRasterRenderLayer(this, left, top, zindex,
                                   width, height, imageInfo);
}

CIALLO_END_NS
