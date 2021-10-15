#ifndef COCOA_PICTURELAYER_H
#define COCOA_PICTURELAYER_H

#include <mutex>

#include "include/core/SkPicture.h"
#include "include/core/SkSurface.h"
#include "include/core/SkImage.h"
#include "include/core/SkPictureRecorder.h"

#include "Vanilla/Base.h"
#include "Vanilla/RenderKit/Layer.h"
VANILLA_NS_BEGIN

class ContentAggregator;

class PictureLayer : public Layer
{
public:
    T_LAYER_OBJECT(PictureLayer)

    using DrawCallback = std::function<void(SkCanvas*)>;

    PictureLayer(const WeakHandle<ContentAggregator>& aggregator,
                 bool opaque, uint32_t x, uint32_t y, uint32_t w, uint32_t h);
    ~PictureLayer() override = default;

    void requestResources();

    void paint(const DrawCallback& painter);
    void activate();

private:
    void onDispose() override;
    void onPostComposite() override;
    void onPreComposite() override;
    sk_sp<SkImage> onGetImage(const SkIRect& bounds) override;

private:
    sk_sp<SkPicture>    fLastPicture;
    sk_sp<SkSurface>    fBackendSurface;
    sk_sp<SkImage>      fCachedImageSnapshot;
    SkIRect             fCachedImageSnapshotBounds;
    std::mutex          fUpdatePictureMutex;
};

VANILLA_NS_END
#endif //COCOA_PICTURELAYER_H
