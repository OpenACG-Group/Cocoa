#include "Core/Errors.h"

#include "include/core/SkCanvas.h"

#include "Core/Exception.h"
#include "Core/Journal.h"
#include "Vanilla/Base.h"
#include "Vanilla/DrawContext.h"
#include "Vanilla/RenderKit/ContentAggregator.h"
#include "Vanilla/RenderKit/Layer.h"
#include "Vanilla/RenderKit/LayerFactories.h"
VANILLA_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Vanilla.RenderKit)

Handle<ContentAggregator> ContentAggregator::Make(const Handle<DrawContext>& ctx)
{
    CHECK(ctx);
    return std::make_shared<ContentAggregator>(ctx);
}

ContentAggregator::ContentAggregator(Handle<DrawContext> ctx)
    : fDisposed(false)
    , fDrawContext(std::move(ctx))
    , fBGFillColor(SK_ColorBLACK)
    , fLastUpdateTp(std::chrono::steady_clock::now())
    , fFrameCounter(0)
    , fFps(0)
{
}

ContentAggregator::~ContentAggregator()
{
    CHECK(fDisposed && "ContentAggregator should be disposed before destructing");
}

SkColorType ContentAggregator::getScreenColorType()
{
    return fDrawContext->getColorType();
}

Handle<Layer> ContentAggregator::getLayerById(uint32_t id)
{
    for (auto& layer : fLayerList)
    {
        if (layer->getLayerId() == id)
            return layer;
    }
    return nullptr;
}

sk_sp<SkSurface> ContentAggregator::requestBackendSurface(const Handle<Layer>& layer,
                                                          const SkImageInfo& info, SkBudgeted budgeted)
{
    if (info.isEmpty() || info.width() <= 0 || info.height() <= 0)
        return nullptr;
    if (std::find(fLayerList.begin(), fLayerList.end(), layer) == fLayerList.end())
        return nullptr;

    sk_sp<SkSurface> surface = fDrawContext->createBackendSurface(info, budgeted);
    if (!surface)
        return nullptr;
    fLayerOwnedSurfacesMap[surface] = layer->getLayerId();
    return surface;
}

void ContentAggregator::releaseBackendSurface(const Handle<Layer>& layer,
                                              const sk_sp<SkSurface>& surface)
{
    if (std::find(fLayerList.begin(), fLayerList.end(), layer) == fLayerList.end())
        return;
    if (!fLayerOwnedSurfacesMap.contains(surface))
        return;

    if (fLayerOwnedSurfacesMap[surface] != layer->getLayerId())
    {
        throw VanillaException(__func__,
                               "A layer must not release a surface which is not owned by itself");
    }
    fLayerOwnedSurfacesMap.erase(surface);
}

