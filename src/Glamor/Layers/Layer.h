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

#include <stack>
#include <utility>
#include <sstream>

#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrBackendSemaphore.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkRect.h"
#include "include/core/SkMatrix.h"

#include "Core/Errors.h"
#include "Glamor/Glamor.h"
GLAMOR_NAMESPACE_BEGIN

// This rectangle indicates the concept of "infinity".
// For example, an infinite clipping approximately means no clipping is applied on the canvas.
static constexpr SkRect kGiantRect = SkRect::MakeLTRB(-1E9F, -1E9F, 1E9F, 1E9F);

class LayerGenerationCache;
class HWComposeSwapchain;
class ContentAggregator;

class Layer
{
public:
    CO_NONCOPYABLE(Layer)
    CO_NONASSIGNABLE(Layer)

    enum class Type
    {
        kContainer,
        kExternalTexture,
        kPicture,
        kGpuSurfaceView
    };

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
        enum ResourceUsageFlags
        {
            kNone_ResourceUsage = 0,
            kExternalTexture_ResourceUsage = 0x01,
            kOffscreenTexture_ResourceUsage = 0x02,
            kGpu_ResourceUsage = 0x04
        };

        GrDirectContext *gr_context;

        bool is_generating_cache;

        SkMatrix root_surface_transformation;

        SkSurface *frame_surface;

        // canvas which is got from backend surface directly.
        SkCanvas *frame_canvas;

        // A SkNWayCanvas which may contain other canvases like drawing operations
        // analyzer, recorder, and redirector.
        SkCanvas *multiplexer_canvas;

        // An exact copy of `PrerollContext::cull_rect`
        SkRect cull_rect;

        std::stack<SkPaint> paints_stack;

        uint32_t resource_usage_flags;

        LayerGenerationCache *cache;

        ContentAggregator *content_aggregator;

        // Layers can set this to let Skia signal the specified semaphores
        // when all the commands in this frame submitted to GPU are finished.
        // Semaphores must be created by, or be imported from other contexts to,
        // the PresentThread's GPU context.
        // When raster backend is used, this is ignored.
        std::vector<GrBackendSemaphore> gpu_finished_semaphores;

        g_nodiscard bool HasCurrentPaint() const {
            return !paints_stack.empty();
        }

        g_nodiscard SkPaint& GetCurrentPaint() {
            CHECK(!paints_stack.empty());
            return paints_stack.top();
        }

        g_nodiscard SkPaint *GetCurrentPaintPtr() {
            if (paints_stack.empty())
                return nullptr;
            return &paints_stack.top();
        }
    };

    // Create a new SkPaint object in the `PaintContext::paints_stack` stack
    // (or copy from an existing one). User can mutate it in the `mutating_callback`
    // function. The mutated new `SkPaint` will be pushed into the stack and
    // will be popped out automatically when destruction.
    class ScopedPaintMutator
    {
    public:
        using Mutator = std::function<void(SkPaint&)>;
        explicit ScopedPaintMutator(PaintContext *ctx, const Mutator& mutating_callback)
            : paint_context_(ctx)
        {
            CHECK(mutating_callback);

            std::stack<SkPaint>& stack = paint_context_->paints_stack;
            SkPaint *paint;
            if (stack.empty())
                paint = &stack.emplace();
            else
                paint = &stack.emplace(stack.top());

            mutating_callback(*paint);
        }

        ~ScopedPaintMutator()
        {
            paint_context_->paints_stack.pop();
        }

    private:
        PaintContext   *paint_context_;
    };

    explicit Layer(Type type);
    virtual ~Layer() = default;

    g_nodiscard Type GetType() const {
        return layer_type_;
    }

    // Generation ID increases when the node is updated.
    g_nodiscard uint64_t GetGenerationId() const {
        return generation_id_;
    }

    g_nodiscard const SkRect& GetPaintBounds() const {
        return paint_bounds_;
    }

    // This should be set for each layer when ContentAggregator is
    // prerolling the layer tree, otherwise it will stay empty.
    void SetPaintBounds(const SkRect& bounds) {
        paint_bounds_ = bounds;
    }

    g_nodiscard uint32_t GetUniqueId() const {
        return unique_id_;
    }

    // Determine if the `Paint` method is necessary for this layer according to
    // the `paint_bound_` and properties in `PaintContext`.
    g_nodiscard bool NeedsPainting(PaintContext *context) const {
        // Workaround for Skia bug (quickReject does not reject empty bounds).
        // https://bugs.chromium.org/p/skia/issues/detail?id=10951
        if (paint_bounds_.isEmpty())
        {
            return false;
        }
        return !context->frame_canvas->quickReject(paint_bounds_);
    }

    g_nodiscard virtual bool IsComparableWith(Layer *other) const;

    // The rasterization process is always split into two stages,
    // the first one of which is called "Preroll", while the second one is called "Paint".
    // In the "Preroll" stage, all the layer nodes will be accessed in preorder traversal.
    // They are supposed to calculate the dirty boundary of themselves and generate
    // `RasterCacheEntry` optionally.
    virtual void Preroll(PrerollContext *context, const SkMatrix& matrix);

    virtual void Paint(PaintContext *context) = 0;

    virtual void DiffUpdate(const std::shared_ptr<Layer>& other) = 0;

    virtual void ToString(std::ostream& out);

    virtual const char *GetLayerTypeName() = 0;

protected:
    uint64_t IncreaseGenerationId();

private:
    Type                layer_type_;
    SkRect              paint_bounds_;
    uint32_t            unique_id_;
    uint64_t            generation_id_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_LAYERS_LAYER_H
