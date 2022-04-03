#ifndef COCOA_COBALT_RENDERHOSTINVOCATION_H
#define COCOA_COBALT_RENDERHOSTINVOCATION_H

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

private:
    co_sp<RenderClientObject>       receiver_;
    RenderClientCallInfo            client_call_info_;
    RenderHostCallback              host_callback_;
};

COBALT_NAMESPACE_END
#endif //COCOA_COBALT_RENDERHOSTINVOCATION_H