void ContentAggregator::update(const SkRect& region)
{
    for (auto& layer : fLayerList)
        layer->onPreComposite();

    DrawContext::DrawScope drawScope(fDrawContext);
    DrawContext::PresentationScope presentScope(fDrawContext, region);

    sk_sp<SkSurface> surface = presentScope.surface();
    SkCanvas *pCanvas = surface->getCanvas();
    uint32_t screenWidth = surface->width(), screenHeight = surface->height();

    /* Clear our screen first */
    pCanvas->clear(fBGFillColor);

    for (auto& layer : fLayerList)
    {
        SkAutoCanvasRestore savedScope(pCanvas, true);
        UniqueHandle<LayerPropertiesGroup>& prop = layer->getProperties();
        auto [x, y] = prop->getPosition();
        auto [w, h] = prop->getDimension();

        if (x >= screenWidth || y >= screenHeight)
            continue;

        switch (prop->getClippingType())
        {
        case LayerPropertiesGroup::Clipping::kRect:
            if (!prop->getClipRect().isEmpty())
                pCanvas->clipRect(prop->getClipRect(), prop->getClipOp(), prop->isClipAA());
            break;
        case LayerPropertiesGroup::Clipping::kRRect:
            if (!prop->getClipRRect().isEmpty() && prop->getClipRRect().isValid())
                pCanvas->clipRRect(prop->getClipRRect(), prop->getClipOp(), prop->isClipAA());
            break;
        case LayerPropertiesGroup::Clipping::kPath:
            if (!prop->getClipPath().isEmpty() && prop->getClipPath().isValid())
                pCanvas->clipPath(prop->getClipPath(), prop->getClipOp(), prop->isClipAA());
            break;
        case LayerPropertiesGroup::Clipping::kNone:
            break;
        }

        SkPaint paint;
        if (prop->hasMatrix() && prop->getMatrix().isFinite())
        {
            pCanvas->setMatrix(prop->getMatrix());
            paint.setAntiAlias(prop->isMatrixAA());
        }

        paint.setImageFilter(prop->getImageFilter());
        if (prop->getShader())
        {
            paint.setShader(prop->getShader());
        }
        else
        {
            paint.setColorFilter(prop->getColorFilter());
        }

        // TODO: Processing detailed effects

        if (prop->isOpaque())
        {
            paint.setBlendMode(SkBlendMode::kSrc);
        }
        else
        {
            paint.setBlendMode(SkBlendMode::kSrcATop);
            paint.setAlphaf(prop->getAlphaValue());
        }

        sk_sp<SkImage> image = layer->onGetImage(SkIRect::MakeWH(static_cast<int32_t>(w), static_cast<int32_t>(h)));
        if (!image)
        {
            QLOG(LOG_ERROR, "Layer #{} can not produce any image to ContentAggregator", layer->getLayerId());
            continue;
        }

        SkSamplingOptions samplingOptions;
        if (fForceSamplingOptions.has_value())
        {
            samplingOptions = fForceSamplingOptions.value();
        }
        else
        {
            samplingOptions = prop->getSamplingOptions();
        }

        pCanvas->drawImage(image, static_cast<SkScalar>(x), static_cast<SkScalar>(y), samplingOptions, &paint);
    }

    ++fFrameCounter;
    auto currentTp = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(currentTp - fLastUpdateTp).count();
    if (duration >= 1000)
    {
        fFps = float(fFrameCounter) / (duration / 1000.0f);
        fLastUpdateTp = currentTp;
        fFrameCounter = 0;
    }

    for (auto& layer : fLayerList)
        layer->onPostComposite();
}

void ContentAggregator::dispose()
{
    if (fDisposed)
        return;

    fDisposed = true;
    for (auto& layer : fLayerList)
        layer->dispose();

    if (!fLayerOwnedSurfacesMap.empty())
    {
        for (const auto& pair : fLayerOwnedSurfacesMap)
        {
            QLOG(LOG_WARNING, "Resource leaking: SkSurface@{} (owned by layer #{}), after: destructing ContentAggregator@{}",
                 fmt::ptr(pair.first.get()), pair.second, fmt::ptr(this));
        }

        QLOG(LOG_EXCEPTION, "Resource leaking (SkSurface, by ContentAggregator)");
        std::abort();
    }
}

Handle<Layer> ContentAggregator::pushLayer(const LayerFactory& factory)
{
    Handle<Layer> layer = factory.create(shared_from_this());
    if (layer == nullptr)
    {
        return nullptr;
    }
    fLayerList.push_back(layer);
    return layer;
}

Handle<Layer> ContentAggregator::insertLayerBefore(uint32_t id, const LayerFactory& factory)
{
    auto maybeBeforeLayer = std::find_if(fLayerList.begin(), fLayerList.end(), [id](const Handle<Layer>& layer) {
        if (layer->getLayerId() == id)
            return true;
        return false;
    });
    if (maybeBeforeLayer == fLayerList.end())
    {
        throw VanillaException(__func__, fmt::format("No such layer specified by id #{}", id));
    }

    Handle<Layer> newLayer = factory.create(shared_from_this());
    if (!newLayer)
    {
        throw VanillaException(__func__, "Failed to create a new layer from factory");
    }
    fLayerList.insert(maybeBeforeLayer, newLayer);
    return newLayer;
}

void ContentAggregator::removeLayer(const Handle<Layer>& layer)
{
    auto maybeLayer = std::find(fLayerList.begin(), fLayerList.end(), layer);
    if (maybeLayer == fLayerList.end())
    {
        throw VanillaException(__func__, "No such layer");
    }

    (*maybeLayer)->dispose();
    fLayerList.erase(maybeLayer);
}

VANILLA_NS_END
