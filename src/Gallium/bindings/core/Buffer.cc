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

#include "Core/Errors.h"
#include "Core/EventLoop.h"
#include "Core/Utils.h"
#include "Core/Data.h"
#include "Core/QResource.h"

#include "Gallium/Gallium.h"
#include "Gallium/binder/Class.h"
#include "Gallium/binder/CallV8.h"
#include "Gallium/bindings/core/Exports.h"
GALLIUM_BINDINGS_NS_BEGIN

namespace {

constexpr int constexpr_log2ui(uint64_t x)
{
    int a = -1;
    while (x) { x >>= 1; a++; }
    return a;
}

size_t stringByteLength(v8::Isolate *isolate, v8::Local<v8::String> str,
                        Buffer::Encoding encoding)
{
    size_t x = str->Length();
    switch (encoding)
    {
    case Buffer::Encoding::kLatin1:
        return x;
    case Buffer::Encoding::kUtf8:
        return str->Utf8Length(isolate);
    case Buffer::Encoding::kUcs2:
        // p = 2^k (k∈N)
        // => x * p == x << k == x << log2(p)
        return x << constexpr_log2ui(sizeof(uint16_t));
    case Buffer::Encoding::kHex:
        // x/2,     if x = 2k (k∈N)
        // x/2 + 1, if x = 2k+1 (k∈N)
        return (x >> 1) + (x & 1);
    }

    MARK_UNREACHABLE();
}

size_t encodeStringUcs2(v8::Isolate* isolate, char *buf, size_t buflen,
                        v8::Local<v8::String> str, int flags, size_t* chars_written)
{
    auto *const dst = reinterpret_cast<uint16_t*>(buf);

    size_t max_chars = buflen / sizeof(*dst);
    if (max_chars == 0) {
        return 0;
    }

    uint16_t* const aligned_dst = utils::AlignUp(dst, sizeof(*dst));
    size_t nchars;
    if (aligned_dst == dst) {
        nchars = str->Write(isolate, dst, 0, static_cast<int32_t>(max_chars), flags);
        *chars_written = nchars;
        return nchars * sizeof(*dst);
    }

    CHECK(reinterpret_cast<uintptr_t>(aligned_dst) % sizeof(*dst) == 0);

    // Write all but the last char
    max_chars = std::min(max_chars, static_cast<size_t>(str->Length()));
    if (max_chars == 0) return 0;
    nchars = str->Write(isolate, aligned_dst, 0,
                        static_cast<int32_t>(max_chars - 1), flags);
    CHECK(nchars == max_chars - 1);

    // Shift everything to unaligned-left
    memmove(dst, aligned_dst, nchars * sizeof(*dst));

    // One more char to be written
    uint16_t last;
    CHECK(str->Write(isolate, &last, nchars, 1, flags) == 1);
    memcpy(buf + nchars * sizeof(*dst), &last, sizeof(last));
    nchars++;

    *chars_written = nchars;
    return nchars * sizeof(*dst);
}

uint8_t parse_hex_byte(char p0)
{
    if (p0 >= '0' && p0 <= '9')
        return p0 - '0';
    else if (p0 >= 'a' && p0 <= 'f')
        return p0 - 'a' + 10;
    else if (p0 >= 'A' && p0 <= 'F')
        return p0 - 'A' + 10;
    throw std::runtime_error("Unexpected character in hex string");
}

size_t encodeStringHex(v8::Isolate *isolate, uint8_t *dstBuf, v8::Local<v8::String> str)
{
    if (!str->IsOneByte())
        throw std::runtime_error("Hex string must be one-byte encoded");
    auto hex_str = binder::from_v8<std::string>(isolate, str);
    size_t p = hex_str.length() & 1;
    uint8_t *ptr = dstBuf;
    if (p)
        *ptr++ = parse_hex_byte(hex_str[0]);
    while (p < hex_str.length())
    {
        uint8_t r0 = parse_hex_byte(hex_str[p++]);
        uint8_t r1 = parse_hex_byte(hex_str[p++]);
        *ptr++ = (r0 << 4) | r1;
    }
    return (ptr - dstBuf);
}

size_t encodeString(v8::Isolate *isolate, uint8_t *buf, size_t buflen,
                    v8::Local<v8::String> str, Buffer::Encoding encoding, int *charsWritten)
{
    v8::HandleScope scope(isolate);
    size_t nbytes;
    int nchars;

    if (charsWritten == nullptr)
        charsWritten = &nchars;

    int flags = v8::String::HINT_MANY_WRITES_EXPECTED |
                v8::String::NO_NULL_TERMINATION |
                v8::String::REPLACE_INVALID_UTF8;
    switch (encoding)
    {
    case Buffer::Encoding::kLatin1:
        if (str->IsExternalOneByte())
        {
            auto ext = str->GetExternalOneByteStringResource();
            nbytes = std::min(buflen, ext->length());
            memcpy(buf, ext->data(), nbytes);
        }
        else
        {
            auto *const dst = reinterpret_cast<uint8_t*>(buf);
            nbytes = str->WriteOneByte(isolate, dst, 0,
                                       static_cast<int32_t>(buflen), flags);
        }
        CHECK(nbytes <= INT32_MAX);
        *charsWritten = static_cast<int32_t>(nbytes);
        break;

    case Buffer::Encoding::kUtf8:
        nbytes = str->WriteUtf8(isolate, reinterpret_cast<char*>(buf),
                                static_cast<int32_t>(buflen),
                                charsWritten, flags);
        break;

    case Buffer::Encoding::kUcs2:
    {
        size_t tempChars = 0;

        nbytes = encodeStringUcs2(isolate, reinterpret_cast<char *>(buf), buflen, str, flags, &tempChars);
        *charsWritten = static_cast<int>(tempChars);
        if (utils::GetEndianness() == utils::Endian::kBig)
            utils::SwapBytes16(buf, nbytes);
        break;
    }

    case Buffer::Encoding::kHex:
        nbytes = encodeStringHex(isolate, buf, str);
        break;
    }
    return nbytes;
}

v8::Local<v8::Uint8Array> newBuffer(std::size_t length)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::ArrayBuffer> ab = v8::ArrayBuffer::New(isolate, length);
    if (ab.IsEmpty())
        throw std::runtime_error("Memory allocation failed");
    return v8::Uint8Array::New(ab, 0, length);
}

