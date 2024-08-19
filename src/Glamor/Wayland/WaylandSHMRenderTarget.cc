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

#include "Core/Journal.h"
#include "Core/TraceEvent.h"
#include "Glamor/Wayland/WaylandUtils.h"
#include "Glamor/Wayland/WaylandDisplay.h"
#include "Glamor/Wayland/WaylandSHMRenderTarget.h"
#include "Glamor/Wayland/WaylandSharedMemoryHelper.h"
#include "Glamor/Wayland/WaylandMonitor.h"
GLAMOR_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Glamor.Wayland.SHMRenderTarget)

#define RT_INITIAL_BUFFERS  3
#define RT_EMPTY_INDEX      (-1)

std::shared_ptr<WaylandSHMRenderTarget>
WaylandSHMRenderTarget::Make(const std::shared_ptr<WaylandDisplay>& display,
                             int32_t width, int32_t height, SkColorType format)
{
    if (format == SkColorType::kUnknown_SkColorType)
    {
        QLOG(LOG_DEBUG, "Failed in creating RenderTarget: invalid color format");
        return nullptr;
    }
    if (width <= 0 || height <= 0)
    {
        QLOG(LOG_DEBUG, "Failed in creating RenderTarget: invalid dimensions ({}, {})", width, height);
        return nullptr;
    }

    auto supportedFormats = display->GetRasterColorFormats();
    auto foundFormat = std::find(supportedFormats.begin(), supportedFormats.end(), format);
    if (foundFormat == supportedFormats.end())
    {
        QLOG(LOG_DEBUG, "Failed in creating RenderTarget: unsupported color format");
        return nullptr;
    }

    wl_event_queue *event_queue = wl_display_create_queue(display->GetWaylandDisplay());
    if (!event_queue)
    {
        QLOG(LOG_ERROR, "Failed to create an event queue for render target");
        return nullptr;
    }

    wl_compositor *compositor = display->GetGlobalsRef()->wl_compositor_;
    wl_surface *surface = wl_compositor_create_surface(compositor);
    if (!surface)
    {
        QLOG(LOG_ERROR, "Failed to create Wayland compositor surface");
        return nullptr;
    }
    wl_proxy_set_queue(reinterpret_cast<wl_proxy *>(surface), event_queue);

    auto rt = std::make_shared<WaylandSHMRenderTarget>(display, width, height, format, surface, event_queue);

    /* Allocate shm buffers */
    rt->AllocateAppendBuffers(RT_INITIAL_BUFFERS, width, height, format);
    rt->buffers_[0]->state = BufferState::kDrawing;
    rt->drawing_buffer_idx_ = 0;
    rt->committed_buffer_idx_ = RT_EMPTY_INDEX;

    return rt;
}

WaylandSHMRenderTarget::WaylandSHMRenderTarget(const std::shared_ptr<Display>& display,
                                               int32_t width,
                                               int32_t height,
                                               SkColorType format,
                                               wl_surface *surface,
                                               wl_event_queue *surface_queue)
    : WaylandRenderTarget(display, RenderDevice::kRaster, width, height, format,
                          surface, surface_queue)
    , drawing_buffer_idx_(RT_EMPTY_INDEX)
    , committed_buffer_idx_(RT_EMPTY_INDEX)
    , wl_frame_callback_(nullptr)
{
}

WaylandSHMRenderTarget::~WaylandSHMRenderTarget()
{
    ReleaseAllBuffers(true);
}

namespace {

const wl_buffer_listener g_buffer_listener = {
    WaylandSHMRenderTarget::BufferReleaseCallback
};

} // namespace anonymous

