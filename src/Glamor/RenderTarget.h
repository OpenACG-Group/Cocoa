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

#ifndef COCOA_GLAMOR_RENDERTARGET_H
#define COCOA_GLAMOR_RENDERTARGET_H

#include <optional>

#include "include/core/SkColor.h"
#include "include/core/SkSurface.h"
#include "include/core/SkRegion.h"
#include "include/gpu/GrBackendSemaphore.h"

#include "Core/Errors.h"
#include "Glamor/Glamor.h"
#include "Glamor/FrameNotificationRouter.h"
#include "Glamor/GraphicsResourcesTrackable.h"
GLAMOR_NAMESPACE_BEGIN

class Display;
class HWComposeSwapchain;

struct FrameSubmitInfo
{
    SkRegion damage_region;
    std::vector<GrBackendSemaphore> hw_signal_semaphores;
};

/**
 * @p RenderTarget is the underlying class of @p Surface.
 * Different from @p Surface, @p RenderTarget only takes the responsibility
 * related to rendering, like buffer management, hardware-acceleration, etc.
 * Other necessary features like input handling are implemented by Surface.
 *
 * @p RenderTarget is an internal class which can not be used out of Glamor.
 */
class RenderTarget : public GraphicsResourcesTrackable
{
public:
    enum class RenderDevice
    {
        /* Vulkan backend with GPU acceleration */
        kHWComposer,

        /* Skia CPU rasterizer backend */
        kRaster
    };

    RenderTarget(const std::shared_ptr<Display>& display, RenderDevice device,
                 int32_t width, int32_t height, SkColorType format);
    ~RenderTarget() override = default;

    g_nodiscard g_inline std::shared_ptr<Display> GetDisplay() const {
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

    g_inline void SetFrameNotificationRouter(FrameNotificationRouter *router) {
        frame_notification_router_ = router;
    }

    g_nodiscard g_inline FrameNotificationRouter *GetFrameNotificationRouter() const {
        return frame_notification_router_;
    }

    virtual std::string GetBufferStateDescriptor();

    void Resize(int32_t width, int32_t height);

    SkSurface *BeginFrame();
    SkSurface *GetCurrentFrameSurface();
    void Submit(const FrameSubmitInfo& submit_info);
    void Present();
    uint32_t RequestNextFrame();

    const std::shared_ptr<HWComposeSwapchain>& GetHWComposeSwapchain();

    sk_sp<SkSurface> CreateOffscreenBackendSurface(const SkImageInfo& info);

    void Trace(GraphicsResourcesTrackable::Tracer *tracer) noexcept override;

protected:
    virtual SkSurface *OnBeginFrame() = 0;
    virtual void OnSubmitFrame(SkSurface *surface, const FrameSubmitInfo& submit_info) = 0;
    virtual void OnPresentFrame(SkSurface *surface, const FrameSubmitInfo& submit_info) = 0;
    virtual void OnResize(int32_t width, int32_t height) = 0;
    virtual const std::shared_ptr<HWComposeSwapchain>& OnGetHWComposeSwapchain();
    virtual sk_sp<SkSurface> OnCreateOffscreenBackendSurface(const SkImageInfo& info) = 0;
    virtual uint32_t OnRequestNextFrame() = 0;

private:
    std::weak_ptr<Display>          display_weak_;
    RenderDevice                    device_type_;
    SkColorType                     color_format_;
    int32_t                         width_;
    int32_t                         height_;
    SkSurface                      *current_frame_;
    std::optional<FrameSubmitInfo>  last_submit_info_;
    FrameNotificationRouter        *frame_notification_router_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_RENDERTARGET_H
