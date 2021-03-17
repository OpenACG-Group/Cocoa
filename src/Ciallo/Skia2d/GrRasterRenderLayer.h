#ifndef COCOA_GRRASTERRENDERLAYER_H
#define COCOA_GRRASTERRENDERLAYER_H

#include <memory>

#include "include/core/SkImageInfo.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"

#include "Ciallo/Ciallo.h"
#include "Ciallo/Skia2d/GrBaseRenderLayer.h"
CIALLO_BEGIN_NS

class GrRasterRenderLayer : public GrBaseRenderLayer
{
public:
    GrRasterRenderLayer(GrBaseCompositor *compositor,
                        int32_t x,
                        int32_t y,
                        int32_t z,
                        int32_t w,
                        int32_t h,
                        const SkImageInfo& imageInfo);
    ~GrRasterRenderLayer() override = default;

private:
    sk_sp<SkImage> onLayerResult() override;
    SkCanvas *onCreateCanvas() override;

private:
    SkImageInfo                 fImageInfo;
    sk_sp<SkSurface>            fSurface;
};

CIALLO_END_NS
#endif //COCOA_GRRASTERRENDERLAYER_H
