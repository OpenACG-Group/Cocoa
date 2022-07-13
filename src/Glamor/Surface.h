#ifndef COCOA_GLAMOR_SURFACE_H
#define COCOA_GLAMOR_SURFACE_H

#include "include/core/SkColor.h"
#include "include/core/SkSurface.h"
#include "include/core/SkRegion.h"

#include "Glamor/Glamor.h"
#include "Glamor/RenderClientObject.h"
#include "Glamor/FrameNotificationRouter.h"
GLAMOR_NAMESPACE_BEGIN

#define CROP_SURFACE_CLOSE                          1
#define CROP_SURFACE_RESIZE                         2
#define CROP_SURFACE_SET_TITLE                      3
#define CROP_SURFACE_GET_BUFFERS_DESCRIPTOR         4
#define CROP_SURFACE_REQUEST_NEXT_FRAME             5

/* Emitted when window is actually closed. */
#define CRSI_SURFACE_CLOSED             1

/* Emitted after resizing is completed during `Surface::Resize` invocation. */
#define CRSI_SURFACE_RESIZE             2

/* Emitted when window manager notifies us the window should be reconfigured. */
#define CRSI_SURFACE_CONFIGURE          3

/* Emitted when window manager notifies us the window should be closed. */
#define CRSI_SURFACE_CLOSE              4

/**
 * Emitted when it is a good time to start submitting a new frame.
 * This signal is actually emitted by the implementation of RenderTarget,
 * for example, in Wayland/WaylandSHMRenderTarget.cc.
 */
#define CRSI_SURFACE_FRAME              5

class RenderTarget;
class Display;

enum class ToplevelStates : uint32_t
{
    kMaximized      = (1 << 1),
    kFullscreen     = (1 << 2),
    kResizing       = (1 << 3),
    kActivated      = (1 << 4),
    kTiledLeft      = (1 << 5),
    kTiledRight     = (1 << 6),
    kTiledTop       = (1 << 7),
    kTiledBottom    = (1 << 8)
};

class Surface : public RenderClientObject,
                public FrameNotificationRouter
{
public:
    explicit Surface(Shared<RenderTarget> rt);
    ~Surface() override;

    g_nodiscard g_inline Shared<Display> GetDisplay() const {
        return display_.lock();
    }

    g_nodiscard g_inline Shared<RenderTarget> GetRenderTarget() const {
        return render_target_;
    }

    g_nodiscard g_inline bool IsClosed() const {
        return has_disposed_;
    }

    g_async_api void Close();
    g_async_api bool Resize(int32_t width, int32_t height);
    g_async_api void SetTitle(const std::string_view& title);
    g_async_api std::string GetBuffersDescriptor();
    g_async_api uint32_t RequestNextFrame();

    g_sync_api g_nodiscard int32_t GetWidth() const;
    g_sync_api g_nodiscard int32_t GetHeight() const;
    g_sync_api g_nodiscard SkColorType GetColorType() const;

    g_sync_api const SkMatrix& GetRootTransformation() const;

protected:
    virtual void OnClose() = 0;
    virtual void OnSetTitle(const std::string_view& title) = 0;
    virtual const SkMatrix& OnGetRootTransformation() const;
    void OnFrameNotification(uint32_t sequence) override;

private:
    bool                            has_disposed_;
    Shared<RenderTarget>            render_target_;
    Weak<Display>                   display_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_SURFACE_H
