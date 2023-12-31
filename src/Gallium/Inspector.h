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

#ifndef COCOA_GALLIUM_INSPECTOR_H
#define COCOA_GALLIUM_INSPECTOR_H

#include <utility>
#include <string>

#include "include/v8.h"
#include "include/v8-inspector.h"
#include "uv.h"

#include "Core/ConcurrentTaskQueue.h"
#include "Gallium/Gallium.h"
#include "Gallium/InspectorThread.h"
#include "Gallium/InspectorChannel.h"
GALLIUM_NS_BEGIN

class Inspector : public InspectorThread::EventHandler,
                  public v8_inspector::V8InspectorClient
{
public:
    Inspector(uv_loop_t *loop, v8::Isolate *isolate,
              v8::Local<v8::Context> context, int32_t port);

    ~Inspector() override;

    void ScheduleModuleEvalOnNextConnect(std::function<void()> eval);

    g_nodiscard v8::Isolate *GetIsolate() const {
        return isolate_;
    }

    g_nodiscard InspectorThread *GetIOThread() const {
        return io_thread_.get();
    }

private:
    void runMessageLoopOnPause(int ctx_group_id) override;
    void quitMessageLoopOnPause() override;
    v8::Local<v8::Context> ensureDefaultContextInGroup(int ctx_group_id) override;
    void runIfWaitingForDebugger(int contextGroupId) override;

    void OnConnect() override;
    void OnDisconnect() override;
    void OnMessage(InspectorThread::MessageBuffer::Ptr message) override;

    constexpr static int kContextGroupId = 1;

    uv_loop_t                          *event_loop_;
    v8::Isolate                        *isolate_;
    v8::Global<v8::Context>             context_;
    std::unique_ptr<InspectorThread>    io_thread_;
    std::function<void()>               schedule_module_eval_;
    std::unique_ptr<InspectorChannel>   channel_;
    bool                                is_nested_message_loop_;
    bool                                should_quit_loop_;
    std::unique_ptr<v8_inspector::V8Inspector>          v8_inspector_;
    std::unique_ptr<v8_inspector::V8InspectorSession>   v8_inspector_session_;
};

GALLIUM_NS_END
#endif //COCOA_GALLIUM_INSPECTOR_H
