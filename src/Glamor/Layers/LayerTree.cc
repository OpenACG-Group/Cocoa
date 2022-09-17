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

#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/utils/SkNWayCanvas.h"

#include "Core/Journal.h"
#include "Glamor/Layers/LayerTree.h"
#include "Glamor/Layers/ContainerLayer.h"
GLAMOR_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Glamor.Layers.LayerTree)

LayerTree::LayerTree(const SkISize& frameSize)
    : frame_size_(frameSize)
{
}

LayerTree::~LayerTree() = default;

bool LayerTree::Preroll(Layer::PrerollContext *context)
{
    if (!root_layer_)
    {
        QLOG(LOG_ERROR, "No available layer tree to preroll");
        return false;
    }

    root_layer_->Preroll(context, context->root_surface_transformation);
    context->cull_rect = root_layer_->GetPaintBounds();

    return true;
}

void LayerTree::Paint(Layer::PaintContext *context)
{
    if (!root_layer_)
        return;

    // In the wayland CPU backend, Wayland compositor supports to submit a pixel
    // buffer with a certain "damage region" which indicates the dirty region
    // that should be updated. However, the HWCompose implementation does not
    // support that yet, so we do an explicit clipping here.
    context->multiplexer_canvas->clipRect(context->cull_rect);

    root_layer_->Paint(context);
}

MaybeGpuObject<SkPicture> LayerTree::Flatten(const SkRect& bounds)
{
    // TODO(sora): implement this.

#if 0
    if (root_layer_ == nullptr)
        return nullptr;

    SkMatrix root_surface_transformation;
    // Identity matrix
    root_surface_transformation.reset();

    SkPictureRecorder recorder;
    SkCanvas *canvas = recorder.beginRecording(bounds);

    if (!canvas)
        return nullptr;

    SkISize canvas_size = canvas->getBaseLayerSize();
    SkNWayCanvas composed_canvas(canvas_size.width(), canvas_size.height());
    composed_canvas.addCanvas(canvas);

    Layer::PrerollContext prerollContext {
        .gr_context = nullptr,
        .root_surface_transformation = root_surface_transformation,
        .cull_rect = bounds
    };

    Layer::PaintContext paintContext {
        .gr_context = nullptr,
        .root_surface_transformation = root_surface_transformation,
        .frame_canvas = canvas,
        .composed_canvas = &composed_canvas,
        .cull_rect = bounds,
        .texture_manager = ,
        .paint = nullptr,
        .has_gpu_retained_resource = false
    };

    root_layer_->Preroll(&prerollContext, root_surface_transformation);
    if (root_layer_->NeedsPainting(&paintContext))
    {
        root_layer_->Paint(&paintContext);
    }

    sk_sp<SkPicture> picture = recorder.finishRecordingAsPicture();
    return {paintContext.has_gpu_retained_resource, picture};
#endif

    return nullptr;
}

GLAMOR_NAMESPACE_END