void WaylandSHMRenderTarget::BufferReleaseCallback(void *data, g_maybe_unused wl_buffer *buffer)
{
    auto *self = reinterpret_cast<WaylandSHMRenderTarget::Buffer *>(data);

    if (self->state == WaylandSHMRenderTarget::BufferState::kDeferredDestroying)
    {
        CHECK(self->surface->unique());
        wl_buffer_destroy(self->buffer);
        self->surface.reset();
        self->shared_pool_helper.reset();

        auto itr = std::find_if(self->rt->deferred_destructing_buffers_.begin(),
                                self->rt->deferred_destructing_buffers_.end(),
                                [self](const std::unique_ptr<Buffer>& v) -> bool {
            return (v.get() == self);
        });

        CHECK(itr != self->rt->deferred_destructing_buffers_.end());
        self->rt->deferred_destructing_buffers_.erase(itr);
    }
    else
    {
        self->state = WaylandSHMRenderTarget::BufferState::kFree;
    }
}

void WaylandSHMRenderTarget::ReleaseAllBuffers(bool forceRelease)
{
    for (std::unique_ptr<Buffer>& ptr : buffers_)
    {
        if (!forceRelease && ptr->state == BufferState::kCommitted)
        {
            ptr->state = BufferState::kDeferredDestroying;
            deferred_destructing_buffers_.push_back(std::move(ptr));
        }
        else
        {
            CHECK(ptr->surface->unique());
            wl_buffer_destroy(ptr->buffer);
            ptr->surface.reset();
            ptr->shared_pool_helper.reset();
        }
    }
    buffers_.clear();
}

void WaylandSHMRenderTarget::AllocateAppendBuffers(int32_t count, int32_t width, int32_t height,
                                                   SkColorType format)
{
    std::optional<MonitorSubpixel> subpixel;
    for (const auto& monitor : GetDisplay()->RequestMonitorList())
    {
        MonitorSubpixel cur = monitor->GetCurrentProperties().subpixel;
        if (subpixel && cur != subpixel)
        {
            subpixel = MonitorSubpixel::kUnknown;
            break;
        }
        subpixel = cur;
    }

    SkPixelGeometry sk_subpixel;
    switch (*subpixel)
    {
    case MonitorSubpixel::kUnknown:
    case MonitorSubpixel::kNone:
        sk_subpixel = SkPixelGeometry::kUnknown_SkPixelGeometry;
        break;
    case MonitorSubpixel::kHorizontalRGB:
        sk_subpixel = SkPixelGeometry::kRGB_H_SkPixelGeometry;
        break;
    case MonitorSubpixel::kHorizontalBGR:
        sk_subpixel = SkPixelGeometry::kBGR_H_SkPixelGeometry;
        break;
    case MonitorSubpixel::kVerticalRGB:
        sk_subpixel = SkPixelGeometry::kRGB_V_SkPixelGeometry;
        break;
    case MonitorSubpixel::kVerticalBGR:
        sk_subpixel = SkPixelGeometry::kBGR_V_SkPixelGeometry;
        break;
    }

    size_t allocSingleSize = SkColorTypeBytesPerPixel(format) * width * height;
    size_t stride = SkColorTypeBytesPerPixel(format) * width;
    size_t poolAllocSize = allocSingleSize * count;

    auto wl_shm = GetDisplay()->As<WaylandDisplay>()->GetGlobalsRef()->wl_shm_;
    auto sharedPool = WaylandSharedMemoryHelper::Make(wl_shm, poolAllocSize,
                                                      WaylandSharedMemoryHelper::kRasterRenderTarget_Role);
    void *poolStartAddress = sharedPool->GetMappedAddress();

    for (uint32_t n = 0; n < count; n++)
    {
        auto offset = static_cast<int32_t>(allocSingleSize * n);

        auto buffer = std::make_unique<Buffer>();
        buffer->state = BufferState::kFree;
        buffer->shared_pool_helper = sharedPool;
        buffer->size = allocSingleSize;
        buffer->ptr = reinterpret_cast<uint8_t *>(poolStartAddress) + offset;
        buffer->damage.setEmpty();
        buffer->buffer = wl_shm_pool_create_buffer(sharedPool->GetShmPool(),
                                                   offset, width, height, static_cast<int32_t>(stride),
                                                   SkColorTypeToWlShmFormat(format));
        buffer->rt = this;

        SkImageInfo info = SkImageInfo::Make(width, height, format, SkAlphaType::kPremul_SkAlphaType);
        SkSurfaceProps props(0, sk_subpixel);
        buffer->surface = SkSurfaces::WrapPixels(info, buffer->ptr, stride, &props);

        wl_buffer_add_listener(buffer->buffer, &g_buffer_listener, buffer.get());

        buffers_.push_back(std::move(buffer));
    }
}

