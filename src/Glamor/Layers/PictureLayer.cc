#include "Glamor/Layers/PictureLayer.h"
GLAMOR_NAMESPACE_BEGIN

PictureLayer::PictureLayer(const SkPoint& offset, const MaybeGpuObject<SkPicture>& picture)
    : sk_picture_(picture)
    , offset_(offset)
{
}

void PictureLayer::Preroll(PrerollContext *context, const SkMatrix& matrix)
{
    SkRect bounds = sk_picture_->cullRect().makeOffset(offset_.x(), offset_.y());
    SetPaintBounds(bounds);
}

void PictureLayer::Paint(PaintContext *context) const
{
    SkAutoCanvasRestore scopedRestore(context->composed_canvas, true);
    context->composed_canvas->translate(offset_.x(), offset_.y());

    sk_picture_->playback(context->composed_canvas);
}

GLAMOR_NAMESPACE_END
