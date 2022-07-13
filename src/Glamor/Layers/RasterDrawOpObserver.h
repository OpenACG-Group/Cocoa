#ifndef COCOA_GLAMOR_LAYERS_RASTERDRAWOPOBSERVER_H
#define COCOA_GLAMOR_LAYERS_RASTERDRAWOPOBSERVER_H

#include "include/utils/SkNoDrawCanvas.h"

#include "Glamor/Glamor.h"
GLAMOR_NAMESPACE_BEGIN

class RasterDrawOpObserver : public SkCanvasVirtualEnforcer<SkNoDrawCanvas>
{
public:
    RasterDrawOpObserver(int32_t width, int32_t height);
    explicit RasterDrawOpObserver(const SkIRect& cull);
    ~RasterDrawOpObserver() override = default;

    virtual void BeginFrame() = 0;
    virtual void EndFrame() = 0;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_LAYERS_RASTERDRAWOPOBSERVER_H
