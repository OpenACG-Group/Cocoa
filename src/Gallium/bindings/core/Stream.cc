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

#include "uv.h"
#include "jemalloc/jemalloc.h"
#include "fmt/format.h"

#include "Core/EventLoop.h"
#include "Gallium/Gallium.h"
#include "Gallium/bindings/core/Exports.h"
#include "Gallium/binder/CallV8.h"
GALLIUM_BINDINGS_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Gallium.bindings.core)

StreamWrap::StreamWrap(uv_stream_t *handle)
    : disposed_(false)
    , stream_handle_(handle)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    handle->data = this;

    v8::Local<v8::Object> iterator = binder::NewObject<StreamAsyncIterator>(isolate, this);
    async_iterator_ = binder::UnwrapObject<StreamAsyncIterator>(isolate, iterator);

    CHECK(async_iterator_);
    async_iterator_obj_.Reset(isolate, iterator);
}

StreamWrap::~StreamWrap()
{
    Dispose();
}

bool StreamWrap::isWritable() const
{
    return uv_is_writable(stream_handle_);
}

bool StreamWrap::isReadable() const
{
    return uv_is_readable(stream_handle_);
}

v8::Local<v8::Value> StreamWrap::asyncIterator()
{
    if (disposed_)
        g_throw(Error, "Stream has already been disposed");

    CHECK(!async_iterator_obj_.IsEmpty());
    return async_iterator_obj_.Get(v8::Isolate::GetCurrent());
}

namespace {

// NOLINTNEXTLINE
struct AsyncWriteClosure
{
    v8::Isolate *isolate;
    std::vector<v8::Global<v8::Object>> buffer_objs;
    std::vector<uv_buf_t> buffers;
    v8::Global<v8::Promise::Resolver> resolver;
    uv_write_t req;
};

} // namespace anonymous

v8::Local<v8::Value> StreamWrap::write(v8::Local<v8::Value> buffers)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!buffers->IsArray())
        g_throw(TypeError, "Argument `buffers` must be an array of `Buffer`");

    v8::Local<v8::Array> array = buffers.As<v8::Array>();
    uint32_t length = array->Length();
    if (length == 0)
        g_throw(TypeError, "No buffer is provided");

    auto *closure = new AsyncWriteClosure;
    closure->isolate = isolate;
    closure->buffer_objs.resize(length);
    closure->buffers.resize(length);

    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    for (uint32_t i = 0; i < length; i++)
    {
        auto E = array->Get(ctx, i).FromMaybe(v8::Local<v8::Value>());
        if (E.IsEmpty())
            g_throw(TypeError, "Argument `buffers` must be an array of `Buffer`");

        Buffer *ptr = binder::UnwrapObject<Buffer>(isolate, E);
        if (!ptr)
            g_throw(TypeError, "Argument `buffers` must be an array of `Buffer`");

        closure->buffer_objs[i].Reset(isolate, E.As<v8::Object>());

        closure->buffers[i].base = reinterpret_cast<char*>(ptr->addressU8());
        closure->buffers[i].len = ptr->length();
    }

    auto resolver = v8::Promise::Resolver::New(ctx).ToLocalChecked();
    closure->resolver.Reset(isolate, resolver);

    uv_write(&closure->req, stream_handle_, closure->buffers.data(), length,
             [](uv_write_t *req, int status) {
        auto *closure = reinterpret_cast<AsyncWriteClosure*>(req->data);
        CHECK(closure);

        v8::HandleScope scope(closure->isolate);

        v8::Local<v8::Context> ctx = closure->isolate->GetCurrentContext();
        auto resolver = closure->resolver.Get(closure->isolate);
        if (status == 0)
            resolver->Resolve(ctx, v8::Undefined(closure->isolate)).Check();
        else
            resolver->Reject(ctx, v8::Int32::New(closure->isolate, status)).Check();

        delete closure;
    });
    closure->req.data = closure;

    return resolver->GetPromise();
}

void StreamWrap::Dispose()
{
    if (disposed_)
        return;

    async_iterator_->Dispose();
    async_iterator_obj_.Reset();
    disposed_ = true;
}

StreamAsyncIterator::StreamAsyncIterator(StreamWrap *stream)
    : disposed_(false)
    , stream_(stream)
    , pending_(false)
{
}

StreamAsyncIterator::~StreamAsyncIterator() = default;

void StreamAsyncIterator::Dispose()
{
    if (disposed_)
        return;

    if (pending_)
        FinishPendingState();

    stream_ = nullptr;
    disposed_ = true;
}

