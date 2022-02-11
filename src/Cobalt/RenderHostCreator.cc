#include "Cobalt/RenderHostCreator.h"
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
    SetMethodTrampoline(COBALT_OP_RENDERHOSTCREATOR_CREATE_DISPLAY,
                        RenderHostCreator_CreateDisplay_Trampoline);
}

co_sp<Display> RenderHostCreator::CreateDisplay(const std::string& name)
{
    printf("create display, name = %s\n", name.c_str());

    RenderClientEmitterInfo emitterInfo;
    emitterInfo.EmplaceBack<co_sp<RenderClientObject>>(nullptr);
    Emit(COBALT_SI_RENDERHOSTCREATOR_CREATED, std::move(emitterInfo));

    return nullptr;
}

COBALT_NAMESPACE_END