int32_t WaylandSHMRenderTarget::GetNextDrawingBuffer()
{
    for (int32_t i = 0; i < buffers_.size(); i++)
    {
        if (buffers_[i]->state == BufferState::kFree)
        {
            buffers_[i]->state = BufferState::kDrawing;
            return i;
        }
    }

    auto last = static_cast<int32_t>(buffers_.size());
    AllocateAppendBuffers(2, GetWidth(), GetHeight(), GetColorType());

    buffers_[last]->state = BufferState::kDrawing;
    return last;
}

SkSurface *WaylandSHMRenderTarget::OnBeginFrame()
{
    TRACE_EVENT("present", "WaylandSHMRenderTarget::OnBeginFrame");

    if (drawing_buffer_idx_ < 0)
        return nullptr;

    std::unique_ptr<Buffer>& buf = buffers_[drawing_buffer_idx_];
    return buf->surface.get();
}

void WaylandSHMRenderTarget::FrameDoneCallback(void *data, wl_callback *cb,
                                               g_maybe_unused uint32_t extraData)
{
    /* We do not submit next frame until this is called */

    auto *rt = static_cast<WaylandSHMRenderTarget*>(data);
    rt->committed_buffer_idx_ = RT_EMPTY_INDEX;
    rt->wl_frame_callback_ = nullptr;
    wl_callback_destroy(cb);
}

namespace {

const wl_callback_listener g_frame_callback_listener = {
    WaylandSHMRenderTarget::FrameDoneCallback
};

} // namespace anonymous

void WaylandSHMRenderTarget::OnSubmitFrame(SkSurface *surface,
                                           const FrameSubmitInfo &submit_info)
{
    // Drawing operations on the raster backend is always performed
    // immediately. We don't need to "submit" the drawing commands
    // as they are not deferred.
}

void WaylandSHMRenderTarget::OnPresentFrame(SkSurface *surface,
                                            const FrameSubmitInfo& submit_info)
{
    TRACE_EVENT("present", "WaylandSHMRenderTarget::OnPresentFrame");

    CHECK(drawing_buffer_idx_ >= 0);
    if (surface != buffers_[drawing_buffer_idx_]->surface.get())
    {
        QLOG(LOG_ERROR, "Submitting an invalid surface, ignored");
        return;
    }

    if (committed_buffer_idx_ != RT_EMPTY_INDEX)
        return;

    committed_buffer_idx_ = drawing_buffer_idx_;
    drawing_buffer_idx_ = GetNextDrawingBuffer();

    std::unique_ptr<Buffer>& committed = buffers_[committed_buffer_idx_];
    committed->state = BufferState::kCommitted;
    wl_surface_attach(wl_surface_, committed->buffer, 0, 0);

    /*
    const SkRegion& damage = submit_info.damage_region;
    for (SkRegion::Iterator itr(submit_info.damage_region); !itr.done(); itr.next())
    {
        SkIRect r = itr.rect();
        wl_surface_damage(wl_surface_, r.x(), r.y(), r.width(), r.height());
    }
    */

    // XXX(sora): As we use multiple framebuffers for rendering, each frame
    // a new framebuffer that is different from the previous frame is used.
    // However, `submit_info.damage_region` represents the damage information
    // of the current frame relative to the previous frame, which is not the
    // actual damage of the submitting framebuffer. If we submit that wrong
    // damage info, Wayland compositor may give us a weird rendering result.
    // As a mitigation, we just tell the compositor that the whole frame is
    // damaged so that it will repaint the whole window.
    wl_surface_damage(wl_surface_, 0, 0, GetWidth(), GetHeight());

    wl_callback *callback = wl_surface_frame(wl_surface_);
    wl_callback_add_listener(callback, &g_frame_callback_listener, this);
    wl_surface_commit(wl_surface_);
    wl_frame_callback_ = callback;
}

