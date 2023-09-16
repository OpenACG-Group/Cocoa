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

#include "include/core/SkCanvas.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkColorSpace.h"
#include "include/utils/SkNWayCanvas.h"

#include "Core/StandaloneThreadPool.h"
#include "Core/TraceEvent.h"
#include "Core/Journal.h"
#include "Glamor/ContentAggregator.h"
#include "Glamor/RenderTarget.h"
#include "Glamor/Surface.h"
#include "Glamor/HWComposeSwapchain.h"
#include "Glamor/GProfiler.h"

#include "Glamor/Layers/LayerTree.h"
#include "Glamor/Layers/RasterDrawOpObserver.h"
GLAMOR_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Glamor.ContentAggregator)

GLAMOR_TRAMPOLINE_IMPL(ContentAggregator, Dispose)
{
    info.GetThis()->As<ContentAggregator>()->Dispose();
    info.SetReturnStatus(PresentRemoteCall::Status::kOpSuccess);
}

GLAMOR_TRAMPOLINE_IMPL(ContentAggregator, Update)
{
    GLAMOR_TRAMPOLINE_CHECK_ARGS_NUMBER(1);
    info.GetThis()->As<ContentAggregator>()->Update(
            info.Get<std::shared_ptr<LayerTree>>(0));
    info.SetReturnStatus(PresentRemoteCall::Status::kOpSuccess);
}

GLAMOR_TRAMPOLINE_IMPL(ContentAggregator, CaptureNextFrameAsPicture)
{
    GLAMOR_TRAMPOLINE_CHECK_ARGS_NUMBER(0);
    auto bl = info.GetThis()->As<ContentAggregator>();
    info.SetReturnValue(bl->CaptureNextFrameAsPicture());
    info.SetReturnStatus(PresentRemoteCall::Status::kOpSuccess);
}

GLAMOR_TRAMPOLINE_IMPL(ContentAggregator, PurgeRasterCacheResources)
{
    GLAMOR_TRAMPOLINE_CHECK_ARGS_NUMBER(0);
    auto bl = info.GetThis()->As<ContentAggregator>();
    bl->PurgeRasterCacheResources();
    info.SetReturnStatus(PresentRemoteCall::Status::kOpSuccess);
}

GLAMOR_TRAMPOLINE_IMPL(ContentAggregator, ImportGpuSemaphoreFromFd)
{
    GLAMOR_TRAMPOLINE_CHECK_ARGS_NUMBER(2);
    auto bl = info.GetThis()->As<ContentAggregator>();
    ContentAggregator::ImportedSemaphoreId id = bl->ImportGpuSemaphoreFromFd(
            info.Get<int32_t>(0), info.Get<bool>(1));
    info.SetReturnStatus(id >= 0 ? PresentRemoteCall::Status::kOpSuccess
                                 : PresentRemoteCall::Status::kOpFailed);
    info.SetReturnValue(id);
}

GLAMOR_TRAMPOLINE_IMPL(ContentAggregator, DeleteImportedGpuSemaphore)
{
    GLAMOR_TRAMPOLINE_CHECK_ARGS_NUMBER(1);
    auto bl = info.GetThis()->As<ContentAggregator>();
    bl->DeleteImportedGpuSemaphore(info.Get<ContentAggregator::ImportedSemaphoreId>(0));
    info.SetReturnStatus(PresentRemoteCall::Status::kOpSuccess);
}

std::shared_ptr<ContentAggregator>
ContentAggregator::Make(const std::shared_ptr<Surface>& surface)
{
    CHECK(surface);
    return std::make_shared<ContentAggregator>(surface);
}

