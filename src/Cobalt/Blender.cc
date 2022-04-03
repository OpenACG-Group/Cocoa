#include "Cobalt/Blender.h"
#include "Cobalt/RenderTarget.h"
#include "Cobalt/Surface.h"
COBALT_NAMESPACE_BEGIN

co_sp<Blender> Blender::Make(const co_sp<Surface>& surface)
{
    return std::make_shared<Blender>(surface);
}

Blender::Blender(const co_sp<Surface>& surface)
    : RenderClientObject(RealType::kBlender)
    , output_surface_(surface)
{
}

Blender::~Blender() = default;

RenderTarget::RenderDevice Blender::GetRenderDeviceType() const
{
    return output_surface_->GetRenderTarget()->GetRenderDeviceType();
}

int32_t Blender::GetWidth() const
{
    return output_surface_->GetWidth();
}

int32_t Blender::GetHeight() const
{
    return output_surface_->GetHeight();
}

SkColorInfo Blender::GetOutputColorInfo() const
{
    return {output_surface_->GetColorType(), SkAlphaType::kPremul_SkAlphaType, nullptr};
}

COBALT_NAMESPACE_END
