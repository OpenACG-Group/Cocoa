#ifndef COCOA_GLAMOR_RENDERCLIENT_H
#define COCOA_GLAMOR_RENDERCLIENT_H

#include <thread>
#include <queue>
#include <mutex>

#include "uv.h"
#include "Glamor/Glamor.h"
GLAMOR_NAMESPACE_BEGIN

class RenderHost;
class RenderHostInvocation;
class RenderClientSignalEmit;
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
    struct DLTSIClosure
    {
        Shared<RenderClientSignalEmit> emit;
        Shared<RenderClientObject> emitter;
    };

    explicit RenderClient(RenderHost *renderHost);
    ~RenderClient();

    g_nodiscard g_inline RenderHost *GetRenderHost() {
        return render_host_;
    }

    g_nodiscard g_inline uv_loop_t *GetEventLoop() {
        return client_event_loop_;
    }

    g_nodiscard Shared<HWComposeContext> GetHWComposeContext();

    void EnqueueHostInvocation(const Shared<RenderHostInvocation>& invocation);
    void ScheduleDeferredLocalThreadSlotsInvocation(const Shared<RenderClientSignalEmit>& emit,
                                                    const Shared<RenderClientObject>& emitter);
    void Dispose();

private:
    void RenderThread();
    void OnInvocationFromHost();

    static void InvocationFromHostTrampoline(uv_async_t *handle);
    static void DLTSICallback(uv_idle_t *handle);

    RenderHost              *render_host_;
    uv_loop_t               *client_event_loop_;
    uv_async_t               host_call_async_;

    // DLTSI: Deferred Local Thread Slots Invocation
    uv_idle_t                dltsi_idle_handle_;
    std::queue<DLTSIClosure> dltsi_closures_queue_;

    std::thread              thread_;
    bool                     disposed_;
    bool                     thread_stopped_;
    std::queue<Shared<RenderHostInvocation>> host_invocation_queue_;
    std::mutex                        host_invocation_queue_lock_;

    bool                     hw_compose_context_creation_failed_;
    bool                     hw_compose_disabled_;
    Shared<HWComposeContext>  hw_compose_context_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_RENDERCLIENT_H