void WaylandSHMRenderTarget::OnTryCancelCurrentFrameRequest()
{
    if (wl_frame_callback_)
    {
        committed_buffer_idx_ = RT_EMPTY_INDEX;
        wl_callback_destroy(wl_frame_callback_);
        wl_frame_callback_ = nullptr;
    }

    WaylandRenderTarget::OnTryCancelCurrentFrameRequest();
}

void WaylandSHMRenderTarget::OnContentBufferDimensionsUpdate(const SkISize& dimensions)
{
    OnTryCancelCurrentFrameRequest();
    ReleaseAllBuffers(false);
    AllocateAppendBuffers(RT_INITIAL_BUFFERS, dimensions.width(), dimensions.height(), GetColorType());
    buffers_[0]->state = BufferState::kDrawing;
    drawing_buffer_idx_ = 0;
    committed_buffer_idx_ = RT_EMPTY_INDEX;
    OnClearFrameBuffers();
}

std::string WaylandSHMRenderTarget::GetBufferStateDescriptor()
{
    // #<idx>:pool=<pool>:addr=<addr>:size=<size>:<status>

    std::string out;
    int32_t idx = 0;
    for (const std::unique_ptr<Buffer>& buffer : buffers_)
    {
        out.append(fmt::format("#{}:pool={}:addr={}:size={}:", idx++,
                               fmt::ptr(buffer->shared_pool_helper.get()), buffer->ptr, buffer->size));

        switch (buffer->state)
        {
        case BufferState::kCommitted:
            out.append("committed");
            break;
        case BufferState::kDrawing:
            out.append("drawing");
            break;
        case BufferState::kFree:
            out.append("free");
            break;
        case BufferState::kDeferredDestroying:
            out.append("destroying");
            break;
        }

        if (buffer != buffers_.back())
            out.push_back('|');
    }

    return out;
}

void WaylandSHMRenderTarget::OnClearFrameBuffers()
{
    FrameSubmitInfo submit_info{
        .damage_region = SkRegion(SkIRect::MakeWH(GetWidth(), GetHeight()))
    };

    for (int i = 0; i < RT_INITIAL_BUFFERS; i++)
    {
        SkSurface *pSurface = BeginFrame();
        pSurface->getCanvas()->clear(SK_ColorBLACK);
        Submit(submit_info);
        Present();
    }
}

sk_sp<SkSurface> WaylandSHMRenderTarget::OnCreateOffscreenBackendSurface(const SkImageInfo& info)
{
    // TODO(sora): Is there a more appropriate way to allocate the pixels?
    //             (like allocate them in shared memory or by Wayland compositor)
    sk_sp<SkSurface> surface = SkSurfaces::Raster(info);
    return surface;
}

void WaylandSHMRenderTarget::Trace(GraphicsResourcesTrackable::Tracer *tracer) noexcept
{
    WaylandRenderTarget::Trace(tracer);

    if (!buffers_.empty())
    {
        std::shared_ptr<WaylandSharedMemoryHelper> pool = buffers_[0]->shared_pool_helper;
        CHECK(pool);

        tracer->TraceResource("Wayland shared memory pool",
                              TRACKABLE_TYPE_POOL,
                              TRACKABLE_DEVICE_CPU,
                              TRACKABLE_OWNERSHIP_SHARED,
                              TraceIdFromPointer(pool->GetMappedAddress()),
                              pool->GetPoolSize());
    }

    int32_t index = 0;
    for (const auto& buffer : buffers_)
    {
        tracer->TraceResource(fmt::format("Buffer#{}", index++),
                              TRACKABLE_TYPE_REPRESENT,
                              TRACKABLE_DEVICE_CPU,
                              TRACKABLE_OWNERSHIP_SHARED,
                              TraceIdFromPointer(buffer->ptr),
                              buffer->size);
    }
}

GLAMOR_NAMESPACE_END
