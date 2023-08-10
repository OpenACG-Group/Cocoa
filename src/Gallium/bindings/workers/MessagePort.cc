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

#include "fmt/format.h"

#include "Core/Errors.h"
#include "Core/Journal.h"
#include "Gallium/bindings/workers/MessagePort.h"
#include "Gallium/bindings/workers/Exports.h"
#include "Gallium/binder/Class.h"
#include "Gallium/RuntimeBase.h"
GALLIUM_BINDINGS_WORKERS_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Gallium.bindings.workers.MessagePort)

MessagePort::PortPair MessagePort::MakeConnectedPair(uv_loop_t *loop)
{
    auto port1 = std::make_shared<MessagePort>(loop);
    auto port2 = std::make_shared<MessagePort>(loop);
    port1->peer_port_ = port2;
    port2->peer_port_ = port1;

    return std::make_pair(port1, port2);
}

MessagePort::MessagePort(uv_loop_t *loop)
    : port_detached_(!loop)
{
    if (loop)
    {
        message_notifier_.emplace(loop, [&] { OnMessageReceive(); });
        message_notifier_->Unref();
    }
}

void MessagePort::DetachFromEventLoop()
{
    if (port_detached_)
        return;

    std::scoped_lock<std::mutex> lock(queue_lock_);
    message_notifier_.reset();
    while (!recv_queue_.empty())
        recv_queue_.pop();
    port_detached_ = true;
}

bool MessagePort::AttachToEventLoop(uv_loop_t *event_loop)
{
    if (!port_detached_)
        return false;

    std::scoped_lock<std::mutex> lock(queue_lock_);
    message_notifier_.emplace(event_loop, [&] { OnMessageReceive(); });
    port_detached_ = false;

    if (!receive_callback_)
        message_notifier_->Unref();

    return true;
}

void MessagePort::SetReceiveCallback(ReceiveCallback callback)
{
    receive_callback_ = std::move(callback);

    if (receive_callback_ && message_notifier_)
        message_notifier_->Ref();
    else if (message_notifier_)
        message_notifier_->Unref();
}

void MessagePort::SetErrorCallback(ErrorCallback callback)
{
    error_callback_ = std::move(callback);
}

namespace {

class SerializerDelegate : public v8::ValueSerializer::Delegate
{
public:
    explicit SerializerDelegate(const std::vector<v8::Local<v8::Value>>& transfer_list)
        : message_(std::make_unique<MessagePort::Message>())
        , serializer_(nullptr)
        , transfer_list_(transfer_list) {}

    void SetSerializer(v8::ValueSerializer *ptr)
    {
        serializer_ = ptr;
    }

    void ThrowDataCloneError(v8::Local<v8::String> message) override {
        v8::Isolate *isolate = v8::Isolate::GetCurrent();
        isolate->ThrowError(message);
    }

    bool HasCustomHostObject(v8::Isolate *isolate) override {
        return true;
    }

    // By default, V8 recognize an `v8::Object` as a host object
    // if its internal field count is not 0.
    // See `ValueSerializer::Delegate::IsHostObject` defined in
    // `//third_party/v8/src/api/api.cc`. But for Cocoa, exported
    // objects (host objects) always have `binder::kInternalFieldsCount`
    // internal fields.
    v8::Maybe<bool> IsHostObject(v8::Isolate *isolate,
                                 v8::Local<v8::Object> object) override
    {
        return v8::Just(object->InternalFieldCount()
                        == binder::kInternalFieldsCount);
    }

    v8::Maybe<bool> WriteHostObject(v8::Isolate *isolate,
                                    v8::Local<v8::Object> object) override
    {
        // `Descriptor` can be treated as a "metaclass" of `object`
        ExportableObjectBase::Descriptor *descriptor =
                binder::UnwrapObjectDescriptor(isolate, object);
        if (!descriptor)
        {
            isolate->ThrowError("Failed to get the descriptor of host object");
            return v8::Nothing<bool>();
        }

        for (uint32_t i = 0; i < host_objects_.size(); i++)
        {
            if (descriptor->GetBase() != host_objects_[i].base)
                continue;
            serializer_->WriteUint32(i);
            return v8::Just(true);
        }

        ExportableObjectBase::SerializerFunc pfn_serialize;
        if (IsInTransferList(object))
            pfn_serialize = descriptor->GetTransferSerializer();
        else
            pfn_serialize = descriptor->GetCloneSerializer();
        if (!pfn_serialize)
        {
            isolate->ThrowError("Object does not support transfer or clone");
            return v8::Nothing<bool>();
        }

        // Calling with `pretest == true` does not transfer or clone the
        // object. Instead, we just check whether the object can be cloned
        // or transferred.
        if (pfn_serialize(isolate, descriptor->GetBase(), true).IsNothing())
        {
            isolate->ThrowError("Object cannot be cloned or transferred. "
                                "Maybe it has been transferred to other contexts.");
            return v8::Nothing<bool>();
        }

        host_objects_.push_back(HostObject{
            descriptor->GetBase(),
            pfn_serialize
        });
        serializer_->WriteUint32(host_objects_.size() - 1);
        return v8::Just(true);
    }

