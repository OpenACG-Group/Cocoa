#ifndef COCOA_IMAGELAYER_H
#define COCOA_IMAGELAYER_H

#include "Core/Data.h"
#include "Vanilla/Base.h"
#include "Vanilla/RenderKit/Layer.h"
#include "Vanilla/RenderKit/ImageAdaptationMethod.h"

class GrDirectContext;

VANILLA_NS_BEGIN

/**
 * @p ImageLayer is a container which stores pixels and the corresponding
 * information.
 * Where to store pixels (system memory or GPU memory) is up to the backend.
 * - Stores in system memory when Raster backend is used.
 * - Stores in GPU memory when Vulkan backend is used (texture uploading).
 */
class ImageLayer : public Layer
{
public:
    T_LAYER_OBJECT(ImageLayer)

    ImageLayer(ImageAdaptationMethod adaptation,
               const WeakHandle<ContentAggregator>& aggregator, bool opaque,
               uint32_t x, uint32_t y, uint32_t w, uint32_t h);
    ~ImageLayer() override = default;

    /* Nearest or liner sampling */
    inline void setScaleSampling(SkFilterMode mode) {
        fScaleSamplingOptions = SkSamplingOptions(mode, SkMipmapMode::kNone);
    }

    /* Cubic sampling */
    inline void setScaleSampling(float B, float C) {
        auto cubicResampler = SkCubicResampler{B, C};
        fScaleSamplingOptions = SkSamplingOptions(cubicResampler);
    }

    inline void setFillColor(SkColor color) {
        fBGFillColor = color;
    }

    inline void setAdaptation(ImageAdaptationMethod method) {
        fAdaptation = method;
    }

    void upload(const Handle<Data>& data);

private:
    void onPreComposite() override;
    void onPostComposite() override;
    sk_sp<SkImage> onGetImage(const SkIRect &bounds) override;
    void onDispose() override;

    sk_sp<SkImage>          fImage;
    ImageAdaptationMethod   fAdaptation;
    GrDirectContext        *fDirectContext;
    SkColor                 fBGFillColor;
    SkSamplingOptions       fScaleSamplingOptions;
};

VANILLA_NS_END
#endif //COCOA_IMAGELAYER_H
