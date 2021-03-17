#include <Poco/UUID.h>
#include <Poco/UUIDGenerator.h>

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPicture.h"
#include "include/core/SkMatrix.h"
#include "include/codec/SkCodec.h"
#include "include/core/SkStream.h"

#include "Ciallo/Skia2d/GrBaseCompositor.h"
#include "Ciallo/Skia2d/GrBaseRenderLayer.h"

CIALLO_BEGIN_NS

GrBaseRenderLayer::GrBaseRenderLayer(GrBaseCompositor *compositor,
                                     int32_t x, int32_t y, int32_t z,
                                     int32_t w, int32_t h)
    : fCompositor(compositor),
      fProperties{
          .fWidth = w,
          .fHeight = h,
          .fLeft = x,
          .fTop = y,
          .fZIndex = z,
          .fVisible = true
      },
      fCanvas(nullptr)
{
    Poco::UUIDGenerator UUIDGen;
    fProperties.fUUID = UUIDGen.createRandom();
}

int GrBaseRenderLayer::elevate()
{
    int upperZIndex;
    bool foundThis = false, foundUpper = false;
    for (auto layerPair : *fCompositor)
    {
        if (foundThis)
        {
            upperZIndex = layerPair.first;
            foundUpper = true;
            break;
        }
        if (layerPair.first == fProperties.fZIndex)
            foundThis = true;
    }
    /* It is meaningless to elevate a layer at the top */
    if (!foundUpper)
        return fProperties.fZIndex;

    fCompositor->swapRenderLayers(fProperties.fZIndex, upperZIndex);
    return fProperties.fZIndex;
}

int GrBaseRenderLayer::depress()
{
    int lowerZIndex;
    bool hasLower = false;
    for (auto layerPair : *fCompositor)
    {
        if (layerPair.first == fProperties.fZIndex)
            break;
        lowerZIndex = layerPair.first;
        hasLower = true;
    }
    /* It is meaningless to depress a layer at the bottom */
    if (!hasLower)
        return fProperties.fZIndex;
    
    fCompositor->swapRenderLayers(fProperties.fZIndex, lowerZIndex);
    return fProperties.fZIndex;
}

void GrBaseRenderLayer::setVisibility(bool visible)
{
    fProperties.fVisible = visible;
}

void GrBaseRenderLayer::moveTo(int32_t left, int32_t top)
{
    fProperties.fLeft = left;
    fProperties.fTop = top;
}

void GrBaseRenderLayer::update()
{
    if ((fDirtyBoundary.fRight - fDirtyBoundary.fLeft)
        * (fDirtyBoundary.fBottom - fDirtyBoundary.fTop) == 0)
        return;
    
    SkIRect clipRect = SkIRect::MakeLTRB(fDirtyBoundary.fLeft,
                                         fDirtyBoundary.fTop,
                                         fDirtyBoundary.fRight,
                                         fDirtyBoundary.fBottom);

    fCompositor->submit(this, onLayerResult(), clipRect);

    fDirtyBoundary.fLeft = 0;
    fDirtyBoundary.fTop = 0;
    fDirtyBoundary.fRight = 0;
    fDirtyBoundary.fBottom = 0;
}

void GrBaseRenderLayer::paint(const sk_sp<SkPicture>& picture,
                            int32_t left,
                            int32_t top,
                            const SkPaint *paint)
{
    SkMatrix mat = SkMatrix::Translate(left, top);
    getCanvas()->drawPicture(picture, &mat, paint);

    SkRect cullRect = picture->cullRect();
    updateDirtyBoundary(SkIRect::MakeLTRB(left + cullRect.left(),
                                          top + cullRect.top(),
                                          left + cullRect.right(),
                                          top + cullRect.bottom()));
}

void GrBaseRenderLayer::drawImageFile(const std::string &file,
                                      int32_t x,
                                      int32_t y,
                                      int32_t width,
                                      int32_t height,
                                      const SkPaint *paint)
{
    RUNTIME_EXCEPTION_ASSERT(x >= 0 && y >= 0);

    auto stream = SkStream::MakeFromFile(file.c_str());
    if (stream == nullptr)
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("Failed to load file")
                .append(file)
                .make<RuntimeException>();
    }

    std::unique_ptr<SkCodec> codec = SkCodec::MakeFromStream(std::move(stream));
    if (codec == nullptr)
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("Failed to decode file")
                .make<RuntimeException>();
    }

    SkImageInfo info = codec->getInfo();
    SkBitmap bitmap;
    bitmap.setInfo(info);
    bitmap.allocPixels();
    codec->getPixels(bitmap.pixmap());

    if (width < 0)
        width = info.width();
    if (height < 0)
        height = info.height();

    SkIRect dstBoundaryRect = SkIRect::MakeXYWH(x, y,
                                                std::min(x + width, this->width()) - x,
                                                std::min(y + height, this->height()) - y);
    SkRect dstRect = SkRect::MakeXYWH(x, y, width, height);
    getCanvas()->drawBitmapRect(bitmap, dstRect, paint);

    updateDirtyBoundary(dstBoundaryRect);
}

void GrBaseRenderLayer::updateDirtyBoundary(const SkIRect& rect)
{
    if (fDirtyBoundary.fLeft > rect.left())
        fDirtyBoundary.fLeft = rect.left();

    if (fDirtyBoundary.fTop > rect.top())
        fDirtyBoundary.fTop = rect.top();

    if (fDirtyBoundary.fRight < rect.right())
        fDirtyBoundary.fRight = rect.right();
        
    if (fDirtyBoundary.fBottom < rect.bottom())
        fDirtyBoundary.fBottom = rect.bottom();
}

SkCanvas *GrBaseRenderLayer::getCanvas()
{
    if (fCanvas == nullptr)
        fCanvas = onCreateCanvas();

    RUNTIME_EXCEPTION_ASSERT(fCanvas != nullptr);
    return fCanvas;
}

void GrBaseRenderLayer::setZIndex(int z)
{
    fProperties.fZIndex = z;
}

CIALLO_END_NS