v8::Local<v8::Uint8Array> newBuffer(void *ptr, size_t length, v8::BackingStore::DeleterCallback deleter,
                                    void *closure)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    std::unique_ptr<v8::BackingStore> bs = v8::ArrayBuffer::NewBackingStore(ptr, length, deleter, closure);
    v8::Local<v8::ArrayBuffer> ab = v8::ArrayBuffer::New(isolate, std::move(bs));
    if (ab.IsEmpty())
        g_throw(Error, "Memory allocation failed");
    return v8::Uint8Array::New(ab, 0, length);
}

v8::Local<v8::Uint8Array> newBuffer(v8::Local<v8::String> str, Buffer::Encoding encoding)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    size_t length = stringByteLength(isolate, str, encoding);
    if (length == 0)
        throw std::invalid_argument("Empty string");
    v8::Local<v8::ArrayBuffer> ab = v8::ArrayBuffer::New(isolate, length);
    if (ab.IsEmpty())
        throw std::runtime_error("Memory allocation failed");

    g_maybe_unused int charsWritten;
    size_t actualSize = encodeString(isolate,
                                     reinterpret_cast<uint8_t *>(ab->GetBackingStore()->Data()),
                                     length,
                                     str,
                                     encoding,
                                     &charsWritten);
    CHECK(actualSize > 0);
    return v8::Uint8Array::New(ab, 0, length);
}

} // namespace anonymous

v8::Local<v8::Object> Buffer::MakeFromString(v8::Local<v8::String> string, uint32_t encoding)
{
    if (encoding > static_cast<uint32_t>(Encoding::kLast))
        g_throw(Error, "Invalid encoding name");

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Object> buf = binder::Class<Buffer>::create_object(isolate);
    Buffer *self = binder::Class<Buffer>::unwrap_object(isolate, buf);

    auto array = newBuffer(string, static_cast<Encoding>(encoding));
    self->array_.Reset(isolate, array);
    self->backing_store_ = array->Buffer()->GetBackingStore();

    return buf;
}

v8::Local<v8::Object> Buffer::MakeFromAdoptBuffer(v8::Local<v8::Object> array)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!array->IsUint8Array())
        g_throw(Error, "'array' must be an instance of 'Uint8Array'");

    v8::Local<v8::Object> buf = binder::Class<Buffer>::create_object(isolate);
    Buffer *self = binder::Class<Buffer>::unwrap_object(isolate, buf);

    auto uint8_array = v8::Local<v8::Uint8Array>::Cast(array);
    self->array_.Reset(isolate, uint8_array);
    self->backing_store_ = uint8_array->Buffer()->GetBackingStore();

    return buf;
}

