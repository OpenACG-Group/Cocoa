#include "Glamor/RenderHostCreator.h"
#include "Glamor/Display.h"
#include "Glamor/Blender.h"
#include "Glamor/Surface.h"
GLAMOR_NAMESPACE_BEGIN

GLAMOR_TRAMPOLINE_IMPL(RenderHostCreator, CreateDisplay)
{
    GLAMOR_TRAMPOLINE_CHECK_ARGS_NUMBER(1);
    auto this_ = info.GetThis()->As<RenderHostCreator>();

    auto& ret = info.SetReturnValue(this_->CreateDisplay(info.Get<std::string>(0)));
    info.SetReturnStatus(ret ? RenderClientCallInfo::Status::kOpSuccess
                             : RenderClientCallInfo::Status::kOpFailed);
}

GLAMOR_TRAMPOLINE_IMPL(RenderHostCreator, CreateBlender)
{
    GLAMOR_TRAMPOLINE_CHECK_ARGS_NUMBER(1);
    auto this_ = info.GetThis()->As<RenderHostCreator>();

    auto& ret = info.SetReturnValue(this_->CreateBlender(info.Get<Shared<RenderClientObject>>(0)));
    info.SetReturnStatus(ret ? RenderClientCallInfo::Status::kOpSuccess
                             : RenderClientCallInfo::Status::kOpFailed);
}

RenderHostCreator::RenderHostCreator()
    : RenderClientObject(RealType::kRenderHostCreator)
{
    SetMethodTrampoline(CROP_RENDERHOSTCREATOR_CREATE_DISPLAY,
                        RenderHostCreator_CreateDisplay_Trampoline);
    SetMethodTrampoline(CROP_RENDERHOSTCERATOT_CREATE_BLENDER,
                        RenderHostCreator_CreateBlender_Trampoline);
}

Shared<RenderClientObject> RenderHostCreator::CreateDisplay(const std::string& name)
{
    auto result = Display::Connect(GlobalScope::Ref().GetRenderClient()->GetEventLoop(), name);

    RenderClientEmitterInfo emitterInfo;
    emitterInfo.PushBack(result->Self());
    Emit(CRSI_RENDERHOSTCREATOR_CREATED, std::move(emitterInfo));

    return result;
}

Shared<RenderClientObject> RenderHostCreator::CreateBlender(const Shared<RenderClientObject>& surface)
{
    return Blender::Make(surface->As<Surface>());
}

GLAMOR_NAMESPACE_END
