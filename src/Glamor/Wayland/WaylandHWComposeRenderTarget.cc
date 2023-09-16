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

#define VK_USE_PLATFORM_WAYLAND_KHR

#include <vulkan/vulkan.h>

#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"

#include "Core/TraceEvent.h"
#include "Core/Journal.h"
#include "Glamor/Glamor.h"
#include "Glamor/HWComposeContext.h"
#include "Glamor/Wayland/WaylandHWComposeRenderTarget.h"
#include "Glamor/Wayland/WaylandDisplay.h"
#include "Glamor/Wayland/WaylandMonitor.h"
#include "Glamor/HWComposeSwapchain.h"
GLAMOR_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Glamor.Wayland.HWComposeRenderTarget)

namespace {

struct WaylandVkSurfaceFactory : public HWComposeSwapchain::VkSurfaceFactory
{
    explicit WaylandVkSurfaceFactory(std::shared_ptr<WaylandHWComposeRenderTarget> rt)
        : hw_compose_rt_(std::move(rt)) {}
    ~WaylandVkSurfaceFactory() = default;

    VkSurfaceKHR Create(const std::shared_ptr<HWComposeContext> &context) override
    {
        VkWaylandSurfaceCreateInfoKHR info{};
        info.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
        info.flags = 0;
        info.pNext = nullptr;
        info.display = hw_compose_rt_->GetDisplay()->Cast<WaylandDisplay>()->GetWaylandDisplay();
        info.surface = hw_compose_rt_->GetWaylandSurface();

        VkSurfaceKHR result;
        if (vkCreateWaylandSurfaceKHR(context->GetVkInstance(), &info, nullptr, &result) != VK_SUCCESS)
        {
            QLOG(LOG_ERROR, "Failed in creating Vulkan surface for Wayland");
            return VK_NULL_HANDLE;
        }

        return result;
    }

    std::shared_ptr<WaylandHWComposeRenderTarget> hw_compose_rt_;
};

} // namespace anonymous

std::shared_ptr<WaylandHWComposeRenderTarget>
WaylandHWComposeRenderTarget::Make(const std::shared_ptr<WaylandDisplay>& display,
                                   int32_t width, int32_t height)
{
    if (width <= 0 || height <= 0)
    {
        QLOG(LOG_DEBUG, "Failed in creating RenderTarget: invalid dimensions ({}, {})",
             width, height);
        return nullptr;
    }

    auto hwcompose_ctx = GlobalScope::Ref().GetHWComposeContext();
    if (!hwcompose_ctx)
        return nullptr;

    auto rt = std::make_shared<WaylandHWComposeRenderTarget>(
            hwcompose_ctx, display, width, height,
            SkColorType::kBGRA_8888_SkColorType);
    CHECK(rt);

    rt->wl_event_queue_ = wl_display_create_queue(display->GetWaylandDisplay());
    if (!rt->wl_event_queue_)
    {
        QLOG(LOG_ERROR, "Failed to create an event queue for render target");
        return nullptr;
    }

    wl_compositor *compositor = display->GetGlobalsRef()->wl_compositor_;
    rt->wl_surface_ = wl_compositor_create_surface(compositor);
    if (!rt->wl_surface_)
    {
        QLOG(LOG_ERROR, "Failed to create Wayland compositor surface");
        return nullptr;
    }
    wl_proxy_set_queue(reinterpret_cast<wl_proxy *>(rt->wl_surface_), rt->wl_event_queue_);
    wl_surface_set_user_data(rt->wl_surface_, rt.get());

    std::optional<MonitorSubpixel> subpixel;
    for (const auto& monitor : display->RequestMonitorList())
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

    WaylandVkSurfaceFactory factory(rt);
    rt->swapchain_ = HWComposeSwapchain::Make(
            hwcompose_ctx, factory, width, height, sk_subpixel);
    if (!rt->swapchain_)
    {
        QLOG(LOG_ERROR, "Failed to create a HWCompose swapchain");
        return nullptr;
    }

    return rt;
}

WaylandHWComposeRenderTarget::WaylandHWComposeRenderTarget(
        std::shared_ptr<HWComposeContext> hwContext,
        const std::shared_ptr<WaylandDisplay>& display,
        int32_t width, int32_t height,
        SkColorType format)
    : WaylandRenderTarget(display, RenderDevice::kHWComposer, width, height, format)
    , hw_compose_context_(std::move(hwContext))
{
}

WaylandHWComposeRenderTarget::~WaylandHWComposeRenderTarget()
{
    if (swapchain_)
    {
        CHECK(swapchain_.unique());
        swapchain_.reset();
    }
    hw_compose_context_.reset();

    if (wl_surface_)
        wl_surface_destroy(wl_surface_);
    if (wl_event_queue_)
        wl_event_queue_destroy(wl_event_queue_);
}

void WaylandHWComposeRenderTarget::OnClearFrameBuffers()
{
    WaylandRoundtripScope scope(GetDisplay()->Cast<WaylandDisplay>());
    SkSurface *surface = swapchain_->NextFrame();
    surface->getCanvas()->clear(SK_ColorBLACK);
    swapchain_->SubmitFrame({});
    swapchain_->PresentFrame();
}

SkSurface *WaylandHWComposeRenderTarget::OnBeginFrame()
{
    TRACE_EVENT("rendering", "WaylandHWComposeRenderTarget::OnBeginFrame");
    WaylandRoundtripScope scope(GetDisplay()->Cast<WaylandDisplay>());
    return swapchain_->NextFrame();
}

void WaylandHWComposeRenderTarget::OnSubmitFrame(SkSurface *surface,
                                                 const FrameSubmitInfo& submit_info)
{
    TRACE_EVENT("rendering", "WaylandHWComposeRenderTarget::OnSubmitFrame");
    swapchain_->SubmitFrame(submit_info.hw_signal_semaphores);
}

void WaylandHWComposeRenderTarget::OnPresentFrame(SkSurface *surface,
                                                  const FrameSubmitInfo &submit_info)
{
    TRACE_EVENT("rendering", "WaylandHWComposeRenderTarget::OnPresentFrame");
    WaylandRoundtripScope scope(GetDisplay()->Cast<WaylandDisplay>());
    swapchain_->PresentFrame();
}

void WaylandHWComposeRenderTarget::OnResize(int32_t width, int32_t height)
{
    WaylandRoundtripScope scope(GetDisplay()->Cast<WaylandDisplay>());
    swapchain_->Resize(width, height);
    OnClearFrameBuffers();
}

const std::shared_ptr<HWComposeSwapchain>&
WaylandHWComposeRenderTarget::OnGetHWComposeSwapchain()
{
    return swapchain_;
}

sk_sp<SkSurface>
WaylandHWComposeRenderTarget::OnCreateOffscreenBackendSurface(const SkImageInfo& info)
{
    sk_sp<SkSurface> rt = SkSurfaces::RenderTarget(swapchain_->GetSkiaGpuContext(),
                                                   skgpu::Budgeted::kYes, info);
    return rt;
}

std::string WaylandHWComposeRenderTarget::GetBufferStateDescriptor()
{
    return swapchain_->GetBufferStateDescriptor();
}

void WaylandHWComposeRenderTarget::Trace(GraphicsResourcesTrackable::Tracer *tracer) noexcept
{
    WaylandRenderTarget::Trace(tracer);

    // `HWComposeSwapchain` is only owned by the current render target,
    // and each render target does not share the same swapchain with others.
    tracer->TraceMember("HWComposeSwapchain", swapchain_.get());
}

GLAMOR_NAMESPACE_END
