/**
 * This file is part of Cocoa.
 *
 * Cocoa is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * Cocoa is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cocoa. If not, see <https://www.gnu.org/licenses/>.
 */

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

#define HOST_HEARTBEAT_TIMER_MS     5000
#define HOST_WAIT_SYNC1_TIMEOUT_MS  500
#define HOST_WAIT_SYNC2_TIMEOUT_MS  2000

class RenderHost : public AsyncSource,
                   public TimerSource
{
public:
    static constexpr size_t kCallbackPoolInitSize = 128;
    using RequestId = uint64_t;

    struct TransferProfileSample;

    using ApplicationInfo = GlobalScope::ApplicationInfo;

    explicit RenderHost(EventLoop *hostLoop, ApplicationInfo applicationInfo);
    ~RenderHost() override;

    void SetRenderClient(RenderClient *pClient);

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

    enum class WaitResult
    {
        kFulfilled,
        kTimeout
    };

    /**
     * Send a sync request to the render thread and block current thread
     * until the render thread responds our sync request.
     * After current thread is resumed and enters the event loop again,
     * we will receive an extra notification of the response of that sync request
     * in the main thread's event loop, and we just ignore it.
     *
     * This method is typically used to "sync" between the main thread and
     * the render thread. When it returns, that means the message queue of
     * render thread is empty and all the requests from the main thread has been
     * processed and responded. But note that it does not mean the render thread
     * is completely idle because it still can be busy processing the notifications
     * from the rendering backend, the timers of animation, and the messages from the
     * system desktop compositor.
     *
     * If any signals are emitted during the blocking time, they will be received in the
     * event loop later, after current thread has resumed.
     *
     * @param timeout_ms Wait up to `timeout_ms` milliseconds;
     *                   `WaitResult::kTimeout` will be returned if render thread does not
     *                   respond in that time.
     */
    WaitResult WaitForSyncBarrier(int64_t timeout_ms = -1);

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
    KeepInLoop timerDispatch() override;

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
