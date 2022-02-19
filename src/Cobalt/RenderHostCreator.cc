#include "Cobalt/RenderHostCreator.h"
#include "Cobalt/Display.h"
COBALT_NAMESPACE_BEGIN

COBALT_TRAMPOLINE_IMPL(RenderHostCreator, CreateDisplay)
{
    auto this_ = info.GetThis()->As<RenderHostCreator>();
    if (info.Length() != 1)
    {
        info.SetReturnStatus(RenderClientCallInfo::Status::kArgsInvalid);
        return;
    }

    auto& ret = info.SetReturnValue(this_->CreateDisplay(info.Get<std::string>(0)));
    info.SetReturnStatus(ret ? RenderClientCallInfo::Status::kOpSuccess
                             : RenderClientCallInfo::Status::kOpFailed);
}

RenderHostCreator::RenderHostCreator()
    : RenderClientObject(RealType::kRenderHostCreator)
{
    SetMethodTrampoline(CROP_RENDERHOSTCREATOR_CREATE_DISPLAY,
                        RenderHostCreator_CreateDisplay_Trampoline);
}

co_sp<RenderClientObject> RenderHostCreator::CreateDisplay(const std::string& name)
{
    auto result = Display::Connect(GlobalScope::Ref().GetRenderClient()->GetEventLoop(), name);

    RenderClientEmitterInfo emitterInfo;
    emitterInfo.PushBack(result->Self());
    Emit(CRSI_RENDERHOSTCREATOR_CREATED, std::move(emitterInfo));

    return result;
}

COBALT_NAMESPACE_END
