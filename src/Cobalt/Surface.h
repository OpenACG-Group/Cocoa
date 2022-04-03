#ifndef COCOA_COBALT_SURFACE_H
#define COCOA_COBALT_SURFACE_H

#include "include/core/SkColor.h"
#include "include/core/SkSurface.h"
#include "include/core/SkRegion.h"

#include "Cobalt/Cobalt.h"
#include "Cobalt/RenderClientObject.h"
COBALT_NAMESPACE_BEGIN

#define CROP_SURFACE_CLOSE                          1
#define CROP_SURFACE_RESIZE                         2
#define CROP_SURFACE_SET_TITLE                      3
#define CROP_SURFACE_GET_BUFFERS_DESCRIPTOR         4

#define CRSI_SURFACE_CLOSED             1
#define CRSI_SURFACE_RESIZE             2
#define CRSI_SURFACE_CONFIGURE          3
#define CRSI_SURFACE_CLOSE              4

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

class Surface : public RenderClientObject
{
public:
    explicit Surface(co_sp<RenderTarget> rt);
    ~Surface() override;

    g_nodiscard g_inline co_sp<Display> GetDisplay() const {
        return display_.lock();
    }

    g_nodiscard g_inline co_sp<RenderTarget> GetRenderTarget() const {
        return render_target_;
    }

    g_nodiscard g_inline bool IsClosed() const {
        return has_disposed_;
    }

    g_async_api void Close();
    g_async_api bool Resize(int32_t width, int32_t height);
    g_async_api void SetTitle(const std::string_view& title);
    g_async_api std::string GetBuffersDescriptor();

    g_sync_api g_nodiscard int32_t GetWidth() const;
    g_sync_api g_nodiscard int32_t GetHeight() const;
    g_sync_api g_nodiscard SkColorType GetColorType() const;

protected:
    virtual void OnClose() = 0;
    virtual void OnSetTitle(const std::string_view& title) = 0;

private:
    bool                            has_disposed_;
    co_sp<RenderTarget>             render_target_;
    co_weak<Display>                display_;
};

COBALT_NAMESPACE_END
#endif //COCOA_COBALT_SURFACE_H
