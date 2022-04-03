#ifndef COCOA_COBALT_RENDERCLIENT_H
#define COCOA_COBALT_RENDERCLIENT_H

#include <thread>
#include <queue>
#include <mutex>

#include "uv.h"
#include "Cobalt/Cobalt.h"
COBALT_NAMESPACE_BEGIN

class RenderHost;
class RenderHostInvocation;
class RenderClientObject;
class HWComposeContext;

enum class ITCProfileMilestone : uint8_t
{
    kHostConstruction   = 0,
    kHostEnqueued       = 1,
    kClientReceived     = 2,
    kClientProcessed    = 3,
    kClientFeedback     = 4,

    kHostReceived       = 5,

    kClientEmitted      = 6,

    kLast = 7
};

class RenderClient
{
public:
    explicit RenderClient(RenderHost *renderHost);
    ~RenderClient();

    g_nodiscard g_inline RenderHost *GetRenderHost() {
        return render_host_;
    }

    g_nodiscard g_inline uv_loop_t *GetEventLoop() {
        return client_event_loop_;
    }

    g_nodiscard co_sp<HWComposeContext> GetHWComposeContext();

    void EnqueueHostInvocation(RenderHostInvocation *invocation);
    void Dispose();

private:
    void RenderThread();
    void OnInvocationFromHost();

    static void InvocationFromHostTrampoline(uv_async_t *handle);

    RenderHost              *render_host_;
    uv_loop_t               *client_event_loop_;
    uv_async_t               host_call_async_;
    std::thread              thread_;
    bool                     disposed_;
    bool                     thread_stopped_;
    std::queue<RenderHostInvocation*> host_invocation_queue_;
    std::mutex                        host_invocation_queue_lock_;

    bool                     hw_compose_context_creation_failed_;
    bool                     hw_compose_disabled_;
    co_sp<HWComposeContext>  hw_compose_context_;
};

COBALT_NAMESPACE_END
#endif //COCOA_COBALT_RENDERCLIENT_H
