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

#ifndef COCOA_GLAMOR_SURFACE_H
#define COCOA_GLAMOR_SURFACE_H

#include "include/core/SkColor.h"
#include "include/core/SkSurface.h"
#include "include/core/SkRegion.h"

#include "Glamor/Glamor.h"
#include "Glamor/RenderClientObject.h"
#include "Glamor/FrameNotificationRouter.h"
#include "Glamor/GraphicsResourcesTrackable.h"
GLAMOR_NAMESPACE_BEGIN

#define GLOP_SURFACE_CLOSE                          1
#define GLOP_SURFACE_RESIZE                         2
#define GLOP_SURFACE_SET_TITLE                      3
#define GLOP_SURFACE_GET_BUFFERS_DESCRIPTOR         4
#define GLOP_SURFACE_REQUEST_NEXT_FRAME             5
#define GLOP_SURFACE_SET_MIN_SIZE                   6
#define GLOP_SURFACE_SET_MAX_SIZE                   7
#define GLOP_SURFACE_SET_MAXIMIZED                  8
#define GLOP_SURFACE_SET_MINIMIZED                  9
#define GLOP_SURFACE_SET_FULLSCREEN                 10
#define GLOP_SURFACE_CREATE_BLENDER                 11
#define GLOP_SURFACE_SET_ATTACHED_CURSOR            12

//! Emitted when the window is actually closed.
//! @prototype (void) -> void
#define GLSI_SURFACE_CLOSED             1

//! Emitted after resizing is completed during `Surface::Resize` invocation.
//! @prototype (i32 width, i32 height) -> void
#define GLSI_SURFACE_RESIZE             2

//! Emitted when window manager notifies us the window should be reconfigured.
//! @prototype (i32 width, i32 height, ToplevelStates status) -> void
#define GLSI_SURFACE_CONFIGURE          3

//! Emitted when window manager notifies us the window should be closed.
//! @prototype (void) -> void
#define GLSI_SURFACE_CLOSE              4

//! Emitted when it is a good time to start submitting a new frame.
//! This signal is actually emitted by the implementation of RenderTarget.
//! @prototype (void) -> void
#define GLSI_SURFACE_FRAME              5

//! Emitted when a pointer device enters the window area.
//! @prototype (bool hovered) -> void
#define GLSI_SURFACE_POINTER_HOVERING   6

//! Emitted when a pointer moves on the corresponding surface
//! @prototype (double x, double y) -> void
#define GLSI_SURFACE_POINTER_MOTION     7

//! Emitted when a button of the hovering pointer device is pressed or released
//! @prototype (PointerButton button, bool pressed) -> void
#define GLSI_SURFACE_POINTER_BUTTON     8

class RenderTarget;
class Display;
class Monitor;
class Blender;
class Cursor;

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
                public FrameNotificationRouter,
                public GraphicsResourcesTrackable
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

    g_async_api Shared<Blender> CreateBlender();

    g_async_api bool Resize(int32_t width, int32_t height);
    g_async_api void SetMaxSize(int32_t width, int32_t height);
    g_async_api void SetMinSize(int32_t width, int32_t height);
    g_async_api void SetMaximized(bool value);
    g_async_api void SetMinimized(bool value);

    /**
     * Make current surface into fullscreen state on a specific monitor.
     * `monitor` is nullable if `value` is `false`, which means caller is attempting to
     * unset the fullscreen state.
     */
    g_async_api void SetFullscreen(bool value, const Shared<Monitor>& monitor);

    g_async_api void SetTitle(const std::string_view& title);

    g_async_api std::string GetBuffersDescriptor();

    g_async_api uint32_t RequestNextFrame();

    g_sync_api g_nodiscard int32_t GetWidth() const;
    g_sync_api g_nodiscard int32_t GetHeight() const;
    g_sync_api g_nodiscard SkColorType GetColorType() const;

    g_sync_api const SkMatrix& GetRootTransformation() const;

    g_sync_api void SetAttachedCursor(const Shared<Cursor>& cursor);

    g_private_api const Shared<Cursor>& GetAttachedCursor() const {
        return attached_cursor_;
    }

    void Trace(GraphicsResourcesTrackable::Tracer *tracer) noexcept override;

protected:
    virtual void OnClose() = 0;
    virtual void OnSetTitle(const std::string_view& title) = 0;
    virtual void OnSetMaxSize(int32_t width, int32_t height) = 0;
    virtual void OnSetMinSize(int32_t width, int32_t height) = 0;
    virtual void OnSetMaximized(bool value) = 0;
    virtual void OnSetMinimized(bool value) = 0;
    virtual void OnSetFullscreen(bool value, const Shared<Monitor>& monitor) = 0;
    virtual const SkMatrix& OnGetRootTransformation() const;
    virtual void OnSetCursor(const Shared<Cursor>& cursor) = 0;
    void OnFrameNotification(uint32_t sequence) override;

private:
    bool                            has_disposed_;
    Shared<RenderTarget>            render_target_;
    Weak<Display>                   display_;
    Shared<Cursor>                  attached_cursor_;
    Weak<Blender>                   weak_blender_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_SURFACE_H
