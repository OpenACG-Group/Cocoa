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

#include "Glamor/Layers/LayerTree.h"
#include "Glamor/Layers/ContainerLayer.h"
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
    auto ret = info.GetThis()->As<ContentAggregator>()->Update(
            info.Get<std::shared_ptr<LayerTree>>(0));
    info.SetReturnStatus(ret == ContentAggregator::UpdateResult::kError
                         ? PresentRemoteCall::Status::kOpFailed
                         : PresentRemoteCall::Status::kOpSuccess);
    info.SetReturnValue(ret);
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
    ContentAggregator::ImportedResourcesId id = bl->ImportGpuSemaphoreFromFd(
            info.Get<int32_t>(0), info.Get<bool>(1));
    info.SetReturnStatus(id >= 0 ? PresentRemoteCall::Status::kOpSuccess
                                 : PresentRemoteCall::Status::kOpFailed);
    info.SetReturnValue(id);
}

GLAMOR_TRAMPOLINE_IMPL(ContentAggregator, DeleteImportedGpuSemaphore)
{
    GLAMOR_TRAMPOLINE_CHECK_ARGS_NUMBER(1);
    auto bl = info.GetThis()->As<ContentAggregator>();
    bl->DeleteImportedGpuSemaphore(info.Get<ContentAggregator::ImportedResourcesId>(0));
    info.SetReturnStatus(PresentRemoteCall::Status::kOpSuccess);
}

GLAMOR_TRAMPOLINE_IMPL(ContentAggregator, ImportGpuSkSurface)
{
    GLAMOR_TRAMPOLINE_CHECK_ARGS_NUMBER(1);
    auto self = info.GetThis()->As<ContentAggregator>();
    ContentAggregator::ImportedResourcesId id = self->ImportGpuSkSurface(
            info.Get<SkiaGpuContextOwner::ExportedSkSurfaceInfo>(0));
    info.SetReturnStatus(id >= 0 ? PresentRemoteCall::Status::kOpSuccess
                                 : PresentRemoteCall::Status::kOpFailed);
    info.SetReturnValue(id);
}

