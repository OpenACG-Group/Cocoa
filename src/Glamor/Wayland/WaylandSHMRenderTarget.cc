#include <linux/memfd.h>
#include <sys/mman.h>
#include <sys/syscall.h>

#include "include/core/SkCanvas.h"

#include "Core/Journal.h"
#include "Glamor/Wayland/WaylandUtils.h"
#include "Glamor/Wayland/WaylandDisplay.h"
#include "Glamor/Wayland/WaylandSHMRenderTarget.h"
GLAMOR_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Glamor.Wayland.SHMRenderTarget)

#define RT_INITIAL_BUFFERS  3
#define RT_EMPTY_INDEX      (-1)

Shared<WaylandSHMRenderTarget>
WaylandSHMRenderTarget::Make(const Shared<WaylandDisplay>& display,
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

    auto renderTarget = std::make_shared<WaylandSHMRenderTarget>(display, width, height, format);

    renderTarget->wl_event_queue_ = wl_display_create_queue(display->GetWaylandDisplay());

    /* Allocate shm buffers */
    renderTarget->AllocateAppendBuffers(RT_INITIAL_BUFFERS, width, height, format);
    renderTarget->buffers_[0]->state = BufferState::kDrawing;
    renderTarget->drawing_buffer_idx_ = 0;
    renderTarget->committed_buffer_idx_ = RT_EMPTY_INDEX;

    wl_compositor *compositor = display->GetGlobalsRef()->wl_compositor_;
    renderTarget->wl_surface_ = wl_compositor_create_surface(compositor);

    if (!renderTarget->wl_surface_)
    {
        QLOG(LOG_ERROR, "Failed to create Wayland compositor surface");
        return nullptr;
    }

    wl_proxy_set_queue(reinterpret_cast<wl_proxy *>(renderTarget->wl_surface_), renderTarget->wl_event_queue_);
    wl_surface_set_user_data(renderTarget->wl_surface_, renderTarget.get());

    return renderTarget;
}

WaylandSHMRenderTarget::WaylandSHMRenderTarget(const Shared<WaylandDisplay>& display,
                                               int32_t width, int32_t height,
                                               SkColorType format)
    : WaylandRenderTarget(display, RenderDevice::kRaster, width, height, format)
    , drawing_buffer_idx_(RT_EMPTY_INDEX)
    , committed_buffer_idx_(RT_EMPTY_INDEX)
{
}

WaylandSHMRenderTarget::~WaylandSHMRenderTarget()
{
    if (wl_surface_)
        wl_surface_destroy(wl_surface_);
    ReleaseAllBuffers(true);

    if (wl_event_queue_)
        wl_event_queue_destroy(wl_event_queue_);
}

namespace {
/**
 * Reference: GTK+ Project (gdk/wayland/gdkdisplay-wayland.c)
 */
int create_shared_memory_fd()
{
    static bool forceShmOpen = false;
    int ret;

#if !defined(__NR_memfd_create)
    forceShmOpen = true;
#endif

    do
    {
#if defined(__NR_memfd_create)
        if (!forceShmOpen)
        {
            int options = MFD_CLOEXEC;
#if defined(MFD_ALLOW_SEALING)
            options = MFD_ALLOW_SEALING;
#endif /* MFD_ALLOW_SEALING */
            ret = memfd_create("Cocoa.Glamor.Wayland.RenderTarget", options);

            if (ret < 0 && errno == ENOSYS)
                forceShmOpen = true;
#if defined(F_ADD_SEALS) && defined(F_SEAL_SHRINK)
            if (ret >= 0)
                fcntl(ret, F_ADD_SEALS, F_SEAL_SHRINK);
#endif /* F_ADD_SEALS && F_SEAL_SHRINK */
        }
#endif /* __NR_memfd_create */

        if (forceShmOpen)
        {
            auto name = fmt::format("/Cocoa.Glamor.Wayland.RenderTarget#{:x}", random());
            ret = shm_open(name.c_str(), O_CREAT | O_EXCL | O_RDWR | O_CLOEXEC, 0600);
            if (ret >= 0)
                shm_unlink(name.c_str());
            else if (errno == EEXIST)
                continue;
        }
    } while (ret < 0 && errno == EINTR);

    if (ret < 0)
    {
        QLOG(LOG_ERROR, "Creating shared memory file (using {}) failed: {}",
             forceShmOpen ? "shm_open" : "memfd_create", strerror(errno));
    }

    return ret;
}

wl_shm_pool *create_shm_pool(wl_shm *shm, size_t size, void **dataPtrOut)
{
    *dataPtrOut = nullptr;

    int fd = create_shared_memory_fd();
    if (fd < 0)
        return nullptr;

    if (ftruncate(fd, static_cast<off_t>(size)) < 0)
    {
        QLOG(LOG_ERROR, "Truncating shared memory file failed: {}", strerror(errno));
        close(fd);
        return nullptr;
    }

    void *data = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED)
    {
        QLOG(LOG_ERROR, "Mapping shared memory file failed: {}", strerror(errno));
        close(fd);
        return nullptr;
    }

    wl_shm_pool *pool = wl_shm_create_pool(shm, fd, static_cast<int32_t>(size));
    close(fd);

    *dataPtrOut = data;
    return pool;
}

const wl_buffer_listener g_buffer_listener = {
    WaylandSHMRenderTarget::BufferReleaseCallback
};

}

