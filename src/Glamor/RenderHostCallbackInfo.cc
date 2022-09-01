/**
 * This file is part of Cocoa.
 *
 * Cocoa is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * Cocoa is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cocoa. If not, see <https://www.gnu.org/licenses/>.
 */

#include "Glamor/RenderHostCallbackInfo.h"
#include "Glamor/RenderHostInvocation.h"
GLAMOR_NAMESPACE_BEGIN

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

Shared<RenderClientObject> RenderHostCallbackInfo::GetReceiver() const
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

std::any& RenderHostCallbackInfo::GetClosureValue()
{
    return invocation_->GetClientCallInfo().GetClosure();
}

GLAMOR_NAMESPACE_END
