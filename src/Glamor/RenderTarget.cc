#include "Glamor/RenderTarget.h"
GLAMOR_NAMESPACE_BEGIN

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
    current_frame_ = this->OnBeginFrame();
    return current_frame_;
}

SkSurface *RenderTarget::GetCurrentFrameSurface()
{
    return current_frame_;
}

void RenderTarget::Submit(const SkRegion& damage)
{
    this->OnSubmitFrame(current_frame_, damage);
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
    return this->OnRequestNextFrame();
}

GLAMOR_NAMESPACE_END