ContentAggregator::ContentAggregator(const std::shared_ptr<Surface>& surface)
    : PresentRemoteHandle(RealType::kContentAggregator)
    , disposed_(false)
    , surface_resize_slot_id_(0)
    , surface_frame_slot_id_(0)
    , weak_surface_(surface)
    , current_dirty_rect_(SkIRect::MakeEmpty())
    , frame_schedule_state_(FrameScheduleState::kIdle)
    , raster_cache_()
    , should_capture_next_frame_(false)
    , capture_next_frame_serial_(0)
    , imported_semaphore_ids_cnt_(0)
{
    CHECK(surface);

    int32_t width = surface->GetWidth();
    int32_t height = surface->GetHeight();
    layer_tree_ = std::make_unique<LayerTree>(SkISize::Make(width, height));

    ContextOptions& options = GlobalScope::Ref().GetOptions();
    if (options.GetEnableProfiler())
    {
        QLOG(LOG_DEBUG, "Graphics profiler is available on the ContentAggregator");
        gfx_profiler_ = std::make_shared<GProfiler>();
    }

    auto device = surface->GetRenderTarget()->GetRenderDeviceType();
    GrDirectContext *direct_context = nullptr;
    if (device == RenderTarget::RenderDevice::kHWComposer)
    {
        direct_context = surface->GetRenderTarget()
                ->GetHWComposeSwapchain()->GetSkiaGpuContext();
    }
    raster_cache_ = std::make_unique<RasterCache>(direct_context);

    SetMethodTrampoline(GLOP_CONTENTAGGREGATOR_DISPOSE, ContentAggregator_Dispose_Trampoline);
    SetMethodTrampoline(GLOP_CONTENTAGGREGATOR_UPDATE, ContentAggregator_Update_Trampoline);
    SetMethodTrampoline(GLOP_CONTENTAGGREGATOR_CAPTURE_NEXT_FRAME_AS_PICTURE,
                        ContentAggregator_CaptureNextFrameAsPicture_Trampoline);
    SetMethodTrampoline(GLOP_CONTENTAGGREGATOR_PURGE_RASTER_CACHE_RESOURCES,
                        ContentAggregator_PurgeRasterCacheResources_Trampoline);
    SetMethodTrampoline(GLOP_CONTENTAGGREGATOR_IMPORT_GPU_SEMAPHORE_FROM_FD,
                        ContentAggregator_ImportGpuSemaphoreFromFd_Trampoline);
    SetMethodTrampoline(GLOP_CONTENTAGGREGATOR_DELETE_IMPORTED_GPU_SEMAPHORE,
                        ContentAggregator_DeleteImportedGpuSemaphore_Trampoline);

    surface_resize_slot_id_ = surface->Connect(
        GLSI_SURFACE_RESIZE,
        [this](PresentSignalArgs& info) {
            this->SurfaceResizeSlot(info.Get<int32_t>(0), info.Get<int32_t>(1));
        },
        true
    );

    surface_frame_slot_id_ = surface->Connect(
        GLSI_SURFACE_FRAME,
        [this](PresentSignalArgs& info) {
            this->SurfaceFrameSlot();
        },
        true
    );
}

ContentAggregator::~ContentAggregator()
{
    Dispose();
}

std::shared_ptr<Surface> ContentAggregator::GetSurfaceChecked() const
{
    auto sp = weak_surface_.lock();
    CHECK(sp && "Surface has expired");
    return sp;
}

RenderTarget::RenderDevice ContentAggregator::GetRenderDeviceType() const
{
    return GetSurfaceChecked()->GetRenderTarget()->GetRenderDeviceType();
}

int32_t ContentAggregator::GetWidth() const
{
    return GetSurfaceChecked()->GetWidth();
}

int32_t ContentAggregator::GetHeight() const
{
    return GetSurfaceChecked()->GetHeight();
}

SkColorInfo ContentAggregator::GetOutputColorInfo() const
{
    return {GetSurfaceChecked()->GetColorType(),
            SkAlphaType::kPremul_SkAlphaType, nullptr};
}

