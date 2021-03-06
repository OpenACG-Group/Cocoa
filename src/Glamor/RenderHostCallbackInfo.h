#ifndef COCOA_GLAMOR_RENDERHOSTCALLBACKINFO_H
#define COCOA_GLAMOR_RENDERHOSTCALLBACKINFO_H

#include <functional>
#include <chrono>
#include <optional>

#include "Glamor/Glamor.h"
#include "Glamor/RenderClient.h"
#include "Glamor/RenderClientCallInfo.h"
GLAMOR_NAMESPACE_BEGIN

class RenderHostInvocation;
class RenderClientObject;

class RenderHostCallbackInfo
{
public:
    explicit RenderHostCallbackInfo(RenderHostInvocation *pInvocation);
    ~RenderHostCallbackInfo() = default;

    g_nodiscard Shared<RenderClientObject> GetReceiver() const;

    g_nodiscard RenderClientCallInfo::OpCode GetOpcode() const;

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

    g_nodiscard RenderClientCallInfo::Status GetReturnStatus() const;

    g_nodiscard const std::exception& GetCaughtException() const;

    g_nodiscard std::chrono::steady_clock::time_point GetProfileMilestone(ITCProfileMilestone tag) const;

private:
    std::any& GetClosureValue();

    RenderHostInvocation    *invocation_;
    bool                     has_return_value_;
    std::any                 return_value_;
};
using RenderHostCallback = std::function<void(RenderHostCallbackInfo&)>;

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_RENDERHOSTCALLBACKINFO_H
