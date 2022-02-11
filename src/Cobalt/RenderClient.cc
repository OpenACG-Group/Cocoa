#include "Core/Errors.h"
#include "Core/Journal.h"
#include "Cobalt/RenderClient.h"
#include "Cobalt/RenderHostInvocation.h"
#include "Cobalt/RenderClientCallInfo.h"
#include "Cobalt/RenderClientObject.h"
#include "Cobalt/RenderHost.h"
COBALT_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Cobalt.RenderClient)

RenderClient::RenderClient(RenderHost *renderHost)
    : render_host_(renderHost)
    , client_event_loop_(nullptr)
    , host_call_async_{}
    , disposed_(false)
    , thread_stopped_(false)
{
    client_event_loop_ = new uv_loop_t;
    CHECK(client_event_loop_);
    uv_loop_init(client_event_loop_);

    uv_async_init(client_event_loop_, &host_call_async_, InvocationFromHostTrampoline);
    uv_handle_set_data((uv_handle_t*)&host_call_async_, this);

    thread_ = std::thread(&RenderClient::RenderThread, this);
}

RenderClient::~RenderClient()
{
    Dispose();
}

void RenderClient::Dispose()
{
    if (!disposed_)
    {
        disposed_ = true;
        while (!thread_stopped_)
            uv_async_send(&host_call_async_);

        thread_.join();

        uv_loop_close(client_event_loop_);
        delete client_event_loop_;
    }
}

void RenderClient::InvocationFromHostTrampoline(uv_async_t *handle)
{
    auto *pClient = reinterpret_cast<RenderClient*>(uv_handle_get_data((uv_handle_t*) handle));
    CHECK(pClient);
    pClient->OnInvocationFromHost();
}

void RenderClient::RenderThread()
{
    pthread_setname_np(pthread_self(), "RenderThread");
    QLOG(LOG_INFO, "Render thread has started, RenderClient:{}", fmt::ptr(this));

    uv_run(client_event_loop_, UV_RUN_DEFAULT);
    thread_stopped_ = true;

    QLOG(LOG_INFO, "Render thread has stopped, RenderClient:{}", fmt::ptr(this));
}

void RenderClient::OnInvocationFromHost()
{
    if (disposed_)
    {
        uv_close((uv_handle_t*) &host_call_async_, nullptr);
        return;
    }

    RenderHostInvocation *pInvocation;
    host_invocation_queue_lock_.lock();
    while (!host_invocation_queue_.empty())
    {
        pInvocation = host_invocation_queue_.front();
        host_invocation_queue_.pop();
        host_invocation_queue_lock_.unlock();

        pInvocation->MarkProfileMilestone(ITCProfileMilestone::kClientReceived);

        co_sp<RenderClientObject> pReceiver = pInvocation->GetReceiver();
        pReceiver->CallFromHostTrampoline(pInvocation->GetClientCallInfo());

        pInvocation->MarkProfileMilestone(ITCProfileMilestone::kClientProcessed);

        render_host_->WakeupHost(pInvocation);
        pInvocation->MarkProfileMilestone(ITCProfileMilestone::kClientFeedback);

        host_invocation_queue_lock_.lock();
    }
    host_invocation_queue_lock_.unlock();
}

void RenderClient::EnqueueHostInvocation(RenderHostInvocation *invocation)
{
    {
        std::scoped_lock scope(host_invocation_queue_lock_);
        host_invocation_queue_.push(invocation);
    }
    uv_async_send(&host_call_async_);
}

COBALT_NAMESPACE_END
