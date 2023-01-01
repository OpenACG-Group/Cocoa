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
    : stream_handle_(handle)
    , is_reading_(false)
    , owned_buf_{nullptr, 0}
{
    uv_handle_set_data(reinterpret_cast<uv_handle_t *>(stream_handle_), this);
}

StreamWrap::~StreamWrap()
{
    if (owned_buf_.base)
    {
        ::free(owned_buf_.base);
        owned_buf_.base = nullptr;
    }
    clearIterationState();

    uv_close(reinterpret_cast<uv_handle_t *>(stream_handle_), [](uv_handle_t *hnd) {
        ::free(hnd);
    });
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
    if (is_reading_)
        g_throw(Error, "Iterating an stream which is in iteration is not allowed");

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Object> self = binder::Class<StreamWrap>::find_object(isolate, this);
    CHECK(!self.IsEmpty() && !self->IsNullOrUndefined());
    return binder::Class<StreamReadIterator>::create_object(isolate, self, this);
}

void StreamWrap::clearIterationState()
{
    is_reading_ = false;
    current_iterate_promise_.Reset();
}

StreamWeakBuffer::StreamWeakBuffer(v8::Local<v8::Object> streamWrap, uv_buf_t *buf,
                                   size_t readBytes)
    : stream_wrap_ref_(v8::Isolate::GetCurrent(), streamWrap)
    , weak_buf_(buf)
    , read_bytes_(readBytes)
{
    CHECK(weak_buf_->base && weak_buf_->len > 0);
}

StreamWeakBuffer::~StreamWeakBuffer()
{
    stream_wrap_ref_.Reset();
}

v8::Local<v8::Value> StreamWeakBuffer::toStrongOwnership()
{
    /* read_bytes_ is always not bigger than weak_buf_->len */
    void *pb = weak_buf_->base;
    if (weak_buf_->len != read_bytes_)
        pb = ::realloc(weak_buf_->base, read_bytes_);
    v8::Local<v8::Object> buf = Buffer::MakeFromPtrWithoutCopy(pb, read_bytes_,
        [](void *ptr, g_maybe_unused size_t len, g_maybe_unused void *d) {
            ::free(ptr);
        }, nullptr);

    stream_wrap_ref_.Reset();
    weak_buf_->base = nullptr;
    weak_buf_->len = 0;
    return buf;
}

bool StreamWeakBuffer::isExpired() const
{
    return (weak_buf_->base == nullptr);
}

StreamReadIterator::StreamReadIterator(v8::Local<v8::Object> streamWrap, StreamWrap *pStream)
    : stream_wrap_ref_(v8::Isolate::GetCurrent(), streamWrap)
    , stream_(pStream)
{
    stream_->is_reading_ = true;
}

StreamReadIterator::~StreamReadIterator()
{
    stream_wrap_ref_.Reset();
}

namespace {

void on_allocator_callback(uv_handle_t *hnd, size_t suggestedSize, uv_buf_t *result)
{
    auto *wrap = reinterpret_cast<StreamWrap *>(uv_handle_get_data(hnd));
    CHECK(wrap);

    size_t size = suggestedSize;
    if (!wrap->owned_buf_.base)
    {
        wrap->owned_buf_.base = reinterpret_cast<char *>(malloc(size));
        CHECK(wrap->owned_buf_.base);
        wrap->owned_buf_.len = size;
    }

    result->base = wrap->owned_buf_.base;
    result->len = wrap->owned_buf_.len;
}

void on_read_callback(uv_stream_t *st, ssize_t nread, const uv_buf_t *buf)
{
    auto *hnd = reinterpret_cast<uv_handle_t *>(st);
    auto *wrap = reinterpret_cast<StreamWrap *>(uv_handle_get_data(hnd));

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
        done = false;
        v8::Local<v8::Object> streamObject = binder::Class<StreamWrap>::find_object(isolate, wrap);
        value = binder::Class<StreamWeakBuffer>::create_object(isolate, streamObject, &wrap->owned_buf_, nread);
    }

    v8::Local<v8::Promise::Resolver> resolver = wrap->current_iterate_promise_.Get(isolate);

    if (error)
    {
        resolver->Reject(isolate->GetCurrentContext(),
                         v8::Exception::Error(value.As<v8::String>())).Check();
    }
    else
    {
        v8::Local<v8::Object> result = binder::to_v8(isolate, std::map<std::string, bool>{{"done", done}});
        result->Set(isolate->GetCurrentContext(), binder::to_v8(isolate, "value"), value).Check();
        resolver->Resolve(isolate->GetCurrentContext(), result).Check();
    }

    wrap->current_iterate_promise_.Reset();
    uv_read_stop(wrap->stream_handle_);
}

} // namespace anonymous

v8::Local<v8::Value> StreamReadIterator::next() const
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    v8::Local<v8::Promise::Resolver> resolver = v8::Promise::Resolver::New(ctx).ToLocalChecked();

    int ret = uv_read_start(stream_->stream_handle_, on_allocator_callback, on_read_callback);
    if (ret < 0)
    {
        g_throw(Error, fmt::format("Failed to start reading: {}", uv_strerror(ret)));
    }
    stream_->current_iterate_promise_.Reset(isolate, resolver);
    return resolver->GetPromise();
}

v8::Local<v8::Value> StreamReadIterator::return_() const
{
    if (stream_->owned_buf_.base)
    {
        ::free(stream_->owned_buf_.base);
        stream_->owned_buf_.base = nullptr;
    }
    stream_->clearIterationState();

    v8::Isolate *i = v8::Isolate::GetCurrent();
    return binder::to_v8(i, std::map<std::string, bool>{{"done", true}});
}

v8::Local<v8::Value> StreamReadIterator::throw_()
{
    // TODO: should we implement this?
    return v8::Undefined(v8::Isolate::GetCurrent());
}

v8::Local<v8::Value> StreamWrap::OpenTTYStdin()
{
    v8::Isolate *i = v8::Isolate::GetCurrent();

    auto *tty = StreamWrap::Allocate<uv_tty_t>();
    int ret = uv_tty_init(EventLoop::Ref().handle(), tty, 0, true);
    if (ret < 0)
    {
        ::free(tty);
        g_throw(Error, fmt::format("Failed in reopening TTY: {}", uv_strerror(ret)));
    }

    return binder::Class<StreamWrap>::create_object(i, reinterpret_cast<uv_stream_t *>(tty));
}

GALLIUM_BINDINGS_NS_END
