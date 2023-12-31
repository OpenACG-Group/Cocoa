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

#include "Core/Errors.h"
#include "Core/Journal.h"
#include "Gallium/Runtime.h"
#include "Gallium/Inspector.h"
#include "Gallium/InspectorThread.h"
#include "Gallium/InspectorChannel.h"
GALLIUM_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Gallium.Inspector)

namespace {

v8_inspector::StringView as_v8sv(const std::string_view& sv)
{
    return {reinterpret_cast<const uint8_t*>(sv.data()), sv.length()};
}

} // namespace anonymous

Inspector::Inspector(uv_loop_t *loop, v8::Isolate *isolate,
                     v8::Local<v8::Context> context, int32_t port)
    : event_loop_(loop)
    , isolate_(isolate)
    , is_nested_message_loop_(false)
    , should_quit_loop_(false)
{
    CHECK(event_loop_ && isolate_);
    context_.Reset(isolate_, context);

    io_thread_ = InspectorThread::Start(loop, port, this);

    channel_ = std::make_unique<InspectorChannel>(this);
    v8_inspector_ = v8_inspector::V8Inspector::create(isolate, this);
    v8_inspector_session_ = v8_inspector_->connect(
        kContextGroupId,
        channel_.get(),
        {},
        v8_inspector::V8Inspector::kFullyTrusted,
        v8_inspector::V8Inspector::SessionPauseState::kWaitingForDebugger
    );

    context->SetAlignedPointerInEmbedderData(1, this);
    v8_inspector::V8ContextInfo context_info(context, kContextGroupId, as_v8sv("mainthread"));
    v8_inspector_->contextCreated(context_info);

    QLOG(LOG_INFO, "Started V8 inspector, listening on ws://127.0.0.1:{}", port);
}

Inspector::~Inspector() = default;

void Inspector::ScheduleModuleEvalOnNextConnect(std::function<void()> eval)
{
    schedule_module_eval_ = std::move(eval);
}

void Inspector::runMessageLoopOnPause(int ctx_group_id)
{
    if (is_nested_message_loop_)
        return;

    should_quit_loop_ = false;
    is_nested_message_loop_ = true;
    while (!should_quit_loop_)
    {
        // During this call, the `OnDisconnect()` or `OnMessage()` method
        // may be called to handle the incoming messages.
        // If `OnMessage()` is called, we enters V8 and then the `quitMessageLoopOnPause()`
        // may be called from v8. These methods will set `should_quit_loop_` to
        // exit this loop properly.
        io_thread_->WaitOnce();
    }
    is_nested_message_loop_ = false;
}

void Inspector::quitMessageLoopOnPause()
{
    should_quit_loop_ = true;
}

void Inspector::runIfWaitingForDebugger(int ctx_group_id)
{
    if (!schedule_module_eval_)
        return;

    Runtime *runtime = Runtime::GetBareFromIsolate(isolate_);
    if (runtime->getOptions().inspector_startup_brk)
    {
        v8_inspector_session_->schedulePauseOnNextStatement(
                as_v8sv("startup"), as_v8sv("{}"));
    }
    schedule_module_eval_();
    schedule_module_eval_ = nullptr;
}

void Inspector::OnConnect()
{
}

void Inspector::OnMessage(InspectorThread::MessageBuffer::Ptr msg)
{
    v8::SealHandleScope seal_handle_scope(isolate_);
    v8_inspector_session_->dispatchProtocolMessage(
            v8_inspector::StringView(msg->payload, msg->payload_size));
}

void Inspector::OnDisconnect()
{
    should_quit_loop_ = true;
}

v8::Local<v8::Context> Inspector::ensureDefaultContextInGroup(int ctx_group_id)
{
    CHECK(ctx_group_id == kContextGroupId);
    return context_.Get(isolate_);
}

GALLIUM_NS_END