#define GPROFILER_TRY_MARK(tag)                                                  \
    if (gfx_profiler_) {                                                         \
        gfx_profiler_->MarkMilestoneInFrame(GProfiler::k##tag##_FrameMilestone); \
    }

#define GPROFILER_TRY_BEGIN_FRAME()     \
    if (gfx_profiler_) {                \
        gfx_profiler_->BeginFrame();    \
    }

#define GPROFILER_TRY_END_FRAME()       \
    if (gfx_profiler_) {                \
        gfx_profiler_->EndFrame();      \
    }

void ContentAggregator::SurfaceFrameSlot()
{
    TRACE_EVENT("rendering", "ContentAggregator::SurfaceFrameSlot");

    if (frame_schedule_state_ == FrameScheduleState::kIdle)
    {
        QLOG(LOG_WARNING, "Frame scheduler: Expecting PendingFrame state instead of Idle");
        return;
    }
    else if (frame_schedule_state_ == FrameScheduleState::kPresented)
    {
        QLOG(LOG_WARNING, "Frame scheduler: Expecting PendingFrame state instead of Presented");
        return;
    }

    auto rt = GetSurfaceChecked()->GetRenderTarget();
    rt->Present();

    for (const auto& observer : layer_tree_->GetObservers())
        observer->EndFrame();

    GPROFILER_TRY_MARK(Presented)
    GPROFILER_TRY_END_FRAME()

    frame_schedule_state_ = FrameScheduleState::kPresented;
}

int32_t ContentAggregator::CaptureNextFrameAsPicture()
{
    TRACE_EVENT("rendering", "ContentAggregator::CaptureNextFrameAsPicture");
    if (!should_capture_next_frame_)
    {
        should_capture_next_frame_ = true;
        capture_next_frame_serial_++;
    }
    return capture_next_frame_serial_;
}

void ContentAggregator::Update(const std::shared_ptr<LayerTree> &layer_tree)
{
    TRACE_EVENT("rendering", "ContentAggregator::Update");

    if (frame_schedule_state_ == FrameScheduleState::kPendingFrame)
    {
        QLOG(LOG_WARNING, "Frame scheduler: frame is dropped (updating in PendingFrame state)");
        return;
    }

    GPROFILER_TRY_BEGIN_FRAME()

    int32_t vp_width = this->GetWidth();
    int32_t vp_height = this->GetHeight();

    raster_cache_->IncreaseFrameCount();

    // TODO: diff & update layer tree
    layer_tree_ = layer_tree;

    auto surface = GetSurfaceChecked();
    auto rt = surface->GetRenderTarget();
    GrDirectContext *gr_context = nullptr;
    if (rt->GetHWComposeSwapchain())
    {
        gr_context = rt->GetHWComposeSwapchain()->GetSkiaGpuContext();
        CHECK(gr_context && "Failed to get Skia GPU direct context");
    }

    // Prepare to preroll the layer tree
    Layer::PrerollContext context {
        .gr_context = gr_context,
        .root_surface_transformation = GetSurfaceChecked()->GetRootTransformation(),
        .cull_rect = SkRect::MakeEmpty(),
        .raster_cache = raster_cache_.get()
    };

    GPROFILER_TRY_MARK(PrerollBegin)

    if (!layer_tree_->Preroll(&context))
    {
        QLOG(LOG_ERROR, "Preroll stage was cancelled, no contents will be represented");
        return;
    }

    GPROFILER_TRY_MARK(PrerollEnd)

    // Prepare canvases
    SkSurface *frame_surface = rt->BeginFrame();
    frame_surface->getCanvas()->clear(SK_ColorBLACK);

    SkNWayCanvas multiplexer_canvas(GetWidth(), GetHeight());
    multiplexer_canvas.addCanvas(frame_surface->getCanvas());
    for (const auto& observer : layer_tree_->GetObservers())
    {
        SkCanvas *observer_canvas = observer->BeginFrame(
                gr_context, SkISize::Make(vp_width, vp_height));
        if (!observer_canvas)
        {
            QLOG(LOG_ERROR, "DrawOp observer \"{}\" could not provide a valid canvas, ignored",
                 observer->GetExternalObserverName());
        }
        else
        {
            multiplexer_canvas.addCanvas(observer_canvas);
        }
    }

    SkPictureRecorder picture_recorder;
    if (should_capture_next_frame_)
    {
        auto width = static_cast<SkScalar>(vp_width);
        auto height = static_cast<SkScalar>(vp_height);
        SkCanvas *canvas = picture_recorder.beginRecording(width, height);
        multiplexer_canvas.addCanvas(canvas);
    }
    should_capture_next_frame_ = false;

    Layer::PaintContext paint_context {
        .gr_context = gr_context,
        .root_surface_transformation = surface->GetRootTransformation(),
        .frame_surface = frame_surface,
        .frame_canvas = frame_surface->getCanvas(),
        .multiplexer_canvas = &multiplexer_canvas,
        .cull_rect = context.cull_rect,
        .has_gpu_retained_resource = false,
        .raster_cache = raster_cache_.get()
    };


    GPROFILER_TRY_MARK(PaintBegin)
    layer_tree_->Paint(&paint_context);
    GPROFILER_TRY_MARK(PaintEnd)

    if (picture_recorder.getRecordingCanvas())
    {
        MaybeGpuObject<SkPicture> picture(
                paint_context.has_gpu_retained_resource,
                picture_recorder.finishRecordingAsPicture());

        PresentSignal info;
        info.EmplaceBack<MaybeGpuObject<SkPicture>>(std::move(picture));
        info.EmplaceBack<int32_t>(capture_next_frame_serial_);
        Emit(GLSI_CONTENTAGGREGATOR_PICTURE_CAPTURED, std::move(info));
    }

    // At last, we request a new frame from WSI layer. We will be notified
    // (slot function `SurfaceFrameSlot` will be called) later
    // when it is a good time to present a new frame (VSync).
    current_dirty_rect_ = SkIRect::MakeXYWH(static_cast<int32_t>(context.cull_rect.x()),
                                            static_cast<int32_t>(context.cull_rect.y()),
                                            static_cast<int32_t>(context.cull_rect.width()),
                                            static_cast<int32_t>(context.cull_rect.height()));
    surface->RequestNextFrame();

    surface->GetRenderTarget()->Submit({
        .damage_region = SkRegion(current_dirty_rect_),
        .hw_signal_semaphores = std::move(paint_context.gpu_finished_semaphores)
    });

    GPROFILER_TRY_MARK(Requested)

    frame_schedule_state_ = FrameScheduleState::kPendingFrame;
}

void ContentAggregator::SurfaceResizeSlot(int32_t width, int32_t height)
{
    TRACE_EVENT("rendering", "ContentAggregator::SurfaceResizeSlot");
    layer_tree_->SetFrameSize(SkISize::Make(width, height));
}

void ContentAggregator::Dispose()
{
    if (disposed_)
        return;

    auto surface = GetSurfaceChecked();

    surface->Disconnect(surface_resize_slot_id_);
    surface->Disconnect(surface_frame_slot_id_);

    // That the frame is in pending state means we have called the
    // `RenderTarget::Begin` function, which expects a corresponding
    // `RenderTarget::Submit` call. But the slot of `frame` signal
    // has been disconnected and `Submit` will never be called,
    // so call it manually here to make sure every `Begin` call has a
    // corresponding `Submit` call.
    if (frame_schedule_state_ == FrameScheduleState::kPendingFrame)
        SurfaceFrameSlot();

    // Delete all the imported semaphores
    if (!imported_semaphore_ids_.empty())
    {
        VkDevice device = surface->GetRenderTarget()
                ->GetHWComposeSwapchain()->GetVkDevice();
        for (const auto& pair : imported_semaphore_ids_)
            vkDestroySemaphore(device, pair.second, nullptr);
    }

    raster_cache_.reset();

    frame_schedule_state_ = FrameScheduleState::kDisposed;
    disposed_ = true;
}

void ContentAggregator::Trace(GraphicsResourcesTrackable::Tracer *tracer) noexcept
{
    CHECK(!disposed_);

    // `LayerTree` only contains raw data and texture references which are from
    // user-defined data (generally from the CanvasKit module in JavaScript land),
    // and there is no need to trace them.
    tracer->TraceMember("RasterCache", raster_cache_.get());
}

void ContentAggregator::PurgeRasterCacheResources()
{
    TRACE_EVENT("rendering", "ContentAggregator::PurgeRasterCacheResources");
    raster_cache_->PurgeAllCaches();
}

ContentAggregator::ImportedSemaphoreId ContentAggregator::ImportGpuSemaphoreFromFd(int32_t fd, bool auto_close)
{
    CHECK(!disposed_);

    ScopeExitAutoInvoker auto_closer([auto_close, fd] {
        if (auto_close)
            close(fd);
    });

    std::shared_ptr<Surface> surface = GetSurfaceChecked();
    std::shared_ptr<RenderTarget> render_target = surface->GetRenderTarget();
    if (render_target->GetRenderDeviceType() != RenderTarget::RenderDevice::kHWComposer)
        return -1;
    std::shared_ptr<HWComposeSwapchain> swapchain =
            render_target->GetHWComposeSwapchain();
    if (!swapchain)
        return -1;

    auto_closer.cancel();
    ImportedSemaphoreId id = imported_semaphore_ids_cnt_++;
    imported_semaphore_ids_[id] = swapchain->ImportSemaphoreFromFd(fd);
    return id;
}

void ContentAggregator::DeleteImportedGpuSemaphore(ImportedSemaphoreId id)
{
    CHECK(!disposed_);
    if (imported_semaphore_ids_.count(id) == 0)
        return;

    std::shared_ptr<Surface> surface = GetSurfaceChecked();
    std::shared_ptr<RenderTarget> render_target = surface->GetRenderTarget();
    CHECK(render_target->GetRenderDeviceType() == RenderTarget::RenderDevice::kHWComposer);
    std::shared_ptr<HWComposeSwapchain> swapchain =
            render_target->GetHWComposeSwapchain();
    CHECK(swapchain);

    VkSemaphore semaphore = imported_semaphore_ids_[id];
    vkDestroySemaphore(swapchain->GetVkDevice(), semaphore, nullptr);
    imported_semaphore_ids_.erase(id);
}

VkSemaphore ContentAggregator::GetImportedGpuSemaphore(ImportedSemaphoreId id)
{
    if (imported_semaphore_ids_.count(id) == 0)
        return VK_NULL_HANDLE;
    return imported_semaphore_ids_[id];
}

GLAMOR_NAMESPACE_END