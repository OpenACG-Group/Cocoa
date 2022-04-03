#include "Cobalt/RenderTarget.h"
COBALT_NAMESPACE_BEGIN

RenderTarget::RenderTarget(const co_sp<Display>& display, RenderDevice device,
                           int32_t width, int32_t height, SkColorType format)
    : display_weak_(display)
    , device_type_(device)
    , color_format_(format)
    , width_(width)
    , height_(height)
    , current_frame_(nullptr)
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

COBALT_NAMESPACE_END
