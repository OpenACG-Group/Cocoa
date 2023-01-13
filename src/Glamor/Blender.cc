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

#include <future>

#include "include/core/SkCanvas.h"
#include "include/core/SkDeferredDisplayListRecorder.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkGraphics.h"
#include "include/utils/SkNWayCanvas.h"

#include "Core/StandaloneThreadPool.h"
#include "Core/Journal.h"
#include "Glamor/Blender.h"
#include "Glamor/RenderTarget.h"
#include "Glamor/Surface.h"
#include "Glamor/HWComposeSwapchain.h"
#include "Glamor/RasterFrameGenerator.h"
#include "Glamor/TextureManager.h"
#include "Glamor/TextureFactory.h"
#include "Glamor/GProfiler.h"

#include "Glamor/Layers/LayerTree.h"
#include "Glamor/Layers/RasterDrawOpObserver.h"
GLAMOR_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Glamor.Blender)

GLAMOR_TRAMPOLINE_IMPL(Blender, Dispose)
{
    info.GetThis()->As<Blender>()->Dispose();
    info.SetReturnStatus(RenderClientCallInfo::Status::kOpSuccess);
}

GLAMOR_TRAMPOLINE_IMPL(Blender, Update)
{
    GLAMOR_TRAMPOLINE_CHECK_ARGS_NUMBER(1);
    info.GetThis()->As<Blender>()->Update(info.Get<Shared<LayerTree>>(0));
    info.SetReturnStatus(RenderClientCallInfo::Status::kOpSuccess);
}

GLAMOR_TRAMPOLINE_IMPL(Blender, DeleteTexture)
{
    GLAMOR_TRAMPOLINE_CHECK_ARGS_NUMBER(1);
    bool stat = info.GetThis()->As<Blender>()->DeleteTexture(
            info.Get<Texture::TextureId>(0));
    info.SetReturnStatus(stat ? RenderClientCallInfo::Status::kOpSuccess
                              : RenderClientCallInfo::Status::kOpFailed);
}

GLAMOR_TRAMPOLINE_IMPL(Blender, NewTextureDeletionSubscriptionSignal)
{
    GLAMOR_TRAMPOLINE_CHECK_ARGS_NUMBER(1);
    auto bl = info.GetThis()->As<Blender>();
    int32_t sig = bl->NewTextureDeletionSubscriptionSignal(
            info.Get<Texture::TextureId>(0));
    info.SetReturnStatus(RenderClientCallInfo::Status::kOpSuccess);
    info.SetReturnValue(sig);
}

GLAMOR_TRAMPOLINE_IMPL(Blender, CreateTextureFromEncodedData)
{
    GLAMOR_TRAMPOLINE_CHECK_ARGS_NUMBER(3);
    auto bl = info.GetThis()->As<Blender>();
    auto a1 = info.Get<Shared<Data>>(0);
    auto a2 = info.Get<std::optional<SkAlphaType>>(1);
    auto a3 = info.Get<std::string>(2);
    Blender::MaybeTextureId id = bl->CreateTextureFromEncodedData(a1, a2, a3);
    info.SetReturnStatus(id ? RenderClientCallInfo::Status::kOpSuccess
                            : RenderClientCallInfo::Status::kOpFailed);
    info.SetReturnValue(*id);
}

GLAMOR_TRAMPOLINE_IMPL(Blender, CreateTextureFromImage)
{
    GLAMOR_TRAMPOLINE_CHECK_ARGS_NUMBER(2);
    auto bl = info.GetThis()->As<Blender>();
    auto a1 = info.Get<sk_sp<SkImage>>(0);
    auto a2 = info.Get<std::string>(1);
    Blender::MaybeTextureId id = bl->CreateTextureFromImage(a1, a2);
    info.SetReturnStatus(id ? RenderClientCallInfo::Status::kOpSuccess
                            : RenderClientCallInfo::Status::kOpFailed);
    info.SetReturnValue(*id);
}