void StreamWrap::OnAllocateCallback(uv_handle_t *handle, size_t suggested, uv_buf_t *result)
{
    auto *stream = reinterpret_cast<StreamWrap*>(handle->data);
    CHECK(stream);

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    v8::Local<v8::Object> buffer_obj = Buffer::MakeFromSize(suggested);
    stream->async_iterator_->SetCurrentBuffer(isolate, buffer_obj);

    auto *buffer = binder::UnwrapObject<Buffer>(isolate, buffer_obj);
    CHECK(buffer);

    result->base = reinterpret_cast<char*>(buffer->addressU8());
    result->len = buffer->length();
}

void StreamWrap::OnReadCallback(uv_stream_t *handle, ssize_t nread, const uv_buf_t *buf)
{
    auto *stream = reinterpret_cast<StreamWrap*>(handle->data);
    CHECK(stream);

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    bool done;
    bool error = false;
    v8::Local<v8::Value> value = v8::Undefined(isolate);

    if (nread == UV_EOF)
    {
        /* We have reached EOF */
        done = true;
    }
    else if (nread < 0)
    {
        /* An error was occurred */
        error = true;
        value = binder::to_v8(isolate, uv_strerror(static_cast<int>(nread)));
    }
    else if (nread == 0)
    {
        /* EAGAIN or EWOULDBLOCK */
        done = false;
    }
    else
    {
        CHECK(nread <= 0xffffffffLL);

        done = false;
        v8::Local<v8::Object> buffer = stream->async_iterator_->GetCurrentBuffer(isolate);
        std::unordered_map<std::string_view, v8::Local<v8::Value>> result{
            { "length", v8::Uint32::NewFromUnsigned(isolate, static_cast<uint32_t>(nread)) },
            { "buffer", buffer }
        };
        value = binder::to_v8(isolate, result);
    }

    v8::Local<v8::Promise::Resolver> resolver =
            stream->async_iterator_->GetCurrentResolver(isolate);

    if (error)
    {
        resolver->Reject(isolate->GetCurrentContext(), value).Check();
    }
    else
    {
        std::unordered_map<std::string_view, v8::Local<v8::Value>> result{
            { "done", v8::Boolean::New(isolate, done) },
            { "value", value }
        };
        resolver->Resolve(
                isolate->GetCurrentContext(), binder::to_v8(isolate, result)).Check();
    }

    stream->async_iterator_->FinishPendingState();
}

v8::Local<v8::Promise> StreamAsyncIterator::EnterPendingState()
{
    CHECK(!pending_);

    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    int ret = uv_read_start(stream_->stream_handle_,
                            StreamWrap::OnAllocateCallback, StreamWrap::OnReadCallback);
    if (ret < 0)
        g_throw(Error, fmt::format("Failed to start reading: {}", uv_strerror(ret)));

    auto resolver = v8::Promise::Resolver::New(isolate->GetCurrentContext()).ToLocalChecked();
    current_resolver_.Reset(isolate, resolver);

    pending_ = true;
    return resolver->GetPromise();
}

void StreamAsyncIterator::FinishPendingState()
{
    CHECK(pending_);

    uv_read_stop(stream_->stream_handle_);

    pending_ = false;
    current_resolver_.Reset();
    current_buffer_.Reset();
}

v8::Local<v8::Value> StreamAsyncIterator::next()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (disposed_)
    {
        std::unordered_map<std::string_view, v8::Local<v8::Value>> result{
            { "done", v8::True(isolate) }
        };
        return binder::to_v8(isolate, result);
    }

    if (pending_)
        g_throw(Error, "`next` should not be called before current promise is fulfilled");

    CHECK(stream_);

    v8::Local<v8::Promise> promise = EnterPendingState();
    CHECK(!promise.IsEmpty());

    return promise;
}

void StreamAsyncIterator::return_(v8::FunctionCallbackInfo<v8::Value> info)
{
    if (pending_)
        FinishPendingState();

    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    v8::Local<v8::Value> value;
    if (info.Length() >= 1)
        value = info[0];
    else
        value = v8::Undefined(isolate);

    std::unordered_map<std::string_view, v8::Local<v8::Value>> result{
        { "done", v8::True(isolate) },
        { "value", value }
    };

    info.GetReturnValue().Set(binder::to_v8(isolate, result));
}

void StreamAsyncIterator::throw_(v8::FunctionCallbackInfo<v8::Value> info)
{
    if (pending_)
        FinishPendingState();

    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    std::unordered_map<std::string_view, v8::Local<v8::Value>> result{
        { "done", v8::True(isolate) }
    };

    info.GetReturnValue().Set(binder::to_v8(isolate, result));
}

GALLIUM_BINDINGS_NS_END
