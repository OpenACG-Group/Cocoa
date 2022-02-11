#ifndef COCOA_RENDERHOSTCALLBACKINFO_H
#define COCOA_RENDERHOSTCALLBACKINFO_H

#include <functional>
#include <chrono>
#include <optional>

#include "Cobalt/Cobalt.h"
#include "Cobalt/RenderClient.h"
#include "Cobalt/RenderClientCallInfo.h"
COBALT_NAMESPACE_BEGIN

class RenderHostInvocation;
class RenderClientObject;

class RenderHostCallbackInfo
{
public:
    explicit RenderHostCallbackInfo(RenderHostInvocation *pInvocation);
    ~RenderHostCallbackInfo() = default;

    g_nodiscard co_sp<RenderClientObject> GetReceiver() const;

    g_nodiscard RenderClientCallInfo::OpCode GetOpcode() const;

    g_nodiscard g_inline bool HasReturnValue() const {
        return has_return_value_;
    }

    template<typename T>
    g_nodiscard T& GetReturnValue() {
        CHECK(has_return_value_);
        return std::any_cast<T&>(return_value_);
    }

    g_nodiscard RenderClientCallInfo::Status GetReturnStatus() const;

    g_nodiscard const std::exception& GetCaughtException() const;

    g_nodiscard std::chrono::steady_clock::time_point GetProfileMilestone(ITCProfileMilestone tag) const;

private:
    RenderHostInvocation    *invocation_;
    bool                     has_return_value_;
    std::any                 return_value_;
};
using RenderHostCallback = std::function<void(RenderHostCallbackInfo&)>;

COBALT_NAMESPACE_END
#endif //COCOA_RENDERHOSTCALLBACKINFO_H
