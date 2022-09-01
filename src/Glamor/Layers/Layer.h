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

#ifndef COCOA_GLAMOR_LAYERS_LAYER_H
#define COCOA_GLAMOR_LAYERS_LAYER_H

#include "include/gpu/GrDirectContext.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkRect.h"
#include "include/core/SkMatrix.h"

#include "Glamor/Glamor.h"
GLAMOR_NAMESPACE_BEGIN

// This rectangle indicates the concept of "infinity".
// For example, an infinite clipping approximately means no clipping is applied on the canvas.
static constexpr SkRect kGiantRect = SkRect::MakeLTRB(-1E9F, -1E9F, 1E9F, 1E9F);

class TextureManager;

class Layer
{
public:
    CO_NONCOPYABLE(Layer)
    CO_NONASSIGNABLE(Layer)

    // NOLINTNEXTLINE
    struct PrerollContext
    {
        GrDirectContext *gr_context;

        SkMatrix root_surface_transformation;

        // Calculated when we are prerolling the layer tree and will be available
        // after finishing prerolling.
        SkRect cull_rect;
    };

    // NOLINTNEXTLINE
    struct PaintContext
    {
        GrDirectContext *gr_context;
        SkMatrix root_surface_transformation;

        // canvas which is got from backend surface directly.
        SkCanvas *frame_canvas;

        // A SkNWayCanvas which may contain other canvases like drawing operations
        // analyzer, recorder, and redirector.
        SkCanvas *composed_canvas;

        // An exact copy of `PrerollContext::cull_rect`
        SkRect cull_rect;

        Unique<TextureManager>& texture_manager;

        SkPaint *paint;

        // Layers should set this if any GPU retained resources was drawn into
        // canvas. For example, a `SkImage` object which holds GPU texture.
        bool has_gpu_retained_resource;
    };

    Layer();
    virtual ~Layer() = default;

    g_nodiscard g_inline const SkRect& GetPaintBounds() const {
        return paint_bounds_;
    }

    // This should be set for each layer when Blender is prerolling the layer tree,
    // otherwise it will stay empty.
    g_inline void SetPaintBounds(const SkRect& bounds) {
        paint_bounds_ = bounds;
    }

    g_nodiscard g_inline uint32_t GetUniqueId() const {
        return unique_id_;
    }

    // Determine if the `Paint` method is necessary for this layer according to
    // the `paint_bound_` and properties in `PaintContext`.
    g_inline bool NeedsPainting(PaintContext *context) const {
        // Workaround for Skia bug (quickReject does not reject empty bounds).
        // https://bugs.chromium.org/p/skia/issues/detail?id=10951
        if (paint_bounds_.isEmpty())
        {
            return false;
        }
        return !context->frame_canvas->quickReject(paint_bounds_);
    }

    // The rasterization process is always split into two stages,
    // the first one of which is called "Preroll", while the second one is called "Paint".
    // In the "Preroll" stage, all the layer nodes will be accessed in preorder traversal.
    // They are supposed to calculate the dirty boundary of themselves and generate
    // `RasterCacheEntry` optionally.
    virtual void Preroll(PrerollContext *context, const SkMatrix& matrix);

    virtual void Paint(PaintContext *context) const = 0;

private:
    SkRect              paint_bounds_;
    uint32_t            unique_id_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_LAYERS_LAYER_H
