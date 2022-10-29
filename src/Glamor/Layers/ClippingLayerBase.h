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

#ifndef COCOA_GLAMOR_LAYERS_CLIPPINGLAYERBASE_H
#define COCOA_GLAMOR_LAYERS_CLIPPINGLAYERBASE_H

#include "include/core/SkClipOp.h"
#include "Glamor/Layers/ContainerLayer.h"
GLAMOR_NAMESPACE_BEGIN

template<typename T>
class ClippingLayerBase : public ContainerLayer
{
public:
    using ClipShape = T;

    explicit ClippingLayerBase(ClipShape shape)
        : clip_shape_(std::move(shape)) {}
    ~ClippingLayerBase() override = default;

    void Preroll(PrerollContext *context, const SkMatrix& matrix) override
    {
        SkRect previous_cull = context->cull_rect;
        SkRect clip_shape_bounds = OnGetClipShapeBounds();

        if (!context->cull_rect.intersect(clip_shape_bounds))
        {
            // The clipping bounds doesn't intersect with current cull rectangle
            context->cull_rect.setEmpty();
        }

        SkRect child_paint_bounds = SkRect::MakeEmpty();
        PrerollChildren(context, matrix, &child_paint_bounds);
        if (child_paint_bounds.intersect(clip_shape_bounds))
        {
            // There is no need to paint children if they are completely
            // outside the clipping shape bounds.
            SetPaintBounds(child_paint_bounds);
        }

        // Restore cull rectangle
        context->cull_rect = previous_cull;
    }

    void Paint(PaintContext *context) const override
    {
        SkCanvas *canvas = context->multiplexer_canvas;
        SkAutoCanvasRestore scoped_restore(canvas, true);
        OnApplyClipShape(clip_shape_, context);

        PaintChildren(context);
    }

protected:
    virtual void OnApplyClipShape(const ClipShape& shape, PaintContext *ctx) const = 0;
    g_nodiscard virtual SkRect OnGetClipShapeBounds() const = 0;

    g_nodiscard const ClipShape& GetClipShape() const {
        return clip_shape_;
    }

private:
    ClipShape   clip_shape_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_LAYERS_CLIPPINGLAYERBASE_H
