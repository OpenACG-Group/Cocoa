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
#include "Gallium/InspectorChannel.h"
#include "Gallium/Inspector.h"
GALLIUM_NS_BEGIN

InspectorChannel::InspectorChannel(Inspector *inspector)
    : inspector_(inspector)
{
    CHECK(inspector);
}

namespace {

std::string inspector_string_view_extract(v8::Isolate *isolate,
                                          const v8_inspector::StringView& view)
{
    int length = static_cast<int>(view.length());
    v8::Local<v8::String> string;
    if (view.is8Bit())
    {
        string = v8::String::NewFromOneByte(isolate,
            reinterpret_cast<const uint8_t*>(view.characters8()),
            v8::NewStringType::kNormal, length).ToLocalChecked();
    }
    else
    {
        string = v8::String::NewFromTwoByte(isolate,
            reinterpret_cast<const uint16_t*>(view.characters16()),
            v8::NewStringType::kNormal, length).ToLocalChecked();
    }

    return *v8::String::Utf8Value(isolate, string);
}

void send_frontend_message(Inspector *inspector,
                           std::unique_ptr<v8_inspector::StringBuffer> message)
{
    v8::Isolate *isolate = inspector->GetIsolate();
    v8::Isolate::AllowJavascriptExecutionScope allow_js_scope(isolate);
    v8::HandleScope scope(isolate);

    std::string string = inspector_string_view_extract(isolate, message->string());
    inspector->GetIOThread()->Send(string);
}

} // namespace anonymous

void InspectorChannel::sendResponse(int callId, std::unique_ptr<v8_inspector::StringBuffer> message)
{
    send_frontend_message(inspector_, std::move(message));
}

void InspectorChannel::sendNotification(std::unique_ptr<v8_inspector::StringBuffer> message)
{
    send_frontend_message(inspector_, std::move(message));
}

void InspectorChannel::flushProtocolNotifications()
{
}

GALLIUM_NS_END
