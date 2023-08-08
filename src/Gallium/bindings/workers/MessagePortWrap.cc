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

#include "Gallium/RuntimeBase.h"
#include "Gallium/bindings/workers/MessagePort.h"
#include "Gallium/bindings/workers/Exports.h"
GALLIUM_BINDINGS_WORKERS_NS_BEGIN

v8::Local<v8::Value> MessagePortWrap::MakeConnectedPair()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    RuntimeBase *runtime = RuntimeBase::FromIsolate(isolate);
    CHECK(runtime);
    auto pair = MessagePort::MakeConnectedPair(runtime->GetEventLoop());
    return binder::to_v8(isolate, std::vector<v8::Local<v8::Value>>{
        binder::NewObject<MessagePortWrap>(isolate, std::move(pair.first)),
        binder::NewObject<MessagePortWrap>(isolate, std::move(pair.second))
    });
}

namespace {

class MessagePortWrapFlattenedData : public MessagePortWrap::FlattenedData
{
public:
    static MessagePortWrap::MaybeFlattened Transfer(v8::Isolate *isolate,
                                                    ExportableObjectBase *base,
                                                    bool pretest)
    {
        auto *wrap = base->Cast<MessagePortWrap>();
        if (pretest)
            // NOLINTNEXTLINE
            return MessagePortWrap::FlattenPretestResult(!!wrap->GetPort());

        // Close the port (detach it from current event loop) first.
        // It should not attach to any event loop until it is delivered
        // to the destination port.
        wrap->close();

        std::shared_ptr<MessagePort> port = wrap->GetPort();
        return MessagePortWrap::JustFlattened(
                std::make_shared<MessagePortWrapFlattenedData>(port));
    }

    explicit MessagePortWrapFlattenedData(std::shared_ptr<MessagePort> port)
        : port_(std::move(port)) {}
    ~MessagePortWrapFlattenedData() override = default;

    v8::MaybeLocal<v8::Object> Deserialize(v8::Isolate *isolate,
                                           v8::Local<v8::Context> context) override
    {
        // Port has been delivered to the destination port, and
        // attach it to the new event loop.
        port_->AttachToEventLoop(
                RuntimeBase::FromIsolate(isolate)->GetEventLoop());

        return binder::NewObject<MessagePortWrap>(isolate, port_);
    }

private:
    std::shared_ptr<MessagePort> port_;
};

} // namespace anonymous

MessagePortWrap::MessagePortWrap(std::shared_ptr<MessagePort> port)
    : ExportableObjectBase(kMessagePort_Attr | kTransferable_Attr,
                           {}, MessagePortWrapFlattenedData::Transfer)
    , port_(std::move(port))
{
    EmitterDefineEvent("Message", [this] {
        auto emit = EmitterWrapAsCallable("Message");
        port_->SetReceiveCallback([emit](v8::Local<v8::Value> v) {
            emit({v});
        });
    }, [this] {
        port_->SetReceiveCallback({});
    });

    EmitterDefineEvent("Error", [this] {
        auto emit = EmitterWrapAsCallable("Error");
        port_->SetErrorCallback([emit](const std::string& err) {
            v8::Isolate *isolate = v8::Isolate::GetCurrent();
            emit({binder::to_v8(isolate, err)});
        });
    }, [this] {
        port_->SetErrorCallback({});
    });
}

v8::Local<v8::Object> MessagePortWrap::OnGetObjectSelf(v8::Isolate *isolate)
{
    return GetObjectWeakReference().Get(isolate);
}

void MessagePortWrap::CheckClosedPort()
{
    if (port_->IsDetached())
        g_throw(Error, "Message port has been closed or transferred");
}

void MessagePortWrap::close()
{
    CheckClosedPort();
    EmitterDispose();
    port_->DetachFromEventLoop();
    port_->SetReceiveCallback({});
    port_->SetErrorCallback({});
}

void MessagePortWrap::postMessage(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    CheckClosedPort();

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    v8::Local<v8::Value> message;
    std::vector<v8::Local<v8::Value>> transfers;
    if (info.Length() == 1)
    {
        message = info[0];
    }
    else if (info.Length() == 2)
    {
        message = info[0];
        if (!info[1]->IsArray())
            g_throw(TypeError, "Argument `transfers` must be an array of values");

        auto array = info[1].As<v8::Array>();
        transfers.reserve(array->Length());
        for (uint32_t i = 0; i < array->Length(); i++)
        {
            v8::Local<v8::Value> v;
            if (!array->Get(context, i).ToLocal(&v))
                g_throw(Error, "Argument `transfers` is an invalid array");
            transfers.emplace_back(v);
        }
    }
    else
    {
        g_throw(TypeError, "Invalid number of arguments, expecting 1 or 2");
    }

    auto maybe = port_->PostMessage(message, transfers);
    if (maybe.IsNothing())
        return;

    if (!maybe.ToChecked())
        g_throw(Error, "Failed to post message");
}

GALLIUM_BINDINGS_WORKERS_NS_END
