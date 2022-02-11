#ifndef COCOA_RENDERHOSTINVOCATION_H
#define COCOA_RENDERHOSTINVOCATION_H

#include <utility>
#include <chrono>

#include "Cobalt/Cobalt.h"
#include "Cobalt/RenderClientCallInfo.h"
#include "Cobalt/RenderHostCallbackInfo.h"
#include "Cobalt/RenderClientTransfer.h"
COBALT_NAMESPACE_BEGIN

class RenderClientObject;

class RenderHostInvocation : public RenderClientTransfer
{
public:
    using Timepoint = std::chrono::steady_clock::time_point;

    RenderHostInvocation(co_sp<RenderClientObject> receiver, RenderClientCallInfo info,
                         RenderHostCallback pHostCallback)
        : RenderClientTransfer(RenderClientTransfer::Type::kInvocationResponse)
        , receiver_(std::move(receiver))
        , client_call_info_(std::move(info))
        , host_callback_(std::move(pHostCallback)) {}

    RenderHostInvocation(const RenderHostInvocation&) = delete;
    ~RenderHostInvocation() override = default;

    RenderHostInvocation& operator=(const RenderHostInvocation&) = delete;

    g_nodiscard g_inline co_sp<RenderClientObject> GetReceiver() const {
        return receiver_;
    }

    g_nodiscard g_inline RenderClientCallInfo& GetClientCallInfo() {
        return client_call_info_;
    }

    g_nodiscard g_inline RenderHostCallback GetHostCallback() const {
        return host_callback_;
    }

    g_inline void MarkProfileMilestone(ITCProfileMilestone tag) {
        profile_milestones_[static_cast<uint8_t>(tag)] = std::chrono::steady_clock::now();
    }

    g_nodiscard g_inline Timepoint GetProfileMilestone(ITCProfileMilestone tag) const {
        return profile_milestones_[static_cast<uint8_t>(tag)];
    }

private:
    static constexpr size_t kMilestonesSize = static_cast<uint8_t>(ITCProfileMilestone::kLast) + 1;

    co_sp<RenderClientObject>       receiver_;
    RenderClientCallInfo            client_call_info_;
    RenderHostCallback              host_callback_;
    Timepoint                       profile_milestones_[kMilestonesSize];
};

COBALT_NAMESPACE_END
#endif //COCOA_RENDERHOSTINVOCATION_H
