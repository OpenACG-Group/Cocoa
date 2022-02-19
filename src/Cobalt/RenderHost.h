#ifndef COCOA_RENDERHOST_H
#define COCOA_RENDERHOST_H

#include <any>
#include <queue>
#include <mutex>
#include "uv.h"

#include "Core/EventSource.h"
#include "Core/Errors.h"
#include "Cobalt/Cobalt.h"
#include "Cobalt/RenderHostCallbackInfo.h"
#include "Cobalt/RenderClientCallInfo.h"
#include "Cobalt/RenderClientTransfer.h"
COBALT_NAMESPACE_BEGIN

class RenderClientObject;
class RenderClient;
class RenderHost;
class RenderHostInvocation;
class RenderHostCreator;

class RenderHost : public AsyncSource
{
public:
    static constexpr size_t kCallbackPoolInitSize = 128;
    using RequestId = uint64_t;

    explicit RenderHost(EventLoop *hostLoop);
    ~RenderHost() override;

    g_inline void SetRenderClient(RenderClient *pClient) {
        render_client_ = pClient;
    }

    g_nodiscard g_inline RenderClient *GetRenderClient() {
        return render_client_;
    }

    g_nodiscard co_sp<RenderClientObject> GetRenderHostCreator();

    /**
     * Host thread calls this to send a request (invocation/call) to render thread.
     * At the point where the request has been processed, `pCallback` will be called
     * if not nullptr.
     *
     * @param receiver      Which RenderClient object will receive the request.
     * @param info          OpCode and arguments.
     * @param pCallback     (nullable) A callback function to notify when the request
     *                      has been processed.
     */
    g_private_api void Send(const co_sp<RenderClientObject>& receiver, RenderClientCallInfo info,
                            const RenderHostCallback& pCallback);

    g_private_api void WakeupHost(RenderClientTransfer *transfer);

    g_private_api void OnDispose();

private:
    void OnResponseFromClient();

    void asyncDispatch() override;

    RenderClient                     *render_client_;
    std::queue<RenderClientTransfer*> client_transfer_queue_;
    std::mutex                        client_transfer_queue_lock_;
    co_sp<RenderHostCreator>          host_creator_;
};

COBALT_NAMESPACE_END
#endif //COCOA_RENDERHOST_H
