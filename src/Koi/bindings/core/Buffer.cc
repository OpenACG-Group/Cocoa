#include <cstring>

#include "include/v8.h"

#include "Core/Errors.h"
#include "Core/Utils.h"
#include "Koi/KoiBase.h"
#include "Koi/binder/Class.h"
#include "Koi/bindings/core/Exports.h"
KOI_BINDINGS_NS_BEGIN

namespace {

enum class Encoding
{
    kLatin1,        // ASCII
    kUtf8,
    kUcs2
};

// NOLINTNEXTLINE
std::map<std::string, Encoding> encoding_names_ = {
        {"latin1", Encoding::kLatin1},
        {"ascii",  Encoding::kLatin1},
        {"utf8",   Encoding::kUtf8},
        {"ucs2",   Encoding::kUcs2}
};

size_t stringByteLength(v8::Isolate *isolate, v8::Local<v8::String> str,
                        Encoding encoding)
{
    switch (encoding)
    {
    case Encoding::kLatin1:
        return str->Length();
    case Encoding::kUtf8:
        return str->Utf8Length(isolate);
    case Encoding::kUcs2:
        return str->Length() * sizeof(uint16_t);
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

size_t encodeString(v8::Isolate *isolate, uint8_t *buf, size_t buflen,
                    v8::Local<v8::String> str, Encoding encoding, int *charsWritten)
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
    case Encoding::kLatin1:
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

    case Encoding::kUtf8:
        nbytes = str->WriteUtf8(isolate, reinterpret_cast<char*>(buf),
                                static_cast<int32_t>(buflen),
                                charsWritten, flags);
        break;

    case Encoding::kUcs2:
    {
        size_t tempChars = 0;

        nbytes = encodeStringUcs2(isolate, reinterpret_cast<char *>(buf), buflen, str, flags, &tempChars);
        *charsWritten = static_cast<int>(tempChars);
        if (utils::GetEndianness() == utils::Endian::kBig)
            utils::SwapBytes16(buf, nbytes);
        break;
    }
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

v8::Local<v8::Uint8Array> newBuffer(v8::Local<v8::String> str, Encoding encoding)
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
                                     nullptr);
    CHECK(actualSize > 0);
    return v8::Uint8Array::New(ab, 0, length);
}

} // namespace anonymous

binder::Class<Buffer> Buffer::GetClass()
{
    return binder::Class<Buffer>(v8::Isolate::GetCurrent())
        .constructor<const v8::FunctionCallbackInfo<v8::Value>&>()
        .set("length", binder::Property(&Buffer::length))
        .set("byteAt", &Buffer::byteAt)
        .set("copy", &Buffer::copy)
        .set("toDataView", &Buffer::toDataView);
}

/**
 * Prototypes:
 *      new Buffer(str: string, encoding: string)
 *      new Buffer(length: number)
 */
Buffer::Buffer(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);
    if (info.Length() <= 0 || info.Length() > 2)
        throw std::invalid_argument("Invalid number of arguments");

    v8::Local<v8::Uint8Array> array;
    if (info.Length() == 2 && info[0]->IsString() && info[1]->IsString())
    {
        std::string encoding = binder::from_v8<std::string>(isolate, info[1]);
        if (!encoding_names_.contains(encoding))
            throw std::invalid_argument("Unexpected encoding");
        array = newBuffer(info[0].As<v8::String>(), encoding_names_[encoding]);
    }
    else if (info.Length() == 1 && info[0]->IsNumber())
    {
        int64_t size = 0;
        if (!info[0].As<v8::Number>()->IntegerValue(isolate->GetCurrentContext()).To(&size))
            throw std::invalid_argument("Bad buffer size");
        if (size <= 0)
            throw std::invalid_argument("Bad buffer size");
        if (size >= v8::Uint8Array::kMaxLength)
            throw std::invalid_argument("Buffer size is too large");
        array = newBuffer(size);
    }
    else
        throw std::invalid_argument("Bad arguments");

    if (array.IsEmpty())
        throw std::invalid_argument("Memory allocation failed");
    fArray = v8::Global<v8::Uint8Array>(isolate, array);
    fBackingStore = array->Buffer()->GetBackingStore();
}

Buffer::~Buffer()
{
    fBackingStore.reset();
    fArray.Reset();
}

uint8_t *Buffer::getWriteableDataPointerByte()
{
    return reinterpret_cast<uint8_t*>(fBackingStore->Data());
}

size_t Buffer::length()
{
    return fBackingStore->ByteLength();
}

uint8_t Buffer::byteAt(int64_t idx)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (idx < 0 || idx >= this->length())
    {
        binder::throw_(isolate, "Index out of range", v8::Exception::RangeError);
        return 0;
    }
    return reinterpret_cast<const uint8_t*>(fBackingStore->Data())[idx];
}

v8::Local<v8::Value> Buffer::copy(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::Isolate *isolate = args.GetIsolate();
    size_t byteSize = this->length();
    uint64_t start = 0;
    uint64_t len = byteSize;
    JS_THROW_RET_IF(args.Length() > 2, "Too many arguments", {}, v8::Exception::Error);
    for (int32_t i = 0; i < args.Length(); i++)
        JS_THROW_RET_IF(!args[i]->IsNumber(), "Arguments are not numbers", {}, v8::Exception::TypeError);
    if (args.Length() > 0)
    {
        start = binder::from_v8<uint64_t>(isolate, args[0]);
        if (args.Length() > 1)
            len = binder::from_v8<uint64_t>(isolate, args[1]);
    }
    JS_THROW_RET_IF(start + len > byteSize, "Invalid length and offset", {}, v8::Exception::RangeError);

    Runtime *pRT = Runtime::GetBareFromIsolate(isolate);
    v8::Local<v8::Object> newBuffer;
    if (!pRT->newObjectFromSynthetic("core", "Buffer", binder::to_v8(isolate, len)).ToLocal(&newBuffer))
        JS_THROW_RET_IF(true, "Failed to construct a new buffer", {}, v8::Exception::Error);

    auto pNativeNewBuffer = binder::Class<Buffer>::unwrap_object(isolate, newBuffer);
    CHECK(pNativeNewBuffer != nullptr);

    std::memcpy(pNativeNewBuffer->getWriteableDataPointerByte(),
                this->getWriteableDataPointerByte() + start, len);
    return newBuffer;
}

v8::Local<v8::Value> Buffer::toDataView(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::Isolate *isolate = args.GetIsolate();
    JS_THROW_RET_IF(args.Length() > 2, "Too many arguments", {}, v8::Exception::Error);
    int64_t offset = 0;
    auto size = static_cast<int64_t>(this->length());
    if (args.Length() > 0)
    {
        offset = binder::from_v8<decltype(offset)>(isolate, args[0]);
        JS_THROW_RET_IF(offset < 0 || offset >= size, "Invalid offset in bytes", {}, v8::Exception::RangeError);
        if (args.Length() > 1)
        {
            int64_t realSize = size;
            size = binder::from_v8<decltype(size)>(isolate, args[1]);
            JS_THROW_RET_IF(size < 0 || size >= realSize, "Invalid size in bytes", {}, v8::Exception::RangeError);
        }
    }
    v8::Local<v8::Uint8Array> array = fArray.Get(isolate);
    return v8::DataView::New(array->Buffer(), offset, size);
}

KOI_BINDINGS_NS_END
