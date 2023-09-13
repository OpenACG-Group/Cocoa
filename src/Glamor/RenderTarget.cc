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

#include "Core/TraceEvent.h"
#include "Core/Journal.h"
#include "Glamor/RenderTarget.h"
GLAMOR_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Glamor.RenderTarget)

RenderTarget::RenderTarget(const Shared<Display>& display, RenderDevice device,
                           int32_t width, int32_t height, SkColorType format)
    : display_weak_(display)
    , device_type_(device)
    , color_format_(format)
    , width_(width)
    , height_(height)
    , current_frame_(nullptr)
    , frame_notification_router_(nullptr)
{
}

SkSurface *RenderTarget::BeginFrame()
{
    TRACE_EVENT("rendering", "RenderTarget::BeginFrame");
    if (current_frame_)
    {
        QLOG(LOG_WARNING, "Could not begin a new frame: a pending frame has"
                          " not been presented yet");
        return nullptr;
    }
    current_frame_ = this->OnBeginFrame();
    return current_frame_;
}

SkSurface *RenderTarget::GetCurrentFrameSurface()
{
    return current_frame_;
}

void RenderTarget::Submit(const FrameSubmitInfo& submit_info)
{
    TRACE_EVENT("rendering", "RenderTarget::Submit");
    if (last_submit_info_)
    {
        QLOG(LOG_WARNING, "Frame cannot be submitted more than once"
                          " in a rendering cycle");
        return;
    }
    last_submit_info_ = submit_info;
    this->OnSubmitFrame(current_frame_, submit_info);
}

void RenderTarget::Present()
{
    TRACE_EVENT("rendering", "RenderTarget::Present");
    if (!last_submit_info_)
    {
        QLOG(LOG_WARNING, "Frame must be submitted before being presented");
        return;
    }
    this->OnPresentFrame(current_frame_, *last_submit_info_);
    last_submit_info_.reset();
    current_frame_ = nullptr;
}

std::string RenderTarget::GetBufferStateDescriptor()
{
    return "<unsupported>";
}

void RenderTarget::Resize(int32_t width, int32_t height)
{
    width_ = width;
    height_ = height;
    this->OnResize(width, height);
}

const Shared<HWComposeSwapchain>& RenderTarget::GetHWComposeSwapchain()
{
    return this->OnGetHWComposeSwapchain();
}


const Shared<HWComposeSwapchain>& RenderTarget::OnGetHWComposeSwapchain()
{
    static Shared<HWComposeSwapchain> sw = nullptr;
    return sw;
}

sk_sp<SkSurface> RenderTarget::CreateOffscreenBackendSurface(const SkImageInfo& info)
{
    return this->OnCreateOffscreenBackendSurface(info);
}

uint32_t RenderTarget::RequestNextFrame()
{
    TRACE_EVENT("rendering", "RenderTarget::RequestNextFrame");
    return this->OnRequestNextFrame();
}

void RenderTarget::Trace(GraphicsResourcesTrackable::Tracer *tracer) noexcept
{
    if (!display_weak_.expired())
    {
        tracer->TraceResource("Display",
                              TRACKABLE_TYPE_CLASS_OBJECT,
                              TRACKABLE_DEVICE_CPU,
                              TRACKABLE_OWNERSHIP_WEAK,
                              TraceIdFromPointer(display_weak_.lock().get()));
    }
}

GLAMOR_NAMESPACE_END