GLAMOR_TRAMPOLINE_IMPL(ContentAggregator, DeleteImportedGpuSkSurface)
{
    GLAMOR_TRAMPOLINE_CHECK_ARGS_NUMBER(1);
    auto self = info.GetThis()->As<ContentAggregator>();
    self->DeleteImportedGpuSkSurface(info.Get<ContentAggregator::ImportedResourcesId>(0));
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
    , surface_frame_slot_id_(0)
    , weak_surface_(surface)
    , current_dirty_rect_(SkIRect::MakeEmpty())
    , frame_schedule_state_(FrameScheduleState::kIdle)
    , should_capture_next_frame_(false)
    , capture_next_frame_serial_(0)
    , imported_resources_ids_cnt_(0)
{
    CHECK(surface);

    auto device = surface->GetRenderTarget()->GetRenderDeviceType();
    std::shared_ptr<SkiaGpuContextOwner> gpu_context_owner;
    if (device == RenderTarget::RenderDevice::kHWComposer)
        gpu_context_owner = surface->GetRenderTarget()->GetHWComposeSwapchain();
    layer_generation_cache_ = std::make_unique<LayerGenerationCache>(gpu_context_owner);

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
    SetMethodTrampoline(GLOP_CONTENTAGGREGATOR_IMPORT_GPU_SKSURFACE,
                        ContentAggregator_ImportGpuSkSurface_Trampoline);
    SetMethodTrampoline(GLOP_CONTENTAGGREGATOR_DELETE_IMPORTED_GPU_SKSURFACE,
                        ContentAggregator_DeleteImportedGpuSkSurface_Trampoline);

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

void ContentAggregator::SurfaceFrameSlot()
{
    TRACE_EVENT("present", "ContentAggregator::SurfaceFrameSlot");

    if (frame_schedule_state_ != FrameScheduleState::kPendingFrame)
        return;
    CHECK(layer_tree_);

    auto rt = GetSurfaceChecked()->GetRenderTarget();
    rt->Present();

    for (const auto& observer : layer_tree_->GetObservers())
        observer->EndFrame();

    frame_schedule_state_ = FrameScheduleState::kPresented;
}

int32_t ContentAggregator::CaptureNextFrameAsPicture()
{
    TRACE_EVENT("present", "ContentAggregator::CaptureNextFrameAsPicture");
    if (!should_capture_next_frame_)
    {
        should_capture_next_frame_ = true;
        capture_next_frame_serial_++;
    }
    return capture_next_frame_serial_;
}

ContentAggregator::UpdateResult
ContentAggregator::Update(const std::shared_ptr<LayerTree>& submitted_layers)
{
    TRACE_EVENT("present", "ContentAggregator::Update");

    if (frame_schedule_state_ == FrameScheduleState::kPendingFrame)
        return UpdateResult::kFrameDropped;

    int32_t vp_width = this->GetWidth();
    int32_t vp_height = this->GetHeight();

    if (layer_tree_)
        layer_tree_->GetRootLayer()->DiffUpdate(submitted_layers->GetRootLayer());
    else
        layer_tree_ = submitted_layers;

    auto surface = GetSurfaceChecked();
    auto rt = surface->GetRenderTarget();
    SkiaGpuContextOwner *gpu_context = nullptr;
    GrDirectContext *gr_context = nullptr;
    if (rt->GetHWComposeSwapchain())
    {
        gpu_context = rt->GetHWComposeSwapchain().get();
        gr_context = gpu_context->GetSkiaGpuContext();
        CHECK(gr_context && "Failed to get Skia GPU direct context");
    }

    // Prepare to preroll the layer tree
    Layer::PrerollContext preroll_context {
        .gr_context = gr_context,
        .root_surface_transformation = GetSurfaceChecked()->GetRootTransformation(),
        .cull_rect = layer_tree_->GetViewportCull()
    };
    if (!layer_tree_->Preroll(&preroll_context))
    {
        QLOG(LOG_ERROR, "Preroll stage was cancelled, no contents will be represented");
        return UpdateResult::kError;
    }

    // Prepare canvases
    SkSurface *frame_surface = rt->BeginFrame();
    SkCanvas *frame_canvas = frame_surface->getCanvas();
    SkAutoCanvasRestore frame_canvas_save(frame_canvas, true);

    SkNWayCanvas multiplexer_canvas(GetWidth(), GetHeight());
    multiplexer_canvas.addCanvas(frame_canvas);
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
        .gpu_context_owner = gpu_context,
        .gr_context = gr_context,
        .is_generating_cache = false,
        .root_surface_transformation = surface->GetRootTransformation(),
        .frame_surface = frame_surface,
        .frame_canvas = frame_canvas,
        .multiplexer_canvas = &multiplexer_canvas,
        .cull_rect = layer_tree_->GetViewportCull(),
        .cache = layer_generation_cache_.get(),
        .content_aggregator = this
    };

    layer_generation_cache_->BeginFrame();

    multiplexer_canvas.clear(SK_ColorWHITE);
    layer_tree_->Paint(&paint_context);
    layer_generation_cache_->EndFrame();

    if (picture_recorder.getRecordingCanvas())
    {
        MaybeGpuObject<SkPicture> picture(
                paint_context.resource_usage_flags & Layer::PaintContext::kGpu_ResourceUsage,
                picture_recorder.finishRecordingAsPicture());

        PresentSignal info;
        info.EmplaceBack<MaybeGpuObject<SkPicture>>(std::move(picture));
        info.EmplaceBack<int32_t>(capture_next_frame_serial_);
        Emit(GLSI_CONTENTAGGREGATOR_PICTURE_CAPTURED, std::move(info));
    }

    // At last, we request a new frame from WSI layer. We will be notified
    // (slot function `SurfaceFrameSlot` will be called) later
    // when it is a good time to present a new frame (VSync).
    current_dirty_rect_ = layer_tree_->GetRootLayer()->GetPaintBounds().roundOut();
    surface->RequestNextFrame();

    surface->GetRenderTarget()->Submit({
        .damage_region = SkRegion(current_dirty_rect_),
        .hw_signal_semaphores = std::move(paint_context.gpu_finished_semaphores)
    });

    frame_schedule_state_ = FrameScheduleState::kPendingFrame;
    return UpdateResult::kSuccess;
}

void ContentAggregator::Dispose()
{
    if (disposed_)
        return;

    auto surface = GetSurfaceChecked();

    surface->Disconnect(surface_frame_slot_id_);

    // That the frame is in pending state means we have called the
    // `RenderTarget::Begin` function, which expects a corresponding
    // `RenderTarget::Submit` call. But the slot of `frame` signal
    // has been disconnected and `Submit` will never be called,
    // so call it manually here to make sure every `Begin` call has a
    // corresponding `Submit` call.
    if (frame_schedule_state_ == FrameScheduleState::kPendingFrame)
        SurfaceFrameSlot();

    // Delete all the imported resources
    if (!imported_resources_ids_.empty())
    {
        VkDevice device = surface->GetRenderTarget()
                ->GetHWComposeSwapchain()->GetVkDevice();
        for (auto& pair : imported_resources_ids_)
        {
            if (pair.second.type == ImportedResourceEntry::kSemaphore)
                vkDestroySemaphore(device, pair.second.semaphore, nullptr);
            else if (pair.second.type == ImportedResourceEntry::kSkSurface)
            {
                CHECK(pair.second.surface->unique() && "SkSurface is referenced");
                pair.second.surface.reset();
            }
        }
    }

    layer_generation_cache_.reset();

    frame_schedule_state_ = FrameScheduleState::kDisposed;
    disposed_ = true;
}

void ContentAggregator::Trace(GraphicsResourcesTrackable::Tracer *tracer) noexcept
{
    CHECK(!disposed_);
    tracer->TraceMember("LayerGenerationCache", layer_generation_cache_.get());
}

void ContentAggregator::PurgeRasterCacheResources()
{
    TRACE_EVENT("present", "ContentAggregator::PurgeRasterCacheResources");
    layer_generation_cache_->PurgeCacheResources(true);
}

std::shared_ptr<HWComposeSwapchain> ContentAggregator::TryGetSwapchain()
{
    CHECK(!disposed_);
    std::shared_ptr<Surface> surface = GetSurfaceChecked();
    std::shared_ptr<RenderTarget> render_target = surface->GetRenderTarget();
    if (render_target->GetRenderDeviceType() != RenderTarget::RenderDevice::kHWComposer)
        return nullptr;
    return render_target->GetHWComposeSwapchain();
}

ContentAggregator::ImportedResourcesId ContentAggregator::ImportGpuSemaphoreFromFd(int32_t fd, bool auto_close)
{
    CHECK(!disposed_);
    ScopeExitAutoInvoker auto_closer([auto_close, fd] {
        if (auto_close)
            close(fd);
    });

    auto swapchain = TryGetSwapchain();
    if (!swapchain)
        return -1;

    auto_closer.Cancel();
    ImportedResourcesId id = imported_resources_ids_cnt_++;
    imported_resources_ids_[id] = {
        .type = ImportedResourceEntry::kSemaphore,
        .semaphore = swapchain->ImportSemaphoreFromFd(fd),
        .surface = nullptr
    };
    return id;
}

void ContentAggregator::DeleteImportedGpuSemaphore(ImportedResourcesId id)
{
    CHECK(!disposed_);
    if (imported_resources_ids_.count(id) == 0)
        return;

    auto swapchain = TryGetSwapchain();
    if (!swapchain)
        return;
    ImportedResourceEntry& entry = imported_resources_ids_[id];
    if (entry.type != ImportedResourceEntry::kSemaphore)
        return;
    vkDestroySemaphore(swapchain->GetVkDevice(), entry.semaphore, nullptr);
    imported_resources_ids_.erase(id);
}

ContentAggregator::ImportedResourcesId
ContentAggregator::ImportGpuSkSurface(const SkiaGpuContextOwner::ExportedSkSurfaceInfo& info)
{
    CHECK(!disposed_);
    ScopeExitAutoInvoker auto_closer([&info] {
        close(info.fd);
    });

    auto swapchain = TryGetSwapchain();
    if (!swapchain)
        return -1;

    sk_sp<SkSurface> sk_surface = swapchain->ImportSkSurface(info);
    if (!sk_surface)
        return -1;

    auto_closer.Cancel();
    ImportedResourcesId id = imported_resources_ids_cnt_++;
    imported_resources_ids_[id] = {
        .type = ImportedResourceEntry::kSkSurface,
        .semaphore = VK_NULL_HANDLE,
        .surface = std::move(sk_surface)
    };
    return id;
}

void ContentAggregator::DeleteImportedGpuSkSurface(ImportedResourcesId id)
{
    CHECK(!disposed_);
    if (imported_resources_ids_.count(id) == 0)
        return;
    ImportedResourceEntry& entry = imported_resources_ids_[id];
    if (entry.type != ImportedResourceEntry::kSkSurface)
        return;
    imported_resources_ids_.erase(id);
}

SkSurface *ContentAggregator::GetImportedSkSurface(ImportedResourcesId id)
{
    CHECK(!disposed_);
    if (imported_resources_ids_.count(id) == 0)
        return nullptr;
    ImportedResourceEntry& entry = imported_resources_ids_[id];
    if (entry.type != ImportedResourceEntry::kSkSurface)
        return nullptr;
    return entry.surface.get();
}

VkSemaphore ContentAggregator::GetImportedGpuSemaphore(ImportedResourcesId id)
{
    if (imported_resources_ids_.count(id) == 0)
        return VK_NULL_HANDLE;
    ImportedResourceEntry& entry = imported_resources_ids_[id];
    if (entry.type != ImportedResourceEntry::kSemaphore)
        return VK_NULL_HANDLE;
    return entry.semaphore;
}

GLAMOR_NAMESPACE_END
