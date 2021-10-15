#include <iostream>
#include "include/core/SkCanvas.h"

#include "Vanilla/Base.h"
#include "Vanilla/RenderKit/PictureLayer.h"
#include "Vanilla/RenderKit/ContentAggregator.h"
VANILLA_NS_BEGIN

PictureLayer::PictureLayer(const WeakHandle<ContentAggregator>& aggregator, bool opaque,
                           uint32_t x, uint32_t y, uint32_t w, uint32_t h)
    : Layer(aggregator, opaque, x, y, w, h)
    , fCachedImageSnapshotBounds(SkIRect::MakeEmpty())
{
}

void PictureLayer::requestResources()
{
    auto [w, h] = Layer::getProperties()->getDimension();
    SkImageInfo info = SkImageInfo::Make(static_cast<int>(w), static_cast<int>(h),
                                         Layer::getContentAggregator()->getScreenColorType(),
                                         SkAlphaType::kPremul_SkAlphaType);

    fBackendSurface = Layer::getContentAggregator()->requestBackendSurface(shared_from_this(),
                                                                           info,
                                                                           SkBudgeted::kYes);
    if (!fBackendSurface)
    {
        throw VanillaException(__func__, "Failed to create backend surface");
    }
}

void PictureLayer::onDispose()
{
    fLastPicture = nullptr;
    fCachedImageSnapshot = nullptr;
    if (fBackendSurface)
    {
        Layer::getContentAggregator()->releaseBackendSurface(shared_from_this(), fBackendSurface);
    }
}

void PictureLayer::onPostComposite()
{
}

void PictureLayer::onPreComposite()
{
}

sk_sp<SkImage> PictureLayer::onGetImage(const SkIRect& bounds)
{
    if (fCachedImageSnapshot && fCachedImageSnapshotBounds == bounds)
    {
        return fCachedImageSnapshot;
    }

    fCachedImageSnapshot = fBackendSurface->makeImageSnapshot(bounds);
    fCachedImageSnapshotBounds = bounds;
    return fCachedImageSnapshot;
}

void PictureLayer::paint(const DrawCallback& painter)
{
    SkPictureRecorder recorder;
    auto [w, h] = Layer::getProperties()->getDimension();
    SkCanvas *pCanvas = recorder.beginRecording(static_cast<SkScalar>(w), static_cast<SkScalar>(h));
    painter(pCanvas);
    sk_sp<SkPicture> pic = recorder.finishRecordingAsPicture();
    fUpdatePictureMutex.lock();
    fLastPicture = pic;
    fUpdatePictureMutex.unlock();
}

void PictureLayer::activate()
{
    fUpdatePictureMutex.lock();
    sk_sp<SkPicture> picture = fLastPicture;
    fUpdatePictureMutex.unlock();
    /* Where the rasterization occurs */
    if (picture)
    {
        picture->playback(fBackendSurface->getCanvas());
        fCachedImageSnapshot = nullptr;
    }
}

VANILLA_NS_END
