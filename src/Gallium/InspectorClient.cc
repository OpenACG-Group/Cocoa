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

#include "include/v8-inspector.h"
#include "fmt/format.h"

#include "Core/Journal.h"
#include "Core/Errors.h"
#include "Gallium/Inspector.h"
#include "Gallium/InspectorClient.h"
#include "Gallium/InspectorChannel.h"
GALLIUM_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Gallium.InspectorClient)

v8_inspector::StringView as_string_view(const char *str)
{
    return {reinterpret_cast<const uint8_t*>(str),
            std::char_traits<char>::length(str)};
}

InspectorClient::InspectorClient(v8::Isolate *isolate,
                                 v8::Local<v8::Context> context,
                                 Inspector *inspector)
    : isolate_(isolate)
    , context_(isolate, context)
    , inspector_(inspector)
    , is_nested_message_loop_(false)
    , should_quit_loop_(false)
{
    channel_ = std::make_unique<InspectorChannel>(this);
    v8_inspector_ = v8_inspector::V8Inspector::create(isolate, this);
    v8_inspector_session_ = v8_inspector_->connect(kContextGroupId, channel_.get(), {},
                                                   v8_inspector::V8Inspector::kFullyTrusted);
    context->SetAlignedPointerInEmbedderData(1, this);

    v8_inspector_->contextCreated(v8_inspector::V8ContextInfo(context, kContextGroupId,
                                                              as_string_view("inspector")));
}

void InspectorClient::SchedulePauseOnNextStatement(const std::string& reason)
{
    v8_inspector::StringView reason_view = as_string_view(reason.c_str());
    v8_inspector_session_->schedulePauseOnNextStatement(reason_view, reason_view);
}

void InspectorClient::NotifyFrontendMessageArrival()
{
    std::string first_message = inspector_->WaitAndTakeFrontendMessage();
    DispatchMessage(first_message);
}

void InspectorClient::runMessageLoopOnPause(int contextGroupId)
{
    // if (is_nested_message_loop_)
    //    return;

    should_quit_loop_ = false;
    is_nested_message_loop_ = true;
    while (!should_quit_loop_)
    {
        std::string message = inspector_->WaitAndTakeFrontendMessage();
        if (should_quit_loop_)
            break;

        DispatchMessage(message);
    }
    is_nested_message_loop_ = false;
}

void InspectorClient::quitMessageLoopOnPause()
{
    should_quit_loop_ = true;
}

void InspectorClient::DisconnectedFromFrontend()
{
    should_quit_loop_ = true;
    v8_inspector_session_.reset();
    v8_inspector_.reset();
}

v8::Local<v8::Context> InspectorClient::ensureDefaultContextInGroup(int contextGroupId)
{
    CHECK(contextGroupId == kContextGroupId);
    return context_.Get(isolate_);
}

void InspectorClient::DispatchMessage(const std::string& view)
{
    v8::HandleScope handle_scope(isolate_);
    auto message = v8::String::NewFromUtf8(isolate_, view.c_str(),
                                           v8::NewStringType::kNormal).ToLocalChecked();
    int length = message->Length();
    std::unique_ptr<uint16_t[]> buffer(new uint16_t[length]);
    message->Write(isolate_, buffer.get(), 0, length);

    v8_inspector::StringView message_view(buffer.get(), length);

    v8::SealHandleScope seal_handle_scope(isolate_);
    v8_inspector_session_->dispatchProtocolMessage(message_view);
}

GALLIUM_NS_END