    v8::Maybe<uint32_t> GetSharedArrayBufferId(
            v8::Isolate *isolate,
            v8::Local<v8::SharedArrayBuffer> shared_array_buffer) override
    {
        for (uint32_t i = 0; i < seen_shared_abs_.size(); i++)
        {
            if (seen_shared_abs_[i] == shared_array_buffer)
                return v8::Just(i);
        }

        message_->shared_array_buffers.emplace_back(shared_array_buffer->GetBackingStore());
        seen_shared_abs_.emplace_back(shared_array_buffer);
        return v8::Just<uint32_t>(seen_shared_abs_.size() - 1);
    }

    v8::Maybe<uint32_t> GetWasmModuleTransferId(
            v8::Isolate *isolate,
            v8::Local<v8::WasmModuleObject> module) override
    {
        message_->wasm_modules.emplace_back(module->GetCompiledModule());
        return v8::Just<uint32_t>(message_->wasm_modules.size() - 1);
    }

    std::unique_ptr<MessagePort::Message> Finalize()
    {
        v8::Isolate *isolate = v8::Isolate::GetCurrent();
        for (const HostObject& host_object : host_objects_)
        {
            auto data = host_object.serialize(
                    isolate, host_object.base, false).ToChecked();
            CHECK(data);
            message_->flattened_objects.emplace_back(data);
        }

        // Let `message_` take over the ownership of
        // serialized buffer
        auto [data, size] = serializer_->Release();
        message_->payload = MessagePort::Message::PayloadArray(data, [](const uint8_t *ptr) {
            std::free(const_cast<uint8_t*>(ptr));
        });
        message_->payload_size = size;
        return std::move(message_);
    }

private:
    bool IsInTransferList(v8::Local<v8::Value> v)
    {
        auto itr = std::find(transfer_list_.begin(), transfer_list_.end(), v);
        return (itr != transfer_list_.end());
    }

    struct HostObject
    {
        ExportableObjectBase *base;
        ExportableObjectBase::SerializerFunc serialize;
    };

    std::unique_ptr<MessagePort::Message>  message_;
    v8::ValueSerializer                   *serializer_;
    const std::vector<v8::Local<v8::Value>>&        transfer_list_;
    std::vector<v8::Local<v8::SharedArrayBuffer>>   seen_shared_abs_;
    std::vector<HostObject>                         host_objects_;
};

} // namespace anonymous

v8::Maybe<bool> MessagePort::PostMessage(v8::Local<v8::Value> message,
                                         const std::vector<v8::Local<v8::Value>>& transfer_list)
{
    if (port_detached_)
        return v8::Just(false);

    auto peer = peer_port_.lock();
    if (!peer)
        return v8::Just(true);

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    SerializerDelegate delegate(transfer_list);
    v8::ValueSerializer serializer(isolate, &delegate);
    delegate.SetSerializer(&serializer);

    std::vector<v8::Local<v8::ArrayBuffer>> array_buffers;
    for (v8::Local<v8::Value> value : transfer_list)
    {
        if (value->IsArrayBuffer())
        {
            auto ab = value.As<v8::ArrayBuffer>();
            if (!ab->IsDetachable())
                g_throw(Error, "ArrayBuffer in transfer list is not detachable");

            auto itr = std::find(array_buffers.begin(),
                                 array_buffers.end(),
                                 ab);
            if (itr != array_buffers.end())
                g_throw(Error, "Duplicate ArrayBuffer in transfer list");

            array_buffers.emplace_back(ab);
            serializer.TransferArrayBuffer(array_buffers.size() - 1, ab);
        }
        else if (binder::UnwrapObjectDescriptor(isolate, value))
        {
            auto *desc = binder::UnwrapObjectDescriptor(isolate, value);
            // The source port and destination should not appear in the
            // transfer list. Let's check it.
            if (desc->IsMessagePort())
            {
                auto *wrap = desc->GetBase()->Cast<MessagePortWrap>();
                if (wrap->GetPort() && wrap->GetPort().get() == this)
                    g_throw(Error, "Transfer list contains the source port");
                if (wrap->GetPort() && wrap->GetPort() == peer)
                    g_throw(Error, "Transfer list contains the destination port");
            }
        }
    }

    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();

    serializer.WriteHeader();
    if (serializer.WriteValue(ctx, message).IsNothing())
        return v8::Nothing<bool>();

    std::unique_ptr<MessagePort::Message> port_message = delegate.Finalize();

    for (v8::Local<v8::ArrayBuffer> ab : array_buffers)
    {
        port_message->array_buffers.emplace_back(ab->GetBackingStore());
        ab->Detach();
    }

    return v8::Just(PostSerializedMessage(peer, std::move(port_message)));
}

