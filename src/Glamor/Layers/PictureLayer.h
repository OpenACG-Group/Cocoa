#ifndef COCOA_GLAMOR_LAYERS_PICTURELAYER_H
#define COCOA_GLAMOR_LAYERS_PICTURELAYER_H

#include "include/core/SkPicture.h"

#include "Glamor/Layers/Layer.h"
#include "Glamor/MaybeGpuObject.h"
GLAMOR_NAMESPACE_BEGIN

class PictureLayer : public Layer
{
public:
    PictureLayer(const SkPoint& offset, const MaybeGpuObject<SkPicture>& picture);
    ~PictureLayer() override = default;

    void Preroll(PrerollContext *context, const SkMatrix &matrix) override;
    void Paint(PaintContext *context) const override;

private:
    // Although `SkPicture` itself is not a GPU object, it may contain references
    // to other GPU objects.
    MaybeGpuObject<SkPicture> sk_picture_;

    // top-left corner of picture in parent's coordinate
    SkPoint offset_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_LAYERS_PICTURELAYER_H
