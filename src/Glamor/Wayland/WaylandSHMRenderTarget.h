#ifndef COCOA_GLAMOR_WAYLAND_WAYLANDSHMRENDERTARGET_H
#define COCOA_GLAMOR_WAYLAND_WAYLANDSHMRENDERTARGET_H

#include "include/core/SkRect.h"
#include "include/core/SkRegion.h"

#include "Glamor/Glamor.h"
#include "Glamor/Wayland/WaylandRenderTarget.h"
#include "Glamor/Wayland/WaylandDisplay.h"
GLAMOR_NAMESPACE_BEGIN

class WaylandSHMRenderTarget : public WaylandRenderTarget
{
public:
    enum class BufferState
    {
        kCommitted,
        kDrawing,
        kFree,
        kDeferredDestroying
    };

    struct Buffer
    {
        BufferState         state;
        SkRegion            damage;
        wl_buffer          *buffer;
        void               *ptr;
        size_t              size;
        sk_sp<SkSurface>    surface;
        Shared<wl_shm_pool>  shared_pool;
        WaylandSHMRenderTarget *rt;
    };

    static Shared<WaylandSHMRenderTarget> Make(const Shared<WaylandDisplay>& display,
                                               int32_t width, int32_t height, SkColorType format);

    WaylandSHMRenderTarget(const Shared<WaylandDisplay>& display, int32_t width, int32_t height,
                           SkColorType format);
    ~WaylandSHMRenderTarget() override;

    SkSurface *OnBeginFrame() override;
    void OnSubmitFrame(SkSurface *surface, const SkRegion& damage) override;
    void OnResize(int32_t width, int32_t height) override;
    sk_sp<SkSurface> OnCreateOffscreenBackendSurface(const SkImageInfo& info) override;

    std::string GetBufferStateDescriptor() override;
    void OnClearFrameBuffers() override;

    static void BufferReleaseCallback(void *data, wl_buffer *buffer);
    static void FrameDoneCallback(void *data, wl_callback *cb, uint32_t extraData);

private:
    void ReleaseAllBuffers(bool forceRelease);
    void AllocateAppendBuffers(int32_t count, int32_t width, int32_t height, SkColorType format);
    int32_t GetNextDrawingBuffer();

    std::vector<Unique<Buffer>>       buffers_;
    std::vector<Unique<Buffer>>       deferred_destructing_buffers_;
    int32_t                              drawing_buffer_idx_;
    int32_t                              committed_buffer_idx_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_WAYLAND_WAYLANDSHMRENDERTARGET_H
