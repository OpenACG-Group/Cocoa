#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkBitmap.h"

#include "Core/Data.h"
#include "Vanilla/DrawContext.h"
#include "Vanilla/VkDrawContext.h"
#include "Vanilla/RenderKit/ContentAggregator.h"
#include "Vanilla/RenderKit/ImageLayer.h"
VANILLA_NS_BEGIN

ImageLayer::ImageLayer(ImageAdaptationMethod adapt,
                       const WeakHandle<ContentAggregator>& aggregator, bool opaque,
                       uint32_t x, uint32_t y, uint32_t w, uint32_t h)
    : Layer(aggregator, opaque, x, y, w, h)
    , fImage(nullptr)
    , fAdaptation(adapt)
    , fDirectContext(nullptr)
    , fBGFillColor(SK_ColorWHITE)
{
    Handle<DrawContext> dc = Layer::getContentAggregator()->getDrawContext();
    if (dc->backend() == DrawContext::RasterizerType::kVulkan)
    {
        /* For GPU backend, acquires its GrDirectContext pointer */
        auto vkDC = std::dynamic_pointer_cast<VkDrawContext>(dc);
        assert(vkDC);
        fDirectContext = vkDC->getDirectContext();
    }
}

namespace {

void adapt_image_repeat_x(SkCanvas *pCanvas, const sk_sp<SkImage>& src, uint32_t dstW)
{
    uint32_t w = src->width();
    for (uint32_t x = 0; x < dstW; x += w)
    {
        pCanvas->drawImage(src, static_cast<SkScalar>(x), 0);
    }
}

void adapt_image_repeat_y(SkCanvas *pCanvas, const sk_sp<SkImage>& src, uint32_t dstH)
{
    uint32_t h = src->height();
    for (uint32_t y = 0; y < dstH; y += h)
    {
        pCanvas->drawImage(src, 0, static_cast<SkScalar>(y));
    }
}

void adapt_image_repeat_xy(SkCanvas *pCanvas, const sk_sp<SkImage>& src, uint32_t dw, uint32_t dh)
{
    uint32_t w = src->width(), h = src->height();
    for (uint32_t y = 0; y < dh; y += h)
    {
        for (uint32_t x = 0; x < dw; x += w)
        {
            pCanvas->drawImage(src, static_cast<SkScalar>(x), static_cast<SkScalar>(y));
        }
    }
}

void adapt_image_scale(SkCanvas *pCanvas, const sk_sp<SkImage>& src, uint32_t dw, uint32_t dh,
                       const SkSamplingOptions& sampler)
{
    pCanvas->drawImageRect(src,
                           SkRect::MakeWH(static_cast<SkScalar>(dw), static_cast<SkScalar>(dh)),
                           sampler);
}

sk_sp<SkImage> adapt_image(const sk_sp<SkImage>& src,
                           uint32_t dstW, uint32_t dstH,
                           SkColorType dstColorType,
                           ImageAdaptationMethod method,
                           SkColor fillColor,
                           const SkSamplingOptions& sampler)
{
    SkImageInfo info = SkImageInfo::Make(static_cast<int32_t>(dstW),
                                         static_cast<int32_t>(dstH),
                                         dstColorType,
                                         SkAlphaType::kPremul_SkAlphaType);
    sk_sp<SkSurface> surface = SkSurface::MakeRaster(info);
    SkCanvas *pCanvas = surface->getCanvas();

    pCanvas->clear(fillColor);
    switch (method)
    {
    case ImageAdaptationMethod::kRepeatX:
        adapt_image_repeat_x(pCanvas, src, dstW);
        break;
    case ImageAdaptationMethod::kRepeatY:
        adapt_image_repeat_y(pCanvas, src, dstH);
        break;
    case ImageAdaptationMethod::kRepeatXY:
        adapt_image_repeat_xy(pCanvas, src, dstW, dstH);
        break;
    case ImageAdaptationMethod::kScale:
        adapt_image_scale(pCanvas, src, dstW, dstH, sampler);
        break;
    case ImageAdaptationMethod::kNone:
        pCanvas->drawImage(src, 0, 0);
        break;
    }

    return surface->makeImageSnapshot();
}

} // namespace anonymous

void ImageLayer::upload(const Handle<Data>& data)
{
    size_t dataSize = data->size();
    auto pDataBuffer = new uint8_t[dataSize];
    assert(pDataBuffer);
    ScopeEpilogue epilogue([pDataBuffer]() -> void {
        delete[] pDataBuffer;
    });
    if (data->read(pDataBuffer, dataSize) <= 0)
    {
        throw VanillaException(__func__, "Failed to read texture source");
    }

    sk_sp<SkImage> localImage = SkImage::MakeFromEncoded(SkData::MakeWithoutCopy(pDataBuffer, dataSize));
    if (!localImage)
    {
        throw VanillaException(__func__, "Failed to decode image data");
    }

    auto [w, h] = Layer::getProperties()->getDimension();
    SkColorType type = Layer::getContentAggregator()->getScreenColorType();
    sk_sp<SkImage> adaptedImage = adapt_image(localImage, w, h, type, fAdaptation,
                                              fBGFillColor, fScaleSamplingOptions);
    if (!adaptedImage)
    {
        throw VanillaException(__func__, "Failed to adapt image by given method");
    }

    /**
     * The @p adaptedImage will be converted to GPU-backed if @p fDirectContext
     * is not nullptr.
     * Assigning @p fImage to @p adaptedImage directly if backend is Raster can
     * avoid unnecessary copying.
     */
    if (fDirectContext)
    {
        fImage = adaptedImage->makeSubset(adaptedImage->bounds(), fDirectContext);
    }
    else
    {
        fImage = adaptedImage;
    }
}

void ImageLayer::onPreComposite()
{
}

void ImageLayer::onPostComposite()
{
}

sk_sp<SkImage> ImageLayer::onGetImage(const SkIRect& bounds)
{
    if (!fImage)
        return nullptr;
    if (bounds == fImage->bounds())
        return fImage;
    return fImage->makeSubset(bounds, fDirectContext);
}

void ImageLayer::onDispose()
{
    fImage = nullptr;
    fDirectContext = nullptr;
}

VANILLA_NS_END