void WaylandSHMRenderTarget::BufferReleaseCallback(void *data, g_maybe_unused wl_buffer *buffer)
{
    auto *self = reinterpret_cast<WaylandSHMRenderTarget::Buffer *>(data);

    if (self->state == WaylandSHMRenderTarget::BufferState::kDeferredDestroying)
    {
        CHECK(self->surface->unique());
        wl_buffer_destroy(self->buffer);
        self->surface.reset();
        self->shared_pool.reset();

        auto itr = std::find_if(self->rt->deferred_destructing_buffers_.begin(),
                                self->rt->deferred_destructing_buffers_.end(),
                                [self](const Unique<Buffer>& v) -> bool {
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
    for (Unique<Buffer>& ptr : buffers_)
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
            ptr->shared_pool.reset();
        }
    }
    buffers_.clear();
}

void WaylandSHMRenderTarget::AllocateAppendBuffers(int32_t count, int32_t width, int32_t height,
                                                   SkColorType format)
{
    size_t allocSingleSize = SkColorTypeBytesPerPixel(format) * width * height;
    size_t stride = SkColorTypeBytesPerPixel(format) * width;
    size_t poolAllocSize = allocSingleSize * count;

    void *poolStartAddress = nullptr;
    wl_shm_pool *pool = create_shm_pool(GetDisplay()->Cast<WaylandDisplay>()->GetGlobalsRef()->wl_shm_,
                                        poolAllocSize, &poolStartAddress);

    auto sharedPool = std::shared_ptr<wl_shm_pool>(pool,
                                                   [ptr = poolStartAddress, size = poolAllocSize]
                                                   (wl_shm_pool *pool) {
        wl_shm_pool_destroy(pool);
        munmap(ptr, size);
    });

    for (uint32_t n = 0; n < count; n++)
    {
        auto offset = static_cast<int32_t>(allocSingleSize * n);

        auto buffer = std::make_unique<Buffer>();
        buffer->state = BufferState::kFree;
        buffer->shared_pool = sharedPool;
        buffer->size = allocSingleSize;
        buffer->ptr = reinterpret_cast<uint8_t *>(poolStartAddress) + offset;
        buffer->damage.setEmpty();
        buffer->buffer = wl_shm_pool_create_buffer(pool, offset, width, height, static_cast<int32_t>(stride),
                                                   SkColorTypeToWlShmFormat(format));
        buffer->rt = this;

        SkImageInfo info = SkImageInfo::Make(width, height, format, SkAlphaType::kPremul_SkAlphaType);
        buffer->surface = SkSurface::MakeRasterDirect(info, buffer->ptr, stride);

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
    if (drawing_buffer_idx_ < 0)
        return nullptr;

    Unique<Buffer>& buf = buffers_[drawing_buffer_idx_];
    return buf->surface.get();
}

void WaylandSHMRenderTarget::FrameDoneCallback(void *data, wl_callback *cb,
                                               g_maybe_unused uint32_t extraData)
{
    /* We do not submit next frame until this is called */

    auto *rt = reinterpret_cast<WaylandSHMRenderTarget *>(data);
    rt->committed_buffer_idx_ = RT_EMPTY_INDEX;
    wl_callback_destroy(cb);
}

namespace {

const wl_callback_listener g_frame_callback_listener = {
    WaylandSHMRenderTarget::FrameDoneCallback
};

} // namespace anonymous

void WaylandSHMRenderTarget::OnSubmitFrame(SkSurface *surface, const SkRegion& damage)
{
    CHECK(drawing_buffer_idx_ >= 0);
    if (surface != buffers_[drawing_buffer_idx_]->surface.get())
    {
        QLOG(LOG_ERROR, "Submitting an invalid surface, ignored");
        return;
    }

    if (committed_buffer_idx_ != RT_EMPTY_INDEX || damage.isEmpty())
        return;

    committed_buffer_idx_ = drawing_buffer_idx_;
    drawing_buffer_idx_ = GetNextDrawingBuffer();

    Unique<Buffer>& committed = buffers_[committed_buffer_idx_];
    committed->state = BufferState::kCommitted;
    wl_surface_attach(wl_surface_, committed->buffer, 0, 0);

    for (SkRegion::Iterator itr(damage); !itr.done(); itr.next())
    {
        SkIRect r = itr.rect();
        wl_surface_damage(wl_surface_, r.x(), r.y(), r.width(), r.height());
    }

    wl_callback *frameCallback = wl_surface_frame(wl_surface_);
    wl_callback_add_listener(frameCallback, &g_frame_callback_listener, this);
    wl_surface_commit(wl_surface_);
}

void WaylandSHMRenderTarget::OnResize(int32_t width, int32_t height)
{
    ReleaseAllBuffers(false);
    AllocateAppendBuffers(RT_INITIAL_BUFFERS, width, height, GetColorType());
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
    for (const Unique<Buffer>& buffer : buffers_)
    {
        out.append(fmt::format("#{}:pool={}:addr={}:size={}:", idx++,
                               fmt::ptr(buffer->shared_pool.get()), buffer->ptr, buffer->size));

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
    for (int i = 0; i < RT_INITIAL_BUFFERS; i++)
    {
        SkSurface *pSurface = BeginFrame();
        pSurface->getCanvas()->clear(SK_ColorBLACK);
        Submit(SkRegion(SkIRect::MakeWH(GetWidth(), GetHeight())));
    }
}

sk_sp<SkSurface> WaylandSHMRenderTarget::OnCreateOffscreenBackendSurface(const SkImageInfo& info)
{
    // TODO(sora): Is there a more appropriate way to allocate the pixels?
    //             (like allocate them in shared memory or by Wayland compositor)
    sk_sp<SkSurface> surface = SkSurface::MakeRaster(info);
    return surface;
}

GLAMOR_NAMESPACE_END