bool MessagePort::PostSerializedMessage(const std::shared_ptr<MessagePort>& peer,
                                        std::unique_ptr<Message> message)
{
    std::scoped_lock<std::mutex> lock(peer->queue_lock_);
    if (peer->port_detached_)
        return false;

    CHECK(peer->message_notifier_);
    peer->recv_queue_.emplace(std::move(message));
    peer->message_notifier_->Send();

    return true;
}

void MessagePort::OnMessageReceive()
{
    std::vector<std::unique_ptr<Message>> messages;
    {
        std::scoped_lock<std::mutex> lock(queue_lock_);
        while (!recv_queue_.empty())
        {
            messages.emplace_back(std::move(recv_queue_.front()));
            recv_queue_.pop();
        }
    }

    if (!receive_callback_)
        return;
    for (const std::unique_ptr<Message>& msg : messages)
    {
        ReceiveSerializedMessage(*msg);
        // Callback function calls `DetachFromEventLoop()`
        if (port_detached_)
            break;
    }
}

namespace {

class DeserializerDelegate : public v8::ValueDeserializer::Delegate
{
public:
    DeserializerDelegate(v8::Isolate *isolate,
                         const MessagePort::Message *message)
        : message_(message), deserializer_(nullptr)
    {
        for (const auto& store : message->shared_array_buffers)
        {
            auto ab = v8::SharedArrayBuffer::New(isolate, store);
            shared_array_buffers_.emplace_back(ab);
        }
    }

    void SetDeserializer(v8::ValueDeserializer *ptr)
    {
        deserializer_ = ptr;
    }

    v8::MaybeLocal<v8::Object> ReadHostObject(v8::Isolate *isolate) override
    {
        uint32_t id;
        if (!deserializer_->ReadUint32(&id))
        {
            isolate->ThrowError("Failed to read host object ID");
            return {};
        }
        CHECK(id < message_->flattened_objects.size());
        return message_->flattened_objects[id]->Deserialize(
                isolate, isolate->GetCurrentContext());
    }

    v8::MaybeLocal<v8::SharedArrayBuffer>
    GetSharedArrayBufferFromId(v8::Isolate *isolate, uint32_t clone_id) override
    {
        CHECK(clone_id < shared_array_buffers_.size());
        return shared_array_buffers_[clone_id];
    }

    v8::MaybeLocal<v8::WasmModuleObject>
    GetWasmModuleFromId(v8::Isolate *isolate, uint32_t transfer_id) override
    {
        CHECK(transfer_id < message_->wasm_modules.size());
        return v8::WasmModuleObject::FromCompiledModule(
                isolate, message_->wasm_modules[transfer_id]);
    }

private:
    const MessagePort::Message  *message_;
    v8::ValueDeserializer       *deserializer_;
    std::vector<v8::Local<v8::SharedArrayBuffer>> shared_array_buffers_;
};

} // namespace anonymous

void MessagePort::ReceiveSerializedMessage(Message& message)
{
    if (!receive_callback_)
        return;

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::HandleScope handle_scope(isolate);

    DeserializerDelegate delegate(isolate, &message);
    v8::ValueDeserializer deserializer(isolate,
                                       message.payload.get(),
                                       message.payload_size,
                                       &delegate);
    delegate.SetDeserializer(&deserializer);

    // Attach ArrayBuffers to the new Isolate
    for (uint32_t i = 0; i < message.array_buffers.size(); i++)
    {
        auto ab = v8::ArrayBuffer::New(isolate, message.array_buffers[i]);
        deserializer.TransferArrayBuffer(i, ab);
    }

    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    v8::TryCatch try_catch(isolate);

    if (deserializer.ReadHeader(ctx).IsNothing())
    {
        CHECK(try_catch.HasCaught());
        HandleCaughtError(isolate, try_catch);
        return;
    }

    v8::Local<v8::Value> message_body;
    if (!deserializer.ReadValue(ctx).ToLocal(&message_body))
    {
        CHECK(try_catch.HasCaught());
        HandleCaughtError(isolate, try_catch);
        return;
    }

    message.wasm_modules.clear();
    message.flattened_objects.clear();
    message.shared_array_buffers.clear();
    message.array_buffers.clear();
    message.payload.reset();
    message.payload_size = 0;

    receive_callback_(message_body);
    if (try_catch.HasCaught())
    {
        RuntimeBase::FromIsolate(isolate)
            ->ReportUncaughtExceptionInCallback(try_catch);
    }
}

void MessagePort::HandleCaughtError(v8::Isolate *isolate, v8::TryCatch &try_catch)
{
    auto msg = binder::from_v8<std::string>(isolate, try_catch.Message()->Get());
    QLOG(LOG_ERROR, "Message error: {}", msg);
    if (error_callback_)
        error_callback_(msg);
}

GALLIUM_BINDINGS_WORKERS_NS_END
