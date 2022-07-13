#ifndef COCOA_GLAMOR_LAYERS_TRANSFORMLAYER_H
#define COCOA_GLAMOR_LAYERS_TRANSFORMLAYER_H

#include "Glamor/Glamor.h"
#include "Glamor/Layers/ContainerLayer.h"
GLAMOR_NAMESPACE_BEGIN

class TransformLayer : public ContainerLayer
{
public:
    explicit TransformLayer(const SkMatrix& transform);
    ~TransformLayer() override = default;

    void Preroll(PrerollContext *context, const SkMatrix &matrix) override;

    void Paint(PaintContext *context) const override;

private:
    SkMatrix transform_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_LAYERS_TRANSFORMLAYER_H
