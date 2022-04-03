#include <cstring>

#include "include/v8.h"

#include "Core/Errors.h"
#include "Core/Utils.h"

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

v8::Local<v8::Uint8Array> newBuffer(v8::Local<v8::String> str, Buffer::Encoding encoding,
                                    int *charsWritten)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    size_t length = stringByteLength(isolate, str, encoding);
    if (length == 0)
        throw std::invalid_argument("Empty string");
    v8::Local<v8::ArrayBuffer> ab = v8::ArrayBuffer::New(isolate, length);
    if (ab.IsEmpty())
        throw std::runtime_error("Memory allocation failed");
    size_t actualSize = encodeString(isolate,
                                     reinterpret_cast<uint8_t *>(ab->GetBackingStore()->Data()),
                                     length,
                                     str,
                                     encoding,
                                     charsWritten);
    CHECK(actualSize > 0);
    return v8::Uint8Array::New(ab, 0, length);
}

} // namespace anonymous

/**
 * Prototypes:
 *      new Buffer(str: string, encoding: string, charsWritten: RefValue)
 *      new Buffer(length: number)
 */
Buffer::Buffer(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    if (info.Length() <= 0 || info.Length() > 3)
        g_throw(Error, "Invalid number of arguments");

    v8::Local<v8::Uint8Array> array;
    if (info.Length() >= 2 && info[0]->IsString() && info[1]->IsString())
    {
        v8::Local<v8::Object> outCharsWritten;
        if (info.Length() == 3)
        {
            if (!info[2]->IsObject())
                g_throw(TypeError, "Bad arguments");
            if (!Runtime::GetBareFromIsolate(isolate)->isInstanceOfGlobalClass(info[2], "RefValue"))
                g_throw(Error, "Bad arguments: not an instance of RefValue");
            outCharsWritten = info[2]->ToObject(context).ToLocalChecked();
        }
        uint32_t encoding = binder::from_v8<uint32_t>(isolate, info[1]);
        if (encoding > static_cast<uint32_t>(Encoding::kLast))
            g_throw(Error, "Invalid encoding name");

        int chars;
        array = newBuffer(info[0].As<v8::String>(), static_cast<Encoding>(encoding), &chars);
        // TODO: no longer support this.
        if (!outCharsWritten.IsEmpty())
        {
            v8::TryCatch catcher(isolate);
            binder::InvokeMethod(isolate, outCharsWritten, "set", chars);
            if (catcher.HasCaught())
                g_throw(Error, "Failed in setting value in RefValue");
        }
    }
    else if (info.Length() == 1 && info[0]->IsNumber())
    {
        int64_t size = 0;
        if (!info[0].As<v8::Number>()->IntegerValue(isolate->GetCurrentContext()).To(&size))
            g_throw(Error, "Bad buffer size: not an integer");
        if (size <= 0 || size >= v8::Uint8Array::kMaxLength)
            g_throw(RangeError, "Bad buffer size: out of range");
        array = newBuffer(size);
    }
    else
        g_throw(Error, "Bad arguments");

    if (array.IsEmpty())
        g_throw(Error, "Memory allocation failed");

    array_ = v8::Global<v8::Uint8Array>(isolate, array);
    backing_store_ = array->Buffer()->GetBackingStore();
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

    std::memcpy(self->getWriteableDataPointerByte(),
                other->getWriteableDataPointerByte() + offset,
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

v8::Local<v8::Object> Buffer::MakeFromPtrCopy(void *data, size_t size)
{
    CHECK(data && size > 0);
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Object> buf = binder::Class<Buffer>::create_object(isolate);
    Buffer *self = binder::Class<Buffer>::unwrap_object(isolate, buf);
    v8::Local<v8::Uint8Array> arr = newBuffer(size);
    self->array_.Reset(isolate, arr);
    self->backing_store_ = arr->Buffer()->GetBackingStore();
    std::memcpy(self->getWriteableDataPointerByte(), data, size);
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

uint8_t *Buffer::getWriteableDataPointerByte()
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
    switch (enc)
    {
    case Encoding::kUtf8:
    {
        auto maybe = v8::String::NewFromUtf8(isolate,
                                             reinterpret_cast<char *>(getWriteableDataPointerByte()),
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

GALLIUM_BINDINGS_NS_END
