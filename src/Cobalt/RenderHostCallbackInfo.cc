#include "Cobalt/RenderHostCallbackInfo.h"
#include "Cobalt/RenderHostInvocation.h"
COBALT_NAMESPACE_BEGIN

RenderHostCallbackInfo::RenderHostCallbackInfo(RenderHostInvocation *pInvocation)
    : invocation_(pInvocation)
    , has_return_value_(false)
{
    if (invocation_->GetClientCallInfo().GetReturnStatus() ==
            RenderClientCallInfo::Status::kOpSuccess)
    {
        return_value_ = invocation_->GetClientCallInfo().MoveReturnValue();
        has_return_value_ = return_value_.has_value();
    }
}

co_sp<RenderClientObject> RenderHostCallbackInfo::GetReceiver() const
{
    return invocation_->GetReceiver();
}

RenderClientCallInfo::OpCode RenderHostCallbackInfo::GetOpcode() const
{
    return invocation_->GetClientCallInfo().GetOpCode();
}

RenderClientCallInfo::Status RenderHostCallbackInfo::GetReturnStatus() const
{
    return invocation_->GetClientCallInfo().GetReturnStatus();
}

std::chrono::steady_clock::time_point
RenderHostCallbackInfo::GetProfileMilestone(ITCProfileMilestone tag) const
{
    return invocation_->GetProfileMilestone(tag);
}

const std::exception& RenderHostCallbackInfo::GetCaughtException() const
{
    return invocation_->GetClientCallInfo().GetCaughtException();
}

COBALT_NAMESPACE_END
