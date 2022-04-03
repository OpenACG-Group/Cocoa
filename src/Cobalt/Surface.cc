#include "Core/Journal.h"
#include "Cobalt/Surface.h"
#include "Cobalt/RenderTarget.h"
#include "Cobalt/Display.h"
COBALT_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Cobalt.Surface)

COBALT_TRAMPOLINE_IMPL(Surface, Close)
{
    auto this_ = info.GetThis()->Cast<Surface>();
    this_->Close();
    info.SetReturnStatus(RenderClientCallInfo::Status::kOpSuccess);
}

COBALT_TRAMPOLINE_IMPL(Surface, Resize)
{
    COBALT_TRAMPOLINE_CHECK_ARGS_NUMBER(2);
    auto this_ = info.GetThis()->Cast<Surface>();
    bool result = this_->Resize(info.Get<int32_t>(0), info.Get<int32_t>(1));

    info.SetReturnStatus(result ? RenderClientCallInfo::Status::kOpSuccess
                                : RenderClientCallInfo::Status::kOpFailed);
}

COBALT_TRAMPOLINE_IMPL(Surface, SetTitle)
{
    COBALT_TRAMPOLINE_CHECK_ARGS_NUMBER(1);
    auto this_ = info.GetThis()->Cast<Surface>();
    this_->SetTitle(info.Get<std::string>(0));
    info.SetReturnStatus(RenderClientCallInfo::Status::kOpSuccess);
}

COBALT_TRAMPOLINE_IMPL(Surface, GetBuffersDescriptor)
{
    auto this_ = info.GetThis()->Cast<Surface>();
    info.SetReturnStatus(RenderClientCallInfo::Status::kOpSuccess);
    info.SetReturnValue(this_->GetBuffersDescriptor());
}

Surface::Surface(co_sp<RenderTarget> rt)
    : RenderClientObject(RealType::kSurface)
    , has_disposed_(false)
    , render_target_(std::move(rt))
    , display_(render_target_->GetDisplay())
{
    SetMethodTrampoline(CROP_SURFACE_CLOSE, Surface_Close_Trampoline);
    SetMethodTrampoline(CROP_SURFACE_RESIZE, Surface_Resize_Trampoline);
    SetMethodTrampoline(CROP_SURFACE_SET_TITLE, Surface_SetTitle_Trampoline);
    SetMethodTrampoline(CROP_SURFACE_GET_BUFFERS_DESCRIPTOR, Surface_GetBuffersDescriptor_Trampoline);
}

Surface::~Surface()
{
    CHECK(has_disposed_ && "Surface should be closed before destructing");
}

int32_t Surface::GetWidth() const
{
    return render_target_->GetWidth();
}

int32_t Surface::GetHeight() const
{
    return render_target_->GetHeight();
}

SkColorType Surface::GetColorType() const
{
    return render_target_->GetColorType();
}

void Surface::Close()
{
    if (!has_disposed_)
    {
        this->OnClose();
        has_disposed_ = true;
        render_target_.reset();
        Emit(CRSI_SURFACE_CLOSED, RenderClientEmitterInfo());
        GetDisplay()->RemoveSurfaceFromList(Self()->Cast<Surface>());
        QLOG(LOG_DEBUG, "Surface has been disposed");
    }
}

bool Surface::Resize(int32_t width, int32_t height)
{
    QLOG(LOG_DEBUG, "Attempting to resize surface to {}x{}", width, height);
    if (width <= 0 || height <= 0)
        return false;
    render_target_->Resize(width, height);

    RenderClientEmitterInfo info;
    info.EmplaceBack<int32_t>(width);
    info.EmplaceBack<int32_t>(height);
    Emit(CRSI_SURFACE_RESIZE, std::move(info));

    return true;
}

void Surface::SetTitle(const std::string_view& title)
{
    QLOG(LOG_DEBUG, "Attempting to set surface title %fg<gr>\"{}\"%reset", title);
    this->OnSetTitle(title);
}

std::string Surface::GetBuffersDescriptor()
{
    return render_target_->GetBufferStateDescriptor();
}

COBALT_NAMESPACE_END
