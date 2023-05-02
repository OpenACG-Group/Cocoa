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

#include <cstring>

#include "include/v8.h"

#include "Gallium/bindings/core/Exports.h"
#include "Gallium/binder/Class.h"
GALLIUM_BINDINGS_NS_BEGIN

CallbackScopedBuffer::FactoryRet
CallbackScopedBuffer::MakeScoped(uint8_t *ptr, size_t size, bool readonly)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    auto *buf = new CallbackScopedBuffer(ptr, size, readonly);
    v8::Local<v8::Object> obj =
            binder::Class<CallbackScopedBuffer>::import_external(isolate, buf);

    return {obj, buf};
}

bool CallbackScopedBuffer::writable() const
{
    CheckScope();
    return !readonly_;
}

size_t CallbackScopedBuffer::length() const
{
    CheckScope();
    return size_;
}

v8::Local<v8::Value> CallbackScopedBuffer::read(v8::Local<v8::Value> dst,
                                                int64_t offset,
                                                int64_t size)
{
    CheckScope();
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    if (offset > size_)
        g_throw(RangeError, "Invalid data offset");

    size = std::min(size, static_cast<int64_t>(size_) - offset);
    if (size == 0)
        return v8::Number::New(isolate, 0);

    if (!dst->IsUint8Array() || !dst.As<v8::Uint8Array>()->HasBuffer())
        g_throw(TypeError, "Argument `dst` must be an allocated Uint8Array");

    v8::Local<v8::Uint8Array> arr = dst.As<v8::Uint8Array>();

    if (size > arr->ByteLength())
        g_throw(RangeError, "Buffer `dst` is too small to hold data");

    uint8_t *ptr = reinterpret_cast<uint8_t*>(arr->Buffer()->Data())
                    + arr->ByteOffset();

    std::memcpy(ptr, ptr_ + offset, size);

    return v8::Number::New(isolate, static_cast<double>(size));
}

v8::Local<v8::Value> CallbackScopedBuffer::write(v8::Local<v8::Value> src,
                                                 int64_t offset)
{
    CheckScope();
    if (readonly_)
        g_throw(Error, "Buffer is readonly");

    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    if (offset > size_)
        g_throw(RangeError, "Invalid data offset");

    if (!src->IsUint8Array() || !src.As<v8::Uint8Array>()->HasBuffer())
        g_throw(TypeError, "Argument `src` must be an allocated Uint8Array");

    v8::Local<v8::Uint8Array> arr = src.As<v8::Uint8Array>();

    size_t size = std::min(arr->ByteLength(), size_ - offset);
    if (size == 0)
        return v8::Number::New(isolate, 0);

    uint8_t *ptr = reinterpret_cast<uint8_t*>(arr->Buffer()->Data())
                   + arr->ByteOffset();

    std::memcpy(ptr_ + offset, ptr, size);

    return v8::Number::New(isolate, static_cast<double>(size));
}

void CallbackScopedBuffer::leaveScope()
{
    ptr_ = nullptr;
    size_ = 0;
}

void CallbackScopedBuffer::CheckScope() const
{
    if (!ptr_)
        g_throw(Error, "Scoped buffer has been disposed (out of scope)");
}

CallbackScopedBuffer::ScopeGuard::ScopeGuard(v8::Local<v8::Value> obj,
                                             CallbackScopedBuffer *buf)
    : obj_(obj), buf_(buf)
{
    CHECK(buf);
}

CallbackScopedBuffer::ScopeGuard::~ScopeGuard()
{
    buf_->leaveScope();
}

GALLIUM_BINDINGS_NS_END
