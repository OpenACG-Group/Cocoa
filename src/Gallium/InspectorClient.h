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

#ifndef COCOA_GALLIUM_INSPECTORCLIENT_H
#define COCOA_GALLIUM_INSPECTORCLIENT_H

#include "include/v8-inspector.h"

#include "Gallium/Gallium.h"
GALLIUM_NS_BEGIN

class InspectorChannel;
class Inspector;

class InspectorClient : public v8_inspector::V8InspectorClient
{
public:
    InspectorClient(v8::Isolate *isolate,
                    v8::Local<v8::Context> context,
                    Inspector *inspector);
    ~InspectorClient() override = default;

    g_nodiscard g_inline v8::Isolate *GetIsolate() {
        return isolate_;
    }

    g_nodiscard g_inline v8::Local<v8::Context> GetContext() {
        return context_.Get(isolate_);
    }

    g_nodiscard g_inline Inspector *GetInspector() {
        return inspector_;
    }

    void runMessageLoopOnPause(int contextGroupId) override;
    void quitMessageLoopOnPause() override;

    void DispatchMessage(const std::string& view);
    void NotifyFrontendMessageArrival();

    void SchedulePauseOnNextStatement(const std::string& reason);

    void DisconnectedFromFrontend();

private:
    v8::Local<v8::Context> ensureDefaultContextInGroup(int contextGroupId) override;

    constexpr static int kContextGroupId = 1;

    v8::Isolate *isolate_;
    v8::Global<v8::Context> context_;
    Inspector *inspector_;
    std::unique_ptr<v8_inspector::V8Inspector> v8_inspector_;
    std::unique_ptr<v8_inspector::V8InspectorSession> v8_inspector_session_;
    std::unique_ptr<InspectorChannel> channel_;
    bool is_nested_message_loop_;
    bool should_quit_loop_;
};

GALLIUM_NS_END
#endif //COCOA_GALLIUM_INSPECTORCLIENT_H