v8::Local<v8::Object> Buffer::MakeFromPackageFile(const std::string& package,
                                                  const std::string& path)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    QResource *resources = QResource::Instance();

    std::shared_ptr<Data> image_data = resources->Lookup(package, path);
    if (!image_data)
        g_throw(Error, "No such path or package existing in the package manager");

    v8::Local<v8::Object> object = Buffer::MakeFromSize(image_data->size());
    Buffer *self = binder::Class<Buffer>::unwrap_object(isolate, object);
    CHECK(self);

    ssize_t result = image_data->read(self->addressU8(),
                                      image_data->size());
    if (result != image_data->size())
    {
        g_throw(Error, fmt::format("Failed to read file {} in package {}",
                                   path, package));
    }

    return object;
}

v8::Local<v8::Object> Buffer::MakeFromCopy(Buffer *other, off_t offset, ssize_t size)
{
    CHECK(other);

    if (size < 0)
    {
        size = static_cast<ssize_t>(other->length()) - offset;
        if (size < 0)
            g_throw(RangeError, "Invalid offset value");
    }
    else
    {
        if (offset + size > other->length())
            g_throw(RangeError, "Invalid offset and size value");
    }

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Object> buf = binder::Class<Buffer>::create_object(isolate);
    Buffer *self = binder::Class<Buffer>::unwrap_object(isolate, buf);

    v8::Local<v8::Uint8Array> arr = newBuffer(size);
    self->array_.Reset(isolate, arr);
    self->backing_store_ = arr->Buffer()->GetBackingStore();

    std::memcpy(self->addressU8(),
                other->addressU8() + offset,
                size);

    return buf;
}

v8::Local<v8::Object> Buffer::MakeFromSize(size_t size)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Object> buf = binder::Class<Buffer>::create_object(isolate);
    Buffer *self = binder::Class<Buffer>::unwrap_object(isolate, buf);
    v8::Local<v8::Uint8Array> arr = newBuffer(size);
    self->array_.Reset(isolate, arr);
    self->backing_store_ = arr->Buffer()->GetBackingStore();
    return buf;
}

v8::Local<v8::Promise> Buffer::MakeFromFile(const std::string& path)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    EventLoop *loop = EventLoop::Instance();

    auto resolver = v8::Promise::Resolver::New(context).ToLocalChecked();
    auto global = std::make_shared<v8::Global<v8::Promise::Resolver>>(isolate, resolver);

    using DataType = std::tuple<std::string, std::shared_ptr<Data>>;
    loop->enqueueThreadPoolTask<DataType>([path]() -> DataType {
        std::shared_ptr<Data> data = Data::MakeFromFile(path, {vfs::OpenFlags::kReadonly});
        if (!data)
        {
            // A filesystem error has occurred
            return {strerror(errno), nullptr};
        }
        std::shared_ptr<Data> linear_data = Data::MakeLinearBuffer(data);
        if (!linear_data)
        {
            return {strerror(errno), nullptr};
        }
        return {"", linear_data};

    }, [global, isolate](const DataType& data) {

        v8::HandleScope scope(isolate);
        v8::Local<v8::Promise::Resolver> resolver = global->Get(isolate);
        v8::Local<v8::Context> context = isolate->GetCurrentContext();

        // An error has occurred
        if (!std::get<0>(data).empty())
        {
            resolver->Reject(context,
                binder::to_v8(isolate, std::get<0>(data))).Check();
            return;
        }

        auto buffer = std::get<1>(data);
        auto buffer_obj = Buffer::MakeFromPtrWithoutCopy(buffer->takeBufferOwnership(),
                                                         buffer->size(),
                                                         [](void *ptr, size_t size, void *closure) {
                                                             free(ptr);
                                                         },
                                                         nullptr);

        resolver->Resolve(context, buffer_obj).Check();
    });

    return resolver->GetPromise();
}

v8::Local<v8::Object> Buffer::MakeFromPtrCopy(const void *data, size_t size)
{
    CHECK(data && size > 0);
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Object> buf = binder::Class<Buffer>::create_object(isolate);
    Buffer *self = binder::Class<Buffer>::unwrap_object(isolate, buf);
    v8::Local<v8::Uint8Array> arr = newBuffer(size);
    self->array_.Reset(isolate, arr);
    self->backing_store_ = arr->Buffer()->GetBackingStore();
    std::memcpy(self->addressU8(), data, size);
    return buf;
}

