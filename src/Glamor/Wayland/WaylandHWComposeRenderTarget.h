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

#ifndef COCOA_GLAMOR_WAYLANDHWCOMPOSERENDERTARGET_H
#define COCOA_GLAMOR_WAYLANDHWCOMPOSERENDERTARGET_H

#include <wayland-client.h>

#include "Glamor/Glamor.h"
#include "Glamor/Wayland/WaylandRenderTarget.h"
GLAMOR_NAMESPACE_BEGIN

class WaylandDisplay;
class HWComposeSwapchain;

class WaylandHWComposeRenderTarget : public WaylandRenderTarget
{
public:
    static std::shared_ptr<WaylandHWComposeRenderTarget>
    Make(const std::shared_ptr<WaylandDisplay>& display, int32_t width, int32_t height);

    WaylandHWComposeRenderTarget(std::shared_ptr<HWComposeContext> hwContext,
                                 const std::shared_ptr<WaylandDisplay>& display,
                                 int32_t width, int32_t height, SkColorType format);
    ~WaylandHWComposeRenderTarget() override;

    void OnClearFrameBuffers() override;
    SkSurface *OnBeginFrame() override;
    void OnSubmitFrame(SkSurface *surface, const FrameSubmitInfo& submit_info) override;
    void OnPresentFrame(SkSurface *surface, const FrameSubmitInfo& submit_info) override;
    void OnResize(int32_t width, int32_t height) override;
    const std::shared_ptr<HWComposeSwapchain>& OnGetHWComposeSwapchain() override;
    sk_sp<SkSurface> OnCreateOffscreenBackendSurface(const SkImageInfo& info) override;
    std::string GetBufferStateDescriptor() override;

    void Trace(GraphicsResourcesTrackable::Tracer *tracer) noexcept override;

private:
    std::shared_ptr<HWComposeContext>   hw_compose_context_;
    std::shared_ptr<HWComposeSwapchain> swapchain_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_WAYLANDHWCOMPOSERENDERTARGET_H