GLAMOR_TRAMPOLINE_IMPL(Blender, CreateTextureFromPixmap)
{
    GLAMOR_TRAMPOLINE_CHECK_ARGS_NUMBER(3);
    auto bl = info.GetThis()->As<Blender>();
    auto a1 = info.Get<const void*>(0);
    auto a2 = info.Get<SkImageInfo>(1);
    auto a3 = info.Get<std::string>(2);
    Blender::MaybeTextureId id = bl->CreateTextureFromPixmap(a1, a2, a3);
    info.SetReturnStatus(id ? RenderClientCallInfo::Status::kOpSuccess
                            : RenderClientCallInfo::Status::kOpFailed);
    info.SetReturnValue(*id);
}

GLAMOR_TRAMPOLINE_IMPL(Blender, CaptureNextFrameAsPicture)
{
    GLAMOR_TRAMPOLINE_CHECK_ARGS_NUMBER(0);
    auto bl = info.GetThis()->As<Blender>();
    info.SetReturnValue(bl->CaptureNextFrameAsPicture());
    info.SetReturnStatus(RenderClientCallInfo::Status::kOpSuccess);
}

GLAMOR_TRAMPOLINE_IMPL(Blender, PurgeRasterCacheResources)
{
    GLAMOR_TRAMPOLINE_CHECK_ARGS_NUMBER(0);
    auto bl = info.GetThis()->As<Blender>();
    bl->PurgeRasterCacheResources();
    info.SetReturnStatus(RenderClientCallInfo::Status::kOpSuccess);
}

Shared<Blender> Blender::Make(const Shared<Surface>& surface)
{
    CHECK(surface);

    auto render_target = surface->GetRenderTarget();
    CHECK(render_target);

    // Create a TextureFactory to prepare for creating `TextureManager`
    auto render_device = render_target->GetRenderDeviceType();
    Unique<TextureFactory> texture_factory;
    if (render_device == RenderTarget::RenderDevice::kHWComposer)
    {
        auto swapchain = render_target->GetHWComposeSwapchain();
        texture_factory = std::make_unique<HWComposeTextureFactory>(swapchain);
    }
    else
    {
        SkColorInfo info{surface->GetColorType(),
                         SkAlphaType::kPremul_SkAlphaType,
                         nullptr};
        texture_factory = std::make_unique<RasterTextureFactory>(info);
    }

    // `TextureManager` is only owned by `Blender` and will be released when the
    // blender is disposed.
    auto texture_manager = std::make_unique<TextureManager>(std::move(texture_factory));

    return std::make_shared<Blender>(surface, std::move(texture_manager));
}

Blender::Blender(const Shared<Surface>& surface,
                 Unique<TextureManager> texture_manager)
    : RenderClientObject(RealType::kBlender)
    , disposed_(false)
    , surface_resize_slot_id_(0)
    , surface_frame_slot_id_(0)
    , output_surface_(surface)
    , layer_tree_(std::make_unique<LayerTree>(SkISize::Make(surface->GetWidth(), surface->GetWidth())))
    , current_dirty_rect_(SkIRect::MakeEmpty())
    , frame_schedule_state_(FrameScheduleState::kIdle)
    , texture_manager_(std::move(texture_manager))
    , raster_cache_()
    , should_capture_next_frame_(false)
    , capture_next_frame_serial_(0)
{
    CHECK(surface);

    ContextOptions& options = GlobalScope::Ref().GetOptions();
    if (options.GetEnableProfiler())
    {
        QLOG(LOG_DEBUG, "Graphics profiler is available on the Blender");
        gfx_profiler_ = std::make_shared<GProfiler>();
    }

    auto device = surface->GetRenderTarget()->GetRenderDeviceType();
    GrDirectContext *direct_context = nullptr;
    if (device == RenderTarget::RenderDevice::kHWComposer)
        direct_context = surface->GetRenderTarget()->GetHWComposeSwapchain()->GetSkiaDirectContext().get();
    raster_cache_ = std::make_unique<RasterCache>(direct_context);

    SetMethodTrampoline(GLOP_BLENDER_DISPOSE, Blender_Dispose_Trampoline);
    SetMethodTrampoline(GLOP_BLENDER_UPDATE, Blender_Update_Trampoline);
    SetMethodTrampoline(GLOP_BLENDER_CREATE_TEXTURE_FROM_PIXMAP,
                        Blender_CreateTextureFromPixmap_Trampoline);
    SetMethodTrampoline(GLOP_BLENDER_CREATE_TEXTURE_FROM_ENCODED_DATA,
                        Blender_CreateTextureFromEncodedData_Trampoline);
    SetMethodTrampoline(GLOP_BLENDER_CREATE_TEXTURE_FROM_IMAGE,
                        Blender_CreateTextureFromImage_Trampoline);
    SetMethodTrampoline(GLOP_BLENDER_DELETE_TEXTURE,
                        Blender_DeleteTexture_Trampoline);
    SetMethodTrampoline(GLOP_BLENDER_NEW_TEXTURE_DELETION_SUBSCRIPTION_SIGNAL,
                        Blender_NewTextureDeletionSubscriptionSignal_Trampoline);
    SetMethodTrampoline(GLOP_BLENDER_CAPTURE_NEXT_FRAME_AS_PICTURE,
                        Blender_CaptureNextFrameAsPicture_Trampoline);
    SetMethodTrampoline(GLOP_BLENDER_PURGE_RASTER_CACHE_RESOURCES,
                        Blender_PurgeRasterCacheResources_Trampoline);

    surface_resize_slot_id_ = surface->Connect(GLSI_SURFACE_RESIZE,
                                               [this](RenderHostSlotCallbackInfo& info) {
        this->SurfaceResizeSlot(info.Get<int32_t>(0), info.Get<int32_t>(1));
    }, true);

    surface_frame_slot_id_ = surface->Connect(GLSI_SURFACE_FRAME,
                                              [this](RenderHostSlotCallbackInfo& info) {
        this->SurfaceFrameSlot();
    }, true);
}

