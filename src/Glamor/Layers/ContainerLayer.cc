#include "Glamor/Layers/ContainerLayer.h"
GLAMOR_NAMESPACE_BEGIN

void ContainerLayer::Preroll(PrerollContext *context, const SkMatrix& matrix)
{
    SkRect paint_bound = SkRect::MakeEmpty();
    PrerollChildren(context, matrix, &paint_bound);
    SetPaintBounds(paint_bound);
}

void ContainerLayer::PrerollChildren(PrerollContext *context,
                                     const SkMatrix& matrix,
                                     SkRect *child_paint_bounds)
{
    // Iterate each child layer and reroll them respectively
    for (const Shared<Layer>& layer : child_layers_)
    {
        // ContainerLayer doesn't have any transformations, so applying `matrix` directly
        // to child layer is reasonable.
        layer->Preroll(context, matrix);

        // The dirty boundary of a ContainerLayer is just the union of all its
        // children's dirty boundaries.
        child_paint_bounds->join(layer->GetPaintBounds());
    }
}

void ContainerLayer::Paint(PaintContext *context) const
{
    PaintChildren(context);
}

void ContainerLayer::PaintChildren(PaintContext *context) const
{
    // Iterate each child layer and paint them respectively
    for (const Shared<Layer>& layer : child_layers_)
    {
        if (layer->NeedsPainting(context))
            layer->Paint(context);
    }
}

GLAMOR_NAMESPACE_END
