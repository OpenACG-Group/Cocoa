#ifndef COCOA_GLAMOR_RENDERHOST_H
#define COCOA_GLAMOR_RENDERHOST_H

#include <any>
#include <queue>
#include <mutex>
#include "uv.h"

#include "Core/EventSource.h"
#include "Core/Errors.h"
#include "Glamor/Glamor.h"
#include "Glamor/RenderHostCallbackInfo.h"
#include "Glamor/RenderClientCallInfo.h"
#include "Glamor/RenderClientTransfer.h"
#include "Glamor/RenderHostTaskRunner.h"

namespace Json {
class Value;
}

GLAMOR_NAMESPACE_BEGIN

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

    struct TransferProfileSample;

    using ApplicationInfo = GlobalScope::ApplicationInfo;

    explicit RenderHost(EventLoop *hostLoop, ApplicationInfo applicationInfo);
    ~RenderHost() override;

    g_inline void SetRenderClient(RenderClient *pClient) {
        render_client_ = pClient;
    }

    g_nodiscard g_inline RenderClient *GetRenderClient() {
        return render_client_;
    }

    g_nodiscard g_inline const ApplicationInfo& GetApplicationInfo() const {
        return application_info_;
    }

    g_nodiscard g_inline const Shared<RenderHostTaskRunner>& GetRenderHostTaskRunner() const {
        return host_task_runner_;
    }

    g_nodiscard Shared<RenderClientObject> GetRenderHostCreator();

    void CollectTransferProfileSample(RenderClientTransfer *transfer);

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
    g_private_api void Send(const Shared<RenderClientObject>& receiver, RenderClientCallInfo info,
                            const RenderHostCallback& pCallback);

    g_private_api void WakeupHost(const std::shared_ptr<RenderClientTransfer>& transfer);

    g_private_api void OnDispose();

private:
    void OnResponseFromClient();
    void FlushProfileSamplesAsync();

    void asyncDispatch() override;

    RenderClient                     *render_client_;
    std::queue<std::shared_ptr<RenderClientTransfer>>
                                      client_transfer_queue_;
    std::mutex                        client_transfer_queue_lock_;
    Shared<RenderHostCreator>         host_creator_;
    Shared<RenderHostTaskRunner>      host_task_runner_;
    ApplicationInfo                   application_info_;

    RenderClientTransfer::Timepoint           samples_time_base_;
    std::vector<Shared<TransferProfileSample>> transfer_profile_samples_;
    Shared<Json::Value>                        profile_json_root_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_RENDERHOST_H
