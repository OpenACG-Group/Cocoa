/**
 * This file is part of Cocoa.
 *
 * Cocoa is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * Cocoa is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cocoa. If not, see <https://www.gnu.org/licenses/>.
 */

#include "Glamor/Layers/BackdropFilterLayer.h"
GLAMOR_NAMESPACE_BEGIN

BackdropFilterLayer::BackdropFilterLayer(const sk_sp<SkImageFilter>& filter,
                                         SkBlendMode blend_mode, bool auto_child_clip)
    : auto_child_clip_(auto_child_clip)
    , image_filter_(filter)
    , blend_mode_(blend_mode)
{
    CHECK(image_filter_);
}

void BackdropFilterLayer::Preroll(PrerollContext *context, const SkMatrix& matrix)
{
    SkRect child_paint_bounds = SkRect::MakeEmpty();
    PrerollChildren(context, matrix, &child_paint_bounds);

    // Compared with ImageFilterLayer, BackdropFilterLayer applies the
    // image filter and blend mode to the background contents instead
    // of the layer contents itself. There is no need to use
    // `SkImageFilter::filterBounds` to calculate the transformed bounds
    // by image filter.
    child_paint_bounds.join(context->cull_rect);
    SetPaintBounds(child_paint_bounds);
}

void BackdropFilterLayer::Paint(PaintContext *context) const
{
    SkCanvas *canvas = context->multiplexer_canvas;
    CHECK(canvas);

    SkAutoCanvasRestore auto_restore(canvas, true);

    // Graphics state stored in `PaintContext::paints_stack` can be overwritten
    // (consider there are multiple container layers linked serially, and each of
    // them will change the graphics states stored in the paints stack),
    // so we must use `SkCanvas::saveLayer` to apply the blend mode settings.
    {
        ScopedPaintMutator mutator(context, [this](SkPaint& paint) {
            paint.setBlendMode(this->blend_mode_);
        });

        SkRect child_paint_bounds = GetPaintBounds();
        if (auto_child_clip_)
            canvas->clipRect(child_paint_bounds);

        SkCanvas::SaveLayerRec save_layer_rec(&child_paint_bounds,
                                              context->GetCurrentPaintPtr(),
                                              image_filter_.get(),
                                              SkCanvas::kInitWithPrevious_SaveLayerFlag);
        canvas->saveLayer(save_layer_rec);
    }

    PaintChildren(context);

    // Restore the `saveLayer`
    canvas->restore();
}

GLAMOR_NAMESPACE_END