v8::Local<v8::Object> Buffer::MakeFromPtrWithoutCopy(void *data, size_t size,
                                                     v8::BackingStore::DeleterCallback deleter,
                                                     void *closure)
{
    CHECK(data && size > 0);
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Object> buf = binder::Class<Buffer>::create_object(isolate);
    Buffer *self = binder::Class<Buffer>::unwrap_object(isolate, buf);
    v8::Local<v8::Uint8Array> arr = newBuffer(data, size, deleter, closure);
    self->array_.Reset(isolate, arr);
    self->backing_store_ = arr->Buffer()->GetBackingStore();
    return buf;
}

Buffer::~Buffer()
{
    backing_store_.reset();
    array_.Reset();
}

uint8_t *Buffer::addressU8()
{
    return reinterpret_cast<uint8_t*>(backing_store_->Data());
}

size_t Buffer::length()
{
    return backing_store_->ByteLength();
}

uint8_t Buffer::byteAt(int64_t idx)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (idx < 0 || idx >= this->length())
    {
        binder::throw_(isolate, "Index out of range", v8::Exception::RangeError);
        return 0;
    }
    return reinterpret_cast<const uint8_t*>(backing_store_->Data())[idx];
}

v8::Local<v8::Value> Buffer::copy(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::Isolate *isolate = args.GetIsolate();
    size_t byteSize = this->length();
    uint64_t start = 0;
    uint64_t len = byteSize;
    if (args.Length() > 2)
        g_throw(Error, "Too many arguments");
    for (int32_t i = 0; i < args.Length(); i++)
    {
        if (!args[i]->IsNumber())
            g_throw(TypeError, "Arguments are not numbers");
    }

    if (args.Length() > 0)
    {
        start = binder::from_v8<uint64_t>(isolate, args[0]);
        if (args.Length() > 1)
            len = binder::from_v8<uint64_t>(isolate, args[1]);
    }
    if (start + len > byteSize)
        g_throw(RangeError, "Invalid length and offset");

    return Buffer::MakeFromCopy(this, static_cast<off_t>(start), static_cast<ssize_t>(len));
}

v8::Local<v8::Value> Buffer::toDataView(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::Isolate *isolate = args.GetIsolate();
    if (args.Length() > 2)
        g_throw(Error, "Too many arguments");
    int64_t offset = 0;
    auto size = static_cast<int64_t>(this->length());
    if (args.Length() > 0)
    {
        offset = binder::from_v8<decltype(offset)>(isolate, args[0]);
        if (offset < 0 || offset >= size)
            g_throw(RangeError, "Invalid offset in bytes");
        if (args.Length() > 1)
        {
            int64_t realSize = size;
            size = binder::from_v8<decltype(size)>(isolate, args[1]);
            if (size < 0 || size >= realSize)
        g_throw(RangeError, "Invalid size in bytes");
        }
    }
    v8::Local<v8::Uint8Array> array = array_.Get(isolate);
    return v8::DataView::New(array->Buffer(), offset, size);
}

v8::Local<v8::Value> Buffer::toString(uint32_t coding, int32_t length)
{
    if (coding > static_cast<uint32_t>(Encoding::kLast))
        g_throw(Error, "Invalid encoding name");
    auto enc = static_cast<Encoding>(coding);

    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    if (length >= v8::String::kMaxLength || length > this->length())
        g_throw(Error, "String is too long");

    v8::Local<v8::String> result;
    // NOLINTNEXTLINE
    switch (enc)
    {
    case Encoding::kUtf8:
    {
        auto maybe = v8::String::NewFromUtf8(isolate,
                                             reinterpret_cast<char *>(addressU8()),
                                             v8::NewStringType::kNormal,
                                             static_cast<int>(length));
        if (!maybe.ToLocal(&result))
            g_throw(Error, "Failed to decode UTF-8 string");
        break;
    }

    default:
        g_throw(Error, "Unexpected coding name");
    }

    return result;
}

void Buffer::memsetZero(uint32_t offset, uint32_t length)
{
    if (offset + length > this->length())
        g_throw(RangeError, "Invalid offset and length");
    std::memset(addressU8() + offset, 0, length);
}

v8::Local<v8::Value> Buffer::getByteArray()
{
    return array_.Get(v8::Isolate::GetCurrent());
}

GALLIUM_BINDINGS_NS_END
