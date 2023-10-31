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

#include "include/gpu/MutableTextureState.h"

#include "Core/Journal.h"
#include "Glamor/ContentAggregator.h"
#include "Glamor/Layers/GpuSurfaceViewLayer.h"
GLAMOR_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Glamor.Layers.GpuSurfaceViewLayer)

using ContentTracker = GpuSurfaceViewLayer::ContentTracker;

ContentTracker::ContentTracker(sk_sp<SkSurface> surface)
    : surface_(std::move(surface))
    , last_track_point_(surface_->generationID())
    , has_changed_(true)
{
}

void ContentTracker::UpdateTrackPoint()
{
    uint32_t current_tp = surface_->generationID();
    has_changed_ = (current_tp != last_track_point_);
    last_track_point_ = current_tp;
}

bool ContentTracker::HasChangedSinceLastTrackPoint() const
{
    return has_changed_;
}

GpuSurfaceViewLayer::GpuSurfaceViewLayer(int64_t surface_id, SkRect dst_rect,
                                         int64_t wait_semaphore_id, int64_t signal_semaphore_id,
                                         ContentTracker *content_tracker)
    : Layer(Type::kGpuSurfaceView)
    , surface_id_(surface_id)
    , dst_rect_(dst_rect)
    , wait_semaphore_id_(wait_semaphore_id)
    , signal_semaphore_id_(signal_semaphore_id)
{
    content_changed_ = true;
    if (content_tracker)
    {
        content_tracker->UpdateTrackPoint();
        content_changed_ = content_tracker->HasChangedSinceLastTrackPoint();
    }
}

void GpuSurfaceViewLayer::Preroll(PrerollContext *context, const SkMatrix& matrix)
{
    SetPaintBounds(dst_rect_);
}

void GpuSurfaceViewLayer::DiffUpdate(const std::shared_ptr<Layer>& other)
{
    CHECK(other->GetType() == Type::kGpuSurfaceView);
    auto layer = std::static_pointer_cast<GpuSurfaceViewLayer>(other);
    surface_id_ = layer->surface_id_;
    dst_rect_ = layer->dst_rect_;
    wait_semaphore_id_ = layer->wait_semaphore_id_;
    signal_semaphore_id_ = layer->signal_semaphore_id_;

    // The `GpuSurfaceViewLayer` is not cachable because we must
    // signal the specified semaphore in each frame.
    IncreaseGenerationId();
}

void GpuSurfaceViewLayer::Paint(PaintContext *context)
{
    // Just skip drawing the view if the GPU rendering is not available.
    // Although we can draw the surface content by downloading its pixels
    // into CPU memory, we still cannot wait/signal the required semaphores
    // without the support of GPU rendering.
    if (!context->gr_context)
        return;

    ContentAggregator *aggregator = context->content_aggregator;
    SkSurface *surface_view = aggregator->GetImportedSkSurface(surface_id_);
    if (!surface_view)
    {
        QLOG(LOG_WARNING, "Failed to find the view surface according to the resource ID");
        return;
    }
    VkSemaphore wait_sem = aggregator->GetImportedGpuSemaphore(wait_semaphore_id_);
    VkSemaphore signal_sem = aggregator->GetImportedGpuSemaphore(signal_semaphore_id_);
    if (wait_sem == VK_NULL_HANDLE || signal_sem == VK_NULL_HANDLE)
    {
        QLOG(LOG_WARNING, "Failed to find the semaphores according to the resource ID");
        return;
    }

    // Wait for the view surface's image memory barrier. Semaphore `wait_sem` should
    // be signaled by user. It must make sure that when the `wait_sem` is signaled,
    // all the drawing commands are finished, and the queue family of it must be
    // `VK_QUEUE_FAMILY_EXTERNAL`.
    GrBackendSemaphore wait_backend_sem;
    wait_backend_sem.initVulkan(wait_sem);
    if (!surface_view->wait(1, &wait_backend_sem, false))
    {
        QLOG(LOG_WARNING, "Could not wait on the required semaphore");
        return;
    }

    SkCanvas *canvas = context->multiplexer_canvas;
    canvas->save();
    canvas->clipRect(dst_rect_);

    if (content_changed_)
    {
        // Notify Skia the content has been changed by code outside of Skia, forcing Skia
        // discards the possibly cached image of the surface. Otherwise, the old content
        // from cached image may be drawn when `surface_view->draw()` is called.
        surface_view->notifyContentWillChange(SkSurface::kDiscard_ContentChangeMode);
    }

    surface_view->draw(canvas, dst_rect_.x(), dst_rect_.y(),
                       context->GetCurrentPaintPtr());

    canvas->restore();

    // Semaphore `signal_sem` will be signaled by the present thread. It is guaranteed
    // that when the semaphore is signaled, all the drawing commands related to the view
    // surface are finished, and the queue family of it is `VK_QUEUE_FAMILY_EXTERNAL`.
    GrBackendSemaphore signal_backend_sem;
    signal_backend_sem.initVulkan(signal_sem);
    GrFlushInfo flush_info{};
    flush_info.fNumSemaphores = 1;
    flush_info.fSignalSemaphores = &signal_backend_sem;
    skgpu::MutableTextureState new_view_state(VK_IMAGE_LAYOUT_UNDEFINED,
                                              VK_QUEUE_FAMILY_EXTERNAL);
    GrSemaphoresSubmitted submitted = context->gr_context->flush(
            surface_view, flush_info, &new_view_state);
    if (submitted != GrSemaphoresSubmitted::kYes)
        QLOG(LOG_WARNING, "Failed to submit semaphores to signal them");
}

void GpuSurfaceViewLayer::ToString(std::ostream& out)
{
    out << fmt::format("(gpu-surface-view#{}:{} '(surface-id {}) '(dst-rect {} {} {} {}))",
                       GetUniqueId(), GetGenerationId(), surface_id_,
                       dst_rect_.x(), dst_rect_.y(), dst_rect_.width(), dst_rect_.height());
}

GLAMOR_NAMESPACE_END
