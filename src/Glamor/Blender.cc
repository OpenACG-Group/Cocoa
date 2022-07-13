#include <future>

#include "include/core/SkCanvas.h"
#include "include/core/SkDeferredDisplayListRecorder.h"
#include "include/utils/SkNWayCanvas.h"

#include "Core/OutOfLoopThreadPool.h"
#include "Core/Journal.h"
#include "Glamor/Blender.h"
#include "Glamor/RenderTarget.h"
#include "Glamor/Surface.h"
#include "Glamor/HWComposeTileFrameGenerator.h"
#include "Glamor/HWComposeSwapchain.h"
#include "Glamor/RasterFrameGenerator.h"

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

namespace {

std::unique_ptr<FrameGeneratorBase> create_frame_generator(const Shared<Blender>& blender)
{
    CHECK(blender);
    Shared<Surface> surface = blender->GetOutputSurface();

    Shared<RenderTarget> rt = surface->GetRenderTarget();
    using RDT = RenderTarget::RenderDevice;

    std::unique_ptr<FrameGeneratorBase> FG;

    if (rt->GetRenderDeviceType() == RDT::kHWComposer)
    {
        FG = std::make_unique<HWComposeTileFrameGenerator>(blender);
        QLOG(LOG_DEBUG, "Using frame generator HWComposeTileFrameGenerator, tiled-base rendering");
    }
    else
    {
        FG = std::make_unique<RasterFrameGenerator>(blender);
        QLOG(LOG_DEBUG, "Using frame generator RasterFrameGenerator");
    }

    return FG;
}

} // namespace anonymous

Shared<Blender> Blender::Make(const Shared<Surface>& surface)
{
    auto blender = std::make_shared<Blender>(surface);
    CHECK(blender);

    blender->frame_generator_ = create_frame_generator(blender);
    if (blender->frame_generator_ == nullptr)
    {
        QLOG(LOG_ERROR, "Failed to create frame generator which is required by Blender");
        return nullptr;
    }

    return blender;
}

Blender::Blender(const Shared<Surface>& surface)
    : RenderClientObject(RealType::kBlender)
    , disposed_(false)
    , surface_resize_slot_id_(0)
    , surface_frame_slot_id_(0)
    , output_surface_(surface)
    , frame_generator_(nullptr)
    , layer_tree_(std::make_unique<LayerTree>(SkISize::Make(surface->GetWidth(), surface->GetWidth())))
    , current_dirty_rect_(SkIRect::MakeEmpty())
    , frame_schedule_state_(FrameScheduleState::kIdle)
{
    CHECK(surface);

    SetMethodTrampoline(CROP_BLENDER_DISPOSE, Blender_Dispose_Trampoline);
    SetMethodTrampoline(CROP_BLENDER_UPDATE, Blender_Update_Trampoline);

    surface_resize_slot_id_ = surface->Connect(CRSI_SURFACE_RESIZE, [this](RenderHostSlotCallbackInfo& info) {
        this->SurfaceResizeSlot(info.Get<int32_t>(0), info.Get<int32_t>(1));
    }, true);

    surface_frame_slot_id_ = surface->Connect(CRSI_SURFACE_FRAME, [this](RenderHostSlotCallbackInfo& info) {
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

    frame_schedule_state_ = FrameScheduleState::kPresented;
}

void Blender::Update(const Shared<LayerTree> &layer_tree)
{
    if (frame_schedule_state_ == FrameScheduleState::kPendingFrame)
    {
        QLOG(LOG_WARNING, "Frame scheduler: frame is dropped (updating in PendingFrame state)");
        return;
    }

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
        .cull_rect = SkRect::MakeEmpty()
    };

    if (!layer_tree_->Preroll(&context))
    {
        QLOG(LOG_ERROR, "Preroll stage was cancelled, no contents will be represented");
        return;
    }

    // Now `RasterCache` and paint boundaries of each layer are prepared,
    // and we start painting (rasterization).

    SkSurface *frame_surface = rt->BeginFrame();

    SkNWayCanvas composed_canvas(frame_surface->width(), frame_surface->height());
    composed_canvas.addCanvas(frame_surface->getCanvas());
    for (const Shared<RasterDrawOpObserver>& observer : layer_tree_->GetObservers())
    {
        observer->BeginFrame();
        composed_canvas.addCanvas(observer.get());
    }

    Layer::PaintContext paintContext {
        .gr_context = gr_context,
        .root_surface_transformation = output_surface_->GetRootTransformation(),
        .frame_canvas = frame_surface->getCanvas(),
        .composed_canvas = &composed_canvas,
        .cull_rect = context.cull_rect,
        .has_gpu_retained_resource = false
    };

    layer_tree_->Paint(&paintContext);

    // At last, we request a new frame from WSI layer. We will be notified
    // (slot function `SurfaceFrameSlot` will be called) later
    // when it is a good time to present a new frame (VSync).
    output_surface_->RequestNextFrame();

    frame_schedule_state_ = FrameScheduleState::kPendingFrame;
}

void Blender::SurfaceResizeSlot(g_maybe_unused int32_t width,
                                g_maybe_unused int32_t height)
{
    layer_tree_->SetFrameSize(SkISize::Make(width, height));
    frame_generator_.reset();
    frame_generator_ = create_frame_generator(Self()->As<Blender>());
}

void Blender::Dispose()
{
    if (disposed_)
        return;

    output_surface_->Disconnect(surface_resize_slot_id_);
    output_surface_->Disconnect(surface_frame_slot_id_);
    frame_generator_.reset();
    output_surface_.reset();

    frame_schedule_state_ = FrameScheduleState::kDisposed;
    disposed_ = true;
}

GLAMOR_NAMESPACE_END
