#include "Cobalt/RenderHostSlotCallbackInfo.h"
#include "Cobalt/RenderClientSignalEmit.h"
COBALT_NAMESPACE_BEGIN

RenderHostSlotCallbackInfo::RenderHostSlotCallbackInfo(RenderClientSignalEmit *emit)
    : client_emit_(emit)
    , args_vector_ref_(emit->GetArgs())
{
}

co_sp<RenderClientObject> RenderHostSlotCallbackInfo::GetEmitter() const
{
    return client_emit_->GetEmitter();
}

COBALT_NAMESPACE_END
