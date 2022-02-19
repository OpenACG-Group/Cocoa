#include "Cobalt/RenderHost.h"
#include "Cobalt/RenderHostInvocation.h"
#include "Cobalt/RenderClient.h"
#include "Cobalt/RenderHostCreator.h"
#include "Cobalt/RenderClientEmitterInfo.h"
COBALT_NAMESPACE_BEGIN

RenderHost::RenderHost(EventLoop *hostLoop)
    : AsyncSource(hostLoop)
    , render_client_(nullptr)
    , host_creator_(std::make_shared<RenderHostCreator>())
{
}

RenderHost::~RenderHost()
{
    CHECK(host_creator_.unique());
}

void RenderHost::OnDispose()
{
    AsyncSource::disableAsync();
    render_client_ = nullptr;
}

co_sp<RenderClientObject> RenderHost::GetRenderHostCreator()
{
    return host_creator_;
}

void RenderHost::asyncDispatch()
{
    OnResponseFromClient();
}

void RenderHost::OnResponseFromClient()
{
    RenderClientTransfer *transfer;

    client_transfer_queue_lock_.lock();
    while (!client_transfer_queue_.empty())
    {
        transfer = client_transfer_queue_.front();
        client_transfer_queue_.pop();
        client_transfer_queue_lock_.unlock();

        if (transfer->IsInvocationResponse())
        {
            auto *invocation = dynamic_cast<RenderHostInvocation*>(transfer);
            invocation->MarkProfileMilestone(ITCProfileMilestone::kHostReceived);
            RenderHostCallbackInfo callbackInfo(invocation);
            invocation->GetHostCallback()(callbackInfo);
        }
        else if (transfer->IsSignalEmit())
        {
            auto *emit = dynamic_cast<RenderClientSignalEmit*>(transfer);
            co_sp<RenderClientObject> emitter = emit->GetEmitter();
            emitter->EmitterTrampoline(emit);
        }

        delete transfer;

        client_transfer_queue_lock_.lock();
    }
    client_transfer_queue_lock_.unlock();
}

void RenderHost::Send(const co_sp<RenderClientObject>& receiver, RenderClientCallInfo info,
                      const RenderHostCallback& pCallback)
{
    CHECK(receiver);

    auto *invocation = new RenderHostInvocation(receiver, std::move(info), pCallback);
    invocation->MarkProfileMilestone(ITCProfileMilestone::kHostConstruction);

    render_client_->EnqueueHostInvocation(invocation);
    invocation->MarkProfileMilestone(ITCProfileMilestone::kHostEnqueued);
}

void RenderHost::WakeupHost(RenderClientTransfer *transfer)
{
    {
        std::scoped_lock scope(client_transfer_queue_lock_);
        client_transfer_queue_.push(transfer);
    }
    AsyncSource::wakeupAsync();
}

COBALT_NAMESPACE_END
