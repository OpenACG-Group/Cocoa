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

#ifndef COCOA_GLAMOR_PRESENTREMOTECALLRETURN_H
#define COCOA_GLAMOR_PRESENTREMOTECALLRETURN_H

#include <functional>
#include <chrono>
#include <optional>

#include "Glamor/Glamor.h"
#include "Glamor/PresentRemoteCall.h"
GLAMOR_NAMESPACE_BEGIN

class PresentRemoteCallMessage;
class PresentRemoteHandle;

class PresentRemoteCallReturn
{
public:
    explicit PresentRemoteCallReturn(PresentRemoteCallMessage *pInvocation);
    ~PresentRemoteCallReturn() = default;

    g_nodiscard Shared<PresentRemoteHandle> GetReceiver() const;

    g_nodiscard PresentRemoteCall::OpCode GetOpcode() const;

    g_nodiscard g_inline bool HasReturnValue() const {
        return has_return_value_;
    }

    template<typename T>
    g_nodiscard T& GetReturnValue() {
        CHECK(has_return_value_);
        return std::any_cast<T&>(return_value_);
    }

    template<typename T>
    g_nodiscard g_inline T& GetClosure() {
        return std::any_cast<T&>(GetClosureValue());
    }

    g_nodiscard PresentRemoteCall::Status GetReturnStatus() const;

    g_nodiscard const std::string& GetCaughtException() const;

    g_nodiscard std::chrono::steady_clock::time_point
    GetProfileMilestone(PresentMessageMilestone tag) const;

private:
    std::any& GetClosureValue();

    PresentRemoteCallMessage *invocation_;
    bool                      has_return_value_;
    std::any                  return_value_;
};
using PresentRemoteCallResultCallback = std::function<void(PresentRemoteCallReturn&)>;

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_PRESENTREMOTECALLRETURN_H
