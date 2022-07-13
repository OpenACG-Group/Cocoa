#include "Glamor/Layers/RasterDrawOpObserver.h"
GLAMOR_NAMESPACE_BEGIN

RasterDrawOpObserver::RasterDrawOpObserver(int32_t width, int32_t height)
    : SkCanvasVirtualEnforcer<SkNoDrawCanvas>(width, height)
{
}

RasterDrawOpObserver::RasterDrawOpObserver(const SkIRect& cull)
    : SkCanvasVirtualEnforcer<SkNoDrawCanvas>(cull)
{
}

GLAMOR_NAMESPACE_END