Blender::~Blender()
{
    Dispose();
}

RenderTarget::RenderDevice Blender::GetRenderDeviceType() const
{
    return output_surface_->GetRenderTarget()->GetRenderDeviceType();
}

int32_t Blender::GetWidth() const
{
    return output_surface_->GetWidth();
}

int32_t Blender::GetHeight() const
{
    return output_surface_->GetHeight();
}

SkColorInfo Blender::GetOutputColorInfo() const
{
    return {output_surface_->GetColorType(), SkAlphaType::kPremul_SkAlphaType, nullptr};
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

void Blender::SurfaceFrameSlot()
{
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

    // Finally, submitting the rasterized surface to the screen notifies `RenderTarget`
    // to swap framebuffer.
    Shared<RenderTarget> rt = output_surface_->GetRenderTarget();
    rt->Submit(SkRegion(current_dirty_rect_));

    for (const Shared<RasterDrawOpObserver>& observer : layer_tree_->GetObservers())
        observer->EndFrame();

    GPROFILER_TRY_MARK(Presented)
    GPROFILER_TRY_END_FRAME()

    frame_schedule_state_ = FrameScheduleState::kPresented;
}

int32_t Blender::CaptureNextFrameAsPicture()
{
    if (!should_capture_next_frame_)
    {
        should_capture_next_frame_ = true;
        capture_next_frame_serial_++;
    }
    return capture_next_frame_serial_;
}

void Blender::Update(const Shared<LayerTree> &layer_tree)
{
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

    auto rt = output_surface_->GetRenderTarget();
    GrDirectContext *gr_context = nullptr;
    if (rt->GetHWComposeSwapchain())
    {
        gr_context = rt->GetHWComposeSwapchain()->GetSkiaDirectContext().get();
        CHECK(gr_context && "Failed to get Skia GPU direct context");
    }

    // Prepare to preroll the layer tree
    Layer::PrerollContext context {
        .gr_context = gr_context,
        .root_surface_transformation = output_surface_->GetRootTransformation(),
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
    for (const Shared<RasterDrawOpObserver>& observer : layer_tree_->GetObservers())
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
        .root_surface_transformation = output_surface_->GetRootTransformation(),
        .frame_surface = frame_surface,
        .frame_canvas = frame_surface->getCanvas(),
        .multiplexer_canvas = &multiplexer_canvas,
        .cull_rect = context.cull_rect,
        .texture_manager = texture_manager_,
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

        RenderClientEmitterInfo info;
        info.EmplaceBack<MaybeGpuObject<SkPicture>>(std::move(picture));
        info.EmplaceBack<int32_t>(capture_next_frame_serial_);
        Emit(GLSI_BLENDER_PICTURE_CAPTURED, std::move(info));
    }

    // At last, we request a new frame from WSI layer. We will be notified
    // (slot function `SurfaceFrameSlot` will be called) later
    // when it is a good time to present a new frame (VSync).
    current_dirty_rect_ = SkIRect::MakeXYWH(static_cast<int32_t>(context.cull_rect.x()),
                                            static_cast<int32_t>(context.cull_rect.y()),
                                            static_cast<int32_t>(context.cull_rect.width()),
                                            static_cast<int32_t>(context.cull_rect.height()));
    output_surface_->RequestNextFrame();

    GPROFILER_TRY_MARK(Requested)

    frame_schedule_state_ = FrameScheduleState::kPendingFrame;
}

void Blender::SurfaceResizeSlot(int32_t width, int32_t height)
{
    layer_tree_->SetFrameSize(SkISize::Make(width, height));
}

bool Blender::DeleteTexture(Texture::TextureId id)
{
    return texture_manager_->Delete(id);
}

int32_t Blender::NewTextureDeletionSubscriptionSignal(Texture::TextureId id)
{
    // Dynamic signal number starts from 16 because it is a number which
    // is big enough to avoid overriding the existing static signal numbers.
    static int32_t signal_counter = 16;

    int32_t sig_number = signal_counter++;
    texture_manager_->SubscribeTextureDeletion(id, [sig_number, this]() {
        this->Emit(sig_number, RenderClientEmitterInfo());
    });

    return sig_number;
}

Blender::MaybeTextureId Blender::CreateTextureFromEncodedData(const Shared<Data>& data,
                                                              std::optional<SkAlphaType> alpha_type,
                                                              const std::string& annotation)
{
    CHECK(data);
    return texture_manager_->Create([data, alpha_type](const Unique<TextureFactory>& factory) {
        return factory->MakeFromEncodedData(data, alpha_type);
    }, annotation);
}

Blender::MaybeTextureId Blender::CreateTextureFromImage(const sk_sp<SkImage>& image,
                                                        const std::string& annotation)
{
    CHECK(image);
    return texture_manager_->Create([image](const Unique<TextureFactory>& factory) {
        return factory->MakeFromImage(image);
    }, annotation);
}

Blender::MaybeTextureId Blender::CreateTextureFromPixmap(const void *pixels,
                                                         const SkImageInfo& image_info,
                                                         const std::string& annotation)
{
    CHECK(pixels);
    return texture_manager_->Create([pixels, image_info](const Unique<TextureFactory>& factory) {
        SkPixmap pixmap(image_info, pixels, image_info.minRowBytes());
        return factory->MakeFromPixmap(pixmap);
    }, annotation);
}

void Blender::Dispose()
{
    if (disposed_)
        return;

    output_surface_->Disconnect(surface_resize_slot_id_);
    output_surface_->Disconnect(surface_frame_slot_id_);

    // That the frame is in pending state means we have called the
    // `RenderTarget::Begin` function, which expects a corresponding
    // `RenderTarget::Submit` call. But the slot of `frame` signal
    // has been disconnected and `Submit` will never be called,
    // so call it manually here to make sure every `Begin` call has a
    // corresponding `Submit` call.
    if (frame_schedule_state_ == FrameScheduleState::kPendingFrame)
        SurfaceFrameSlot();

    raster_cache_.reset();
    texture_manager_.reset();
    output_surface_.reset();

    frame_schedule_state_ = FrameScheduleState::kDisposed;
    disposed_ = true;
}

void Blender::Trace(GraphicsResourcesTrackable::Tracer *tracer) noexcept
{
    CHECK(!disposed_);

    // `LayerTree` only contains raw data and texture references which are from
    // user defined data (generally from the CanvasKit module in JavaScript land),
    // and there is no need to trace them.

    tracer->TraceMember("TextureManager", texture_manager_.get());
    tracer->TraceMember("RasterCache", raster_cache_.get());
}

void Blender::PurgeRasterCacheResources()
{
    raster_cache_->PurgeAllCaches();
}

GLAMOR_NAMESPACE_END
