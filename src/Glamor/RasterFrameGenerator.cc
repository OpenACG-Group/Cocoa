#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"

#include "Glamor/RasterFrameGenerator.h"
GLAMOR_NAMESPACE_BEGIN

RasterFrameGenerator::RasterFrameGenerator(const Shared<Blender>& blender)
    : FrameGeneratorBase(blender)
{
}

RasterFrameGenerator::~RasterFrameGenerator() = default;

void RasterFrameGenerator::OnPaint(SkSurface *surface, const sk_sp<SkPicture>& picture,
                                   const SkIRect& rect)
{
    SkCanvas *canvas = surface->getCanvas();
    SkAutoCanvasRestore autoRestore(canvas, true);

    canvas->clipIRect(rect);
    picture->playback(canvas);
}

GLAMOR_NAMESPACE_END
