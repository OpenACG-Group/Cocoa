#ifndef COCOA_GLAMOR_RENDERHOSTINVOCATION_H
#define COCOA_GLAMOR_RENDERHOSTINVOCATION_H

#include <utility>
#include <chrono>

#include "Glamor/Glamor.h"
#include "Glamor/RenderClientCallInfo.h"
#include "Glamor/RenderHostCallbackInfo.h"
#include "Glamor/RenderClientTransfer.h"
GLAMOR_NAMESPACE_BEGIN

class RenderClientObject;

class RenderHostInvocation : public RenderClientTransfer
{
public:
    RenderHostInvocation(Shared<RenderClientObject> receiver, RenderClientCallInfo info,
                         RenderHostCallback pHostCallback)
        : RenderClientTransfer(RenderClientTransfer::Type::kInvocationResponse)
        , receiver_(std::move(receiver))
        , client_call_info_(std::move(info))
        , host_callback_(std::move(pHostCallback)) {}

    RenderHostInvocation(const RenderHostInvocation&) = delete;
    ~RenderHostInvocation() override = default;

    RenderHostInvocation& operator=(const RenderHostInvocation&) = delete;

    g_nodiscard g_inline Shared<RenderClientObject> GetReceiver() const {
        return receiver_;
    }

    g_nodiscard g_inline RenderClientCallInfo& GetClientCallInfo() {
        return client_call_info_;
    }

    g_nodiscard g_inline RenderHostCallback GetHostCallback() const {
        return host_callback_;
    }

private:
    Shared<RenderClientObject>       receiver_;
    RenderClientCallInfo            client_call_info_;
    RenderHostCallback              host_callback_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_RENDERHOSTINVOCATION_H
