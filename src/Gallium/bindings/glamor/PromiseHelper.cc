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

#include <utility>

#include "fmt/format.h"

#include "Core/Exception.h"
#include "Core/TraceEvent.h"
#include "Gallium/bindings/glamor/PromiseHelper.h"
#include "Gallium/RuntimeBase.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

struct EventDefinitionData
{
    EventEmitterBase                        *this_ = nullptr;
    std::shared_ptr<gl::RenderClientObject>  handle;
    std::string                              name;
    uint32_t                                 signum = 0;
    InfoAcceptor                             args_converter;
};

void DefineSignalEventsOnEventEmitter(EventEmitterBase *this_,
                                      const std::shared_ptr<gl::RenderClientObject>& handle,
                                      const std::vector<SignalEventInfo>& info_vec)
{
    for (const SignalEventInfo& event_info : info_vec)
    {
        auto sp_event_data = std::make_shared<EventDefinitionData>();
        sp_event_data->this_ = this_;
        sp_event_data->handle = handle;
        sp_event_data->name = event_info.name;
        sp_event_data->signum = event_info.signum;
        sp_event_data->args_converter = event_info.args_converter;

        this_->EmitterDefineEvent(event_info.name, [sp_event_data] {
            auto emit = sp_event_data->this_->EmitterWrapAsCallable(sp_event_data->name);
            return sp_event_data->handle->Connect(
                sp_event_data->signum,
                [emit, sp_event_data](gl::RenderHostSlotCallbackInfo& info) {
                    v8::Isolate *isolate = v8::Isolate::GetCurrent();
                    v8::HandleScope handle_scope(isolate);
                    if (sp_event_data->args_converter)
                        emit(sp_event_data->args_converter(isolate, info));
                    else
                        emit({});
                }
            );
        }, [sp_event_data](uint64_t id) {
            sp_event_data->handle->Disconnect(id);
        });
    }
}

void PromisifiedRemoteCall::ResultCallback(gl::RenderHostCallbackInfo& info)
{
    auto closure = info.GetClosure<std::shared_ptr<PromisifiedRemoteCall>>();

    v8::Isolate *isolate = closure->isolate;
    v8::HandleScope handle_scope(isolate);

    v8::Local<v8::Promise::Resolver> resolver = closure->resolver.Get(isolate);

    std::shared_ptr<gl::RenderClientObject> receiver = info.GetReceiver();
    std::string error = fmt::format("[RemoteCall(receiver={}@{} opcode={})] ",
                                    gl::RenderClientObject::GetTypeName(receiver->GetRealType()),
                                    fmt::ptr(receiver.get()),
                                    info.GetOpcode());

    switch (info.GetReturnStatus())
    {
    case gl::RenderClientCallInfo::Status::kOpCodeInvalid:
        error += "Invalid opcode";
        break;
    case gl::RenderClientCallInfo::Status::kArgsInvalid:
        error += "Invalid number or type of arguments";
        break;
    case gl::RenderClientCallInfo::Status::kCaught:
        error += fmt::format("Caught error: {}", info.GetCaughtException());
        break;
    case gl::RenderClientCallInfo::Status::kOpFailed:
        error += "Operation failed";
        break;
    case gl::RenderClientCallInfo::Status::kOpSuccess:
        break;
    default:
        MARK_UNREACHABLE();
    }

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    if (info.GetReturnStatus() != gl::RenderClientCallInfo::Status::kOpSuccess)
    {
        CHECK(resolver->Reject(context, binder::to_v8(isolate, error)).ToChecked());
        return;
    }

    v8::Local<v8::Value> result;
    if (closure->result_converter)
    {
        result = closure->result_converter(isolate, info);
        if (result.IsEmpty())
        {
            error += "Failed to convert result to a JS value";
            CHECK(resolver->Reject(
                    context, binder::to_v8(isolate, error)).ToChecked());
            return;
        }
    }
    else
        result = v8::Undefined(isolate);

    CHECK(resolver->Resolve(context, result).ToChecked());
}

GALLIUM_BINDINGS_GLAMOR_NS_END
