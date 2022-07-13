#ifndef COCOA_GLAMOR_RASTERFRAMEGENERATOR_H
#define COCOA_GLAMOR_RASTERFRAMEGENERATOR_H

#include "Glamor/Glamor.h"
#include "Glamor/FrameGeneratorBase.h"
GLAMOR_NAMESPACE_BEGIN

class RasterFrameGenerator : public FrameGeneratorBase
{
public:
    explicit RasterFrameGenerator(const Shared<Blender>& blender);
    ~RasterFrameGenerator() override;

    void OnPaint(SkSurface *surface, const sk_sp<SkPicture> &picture,
                 const SkIRect &rect) override;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_RASTERFRAMEGENERATOR_H
