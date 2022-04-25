#ifndef COCOA_COBALT_RENDERTARGET_H
#define COCOA_COBALT_RENDERTARGET_H

#include "include/core/SkColor.h"
#include "include/core/SkSurface.h"
#include "include/core/SkRegion.h"

#include "Core/Errors.h"
#include "Cobalt/Cobalt.h"
COBALT_NAMESPACE_BEGIN

class Display;

/**
 * @p RenderTarget is the underlying class of @p Surface.
 * Different from @p Surface, @p RenderTarget only takes the responsibility
 * related to rendering, like buffer management, hardware-acceleration, etc.
 * Other necessary features like input handling are implemented by Surface.
 *
 * @p RenderTarget is an internal class which can not be used out of Cobalt.
 */
class RenderTarget
{
public:
    enum class RenderDevice
    {
        /* Vulkan backend with GPU acceleration */
        kHWComposer,

        /* Skia CPU rasterizer backend */
        kRaster
    };

    class ScopedFrame
    {
    public:
        explicit ScopedFrame(co_sp<RenderTarget> rt)
            : render_target_(std::move(rt)), surface_(nullptr)
        {
            surface_ = render_target_->BeginFrame();
            CHECK(surface_ && "Failed to acquire a new graphics frame");
        }

        ~ScopedFrame() {
            render_target_->Submit(dirty_region_);
        }

        void MarkWholeFrameDirty() {
            SkISize size = surface_->imageInfo().dimensions();
            dirty_region_ = SkRegion(SkIRect::MakeSize(size));
        }

        void MarkDirtyRegion(const SkRegion& region) {
            dirty_region_ = region;
        }

        SkSurface *GetSurface() {
            return surface_;
        }

    private:
        co_sp<RenderTarget>     render_target_;
        SkSurface              *surface_;
        SkRegion                dirty_region_;
    };

    RenderTarget(const co_sp<Display>& display, RenderDevice device,
                 int32_t width, int32_t height, SkColorType format);
    virtual ~RenderTarget() = default;

    g_nodiscard g_inline co_sp<Display> GetDisplay() const {
        return display_weak_.lock();
    }

    g_nodiscard g_inline RenderDevice GetRenderDeviceType() const {
        return device_type_;
    }

    g_nodiscard g_inline int32_t GetWidth() const {
        return width_;
    }

    g_nodiscard g_inline int32_t GetHeight() const {
        return height_;
    }

    g_nodiscard g_inline SkColorType GetColorType() const {
        return color_format_;
    }

    virtual std::string GetBufferStateDescriptor();

    void Resize(int32_t width, int32_t height);

    SkSurface *BeginFrame();
    SkSurface *GetCurrentFrameSurface();
    void Submit(const SkRegion& damage);

protected:
    virtual SkSurface *OnBeginFrame() = 0;
    virtual void OnSubmitFrame(SkSurface *surface, const SkRegion& damage) = 0;
    virtual void OnResize(int32_t width, int32_t height) = 0;

private:
    co_weak<Display>    display_weak_;
    RenderDevice        device_type_;
    SkColorType         color_format_;
    int32_t             width_;
    int32_t             height_;
    SkSurface          *current_frame_;
};

COBALT_NAMESPACE_END
#endif //COCOA_COBALT_RENDERTARGET_H
