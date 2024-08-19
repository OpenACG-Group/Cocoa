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

ffi::RetLocal<v8::Value> MessagePortWrap::MakeConnectedPair()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    RuntimeBase *runtime = RuntimeBase::FromIsolate(isolate);
    CHECK(runtime);
    auto pair = MessagePort::MakeConnectedPair(runtime->GetEventLoop());

    v8::Local<v8::Value> ports[] = {
        ffi::JSObject::New<MessagePortWrap>(isolate, std::move(pair.first)),
        ffi::JSObject::New<MessagePortWrap>(isolate, std::move(pair.second))
    };
    return v8::Array::New(isolate, ports, 2);
}

namespace {

class TransferData : public ffi::JSTransferData
{
public:
    explicit TransferData(std::shared_ptr<MessagePort> port)
        : port_(std::move(port)) {}
    ~TransferData() override = default;

    ffi::RetLocal<v8::Object> Construct(v8::Isolate *isolate,
                                        v8::Local<v8::Context> context) override
    {
        // The port has been delivered to the destination thread, and
        // attach it to the new event loop.
        uv_loop_t *event_loop = RuntimeBase::FromIsolate(isolate)->GetEventLoop();
        port_->AttachToEventLoop(event_loop);

        return ffi::JSObject::New<MessagePortWrap>(isolate, port_);
    }

private:
    std::shared_ptr<MessagePort> port_;
};

} // namespace anonymous

MessagePortWrap::MessagePortWrap(std::shared_ptr<MessagePort> port)
    : port_(std::move(port))
{
    EmitterDefineEvent("message", [this] {
        auto emit = EmitterWrapAsCallable("message");
        port_->SetReceiveCallback([emit](v8::Local<v8::Value> v) {
            emit({v});
        });
        return 0;
    }, [this](uint64_t) {
        port_->SetReceiveCallback({});
    });

    EmitterDefineEvent("error", [this] {
        auto emit = EmitterWrapAsCallable("error");
        port_->SetErrorCallback([emit](const std::string& err) {
            v8::Isolate *isolate = v8::Isolate::GetCurrent();
            emit({ffi::Cast<std::string>::ToChecked(isolate, err)});
        });
        return 0;
    }, [this](uint64_t) {
        port_->SetErrorCallback({});
    });
}

std::unique_ptr<ffi::JSTransferData>
MessagePortWrap::OnObjectTransfer(v8::Isolate *isolate)
{
    // Close the port (detach it from the current event loop) first.
    // It should not attach to any event loop until it is delivered
    // to the destination thread.
    this->close();
    return std::make_unique<TransferData>(std::move(port_));
}

ffi::Ret<void> MessagePortWrap::close()
{
    ffi::JSObject::NotifyDisposeState(DisposeState::kDisposing);
    EmitterDispose();
    port_->DetachFromEventLoop();
    port_->SetReceiveCallback({});
    port_->SetErrorCallback({});
    ffi::JSObject::NotifyDisposeState(DisposeState::kDisposed);
    return {};
}

ffi::Ret<void> MessagePortWrap::postMessage(v8::Local<v8::Value> message,
                                            ffi::OptLocal<v8::Array> transfers)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    std::vector<v8::Local<v8::Value>> transfers_vec;
    if (transfers)
    {
        v8::Local<v8::Array> array = *transfers;
        transfers_vec.reserve(array->Length());
        for (uint32_t i = 0; i < array->Length(); i++)
        {
            v8::Local<v8::Value> v;
            if (!array->Get(context, i).ToLocal(&v))
                return ffi::Fail(ffi::kErr, "Argument `transfers` is an invalid array");
            transfers_vec.emplace_back(v);
        }
    }

    return port_->PostMessage(message, transfers_vec);
}

GALLIUM_BINDINGS_WORKERS_NS_END
