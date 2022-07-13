#include "Glamor/RenderHostSlotCallbackInfo.h"
#include "Glamor/RenderClientSignalEmit.h"
GLAMOR_NAMESPACE_BEGIN

RenderHostSlotCallbackInfo::RenderHostSlotCallbackInfo(RenderClientSignalEmit *emit)
    : client_emit_(emit)
    , args_vector_ref_(emit->GetArgs())
{
}

Shared<RenderClientObject> RenderHostSlotCallbackInfo::GetEmitter() const
{
    return client_emit_->GetEmitter();
}

GLAMOR_NAMESPACE_END
