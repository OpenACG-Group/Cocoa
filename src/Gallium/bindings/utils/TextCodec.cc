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
 
#include "Core/Utils.h"
#include "Gallium/bindings/utils/TextCodec.h"
GALLIUM_BINDINGS_UTILS_NS_BEGIN

namespace {

uint32_t string_byte_length(v8::Isolate *isolate, v8::Local<v8::String> str, TextCodec codec)
{
    uint32_t x = str->Length();
    switch (codec)
    {
    case TextCodec::kLatin1:
        return x;
    case TextCodec::kUTF8:
        return str->Utf8Length(isolate);
    case TextCodec::kUCS2:
        return x << 1;
    case TextCodec::kHex:
        // x/2,     if x = 2k (k∈N)
        // x/2 + 1, if x = 2k+1 (k∈N)
        return (x >> 1) + (x & 1);
    }

    MARK_UNREACHABLE();
}

uint32_t encode_string_UCS2(v8::Isolate* isolate, char *buf, size_t buflen,
                            v8::Local<v8::String> str, int flags, size_t *chars_written)
{
    auto *const dst = reinterpret_cast<uint16_t*>(buf);

    size_t max_chars = buflen / sizeof(*dst);
    if (max_chars == 0) {
        return 0;
    }

    uint16_t* const aligned_dst = cocoa::utils::AlignUp(dst, sizeof(*dst));
    int nchars;
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
    throw std::runtime_error("unexpected character in hex string");
}

uint32_t encode_string_hex(v8::Isolate *isolate, uint8_t *dst, v8::Local<v8::String> str)
{
    v8::String::Utf8Value utf8_value(isolate, str);
    if (utf8_value.length() == 0)
        throw std::runtime_error("failed to read the string's content");

    char *hex_str = *utf8_value;

    int p = utf8_value.length() & 1;
    uint8_t *ptr = dst;

    if (p)
        *ptr++ = parse_hex_byte(hex_str[0]);

    while (p < utf8_value.length())
    {
        uint8_t r0 = parse_hex_byte(hex_str[p++]);
        uint8_t r1 = parse_hex_byte(hex_str[p++]);
        *ptr++ = (r0 << 4) | r1;
    }
    return (ptr - dst);
}

uint32_t encode_string(v8::Isolate *isolate, uint8_t *buf, size_t buflen,
                       v8::Local<v8::String> str, TextCodec codec, int *chars_written)
{
    v8::HandleScope scope(isolate);
    uint32_t nbytes;
    int nchars;

    if (chars_written == nullptr)
        chars_written = &nchars;

    int flags = v8::String::HINT_MANY_WRITES_EXPECTED |
                v8::String::NO_NULL_TERMINATION |
                v8::String::REPLACE_INVALID_UTF8;
    switch (codec)
    {
    case TextCodec::kLatin1:
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
        *chars_written = static_cast<int32_t>(nbytes);
        break;

    case TextCodec::kUTF8:
        nbytes = str->WriteUtf8(isolate, reinterpret_cast<char*>(buf),
                                static_cast<int32_t>(buflen), chars_written, flags);
        break;

    case TextCodec::kUCS2:
    {
        size_t n_chars = 0;

        nbytes = encode_string_UCS2(isolate, reinterpret_cast<char *>(buf), buflen, str, flags, &n_chars);
        *chars_written = static_cast<int>(n_chars);
        if (cocoa::utils::GetEndianness() == cocoa::utils::Endian::kBig)
            cocoa::utils::SwapBytes16(buf, nbytes);
        break;
    }

    case TextCodec::kHex:
        nbytes = encode_string_hex(isolate, buf, str);
        break;
    }

    return nbytes;
}

} // namespace anonymous

ffi::Ret<uint64_t> computeTextByteSize(v8::Local<v8::String> text,
                                       const ffi::Enum<TextCodec>& codec)
{
    return string_byte_length(v8::Isolate::GetCurrent(), text, *codec);
}

ffi::RetLocal<v8::Value> encodeText(v8::Local<v8::String> text,
                                    const ffi::Enum<TextCodec>& codec)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    size_t byte_size = string_byte_length(isolate, text, *codec);
    if (byte_size == 0)
        return v8::Uint8Array::New(v8::Local<v8::ArrayBuffer>{}, 0, 0);

    auto ab = v8::ArrayBuffer::New(isolate, byte_size);
    encode_string(isolate, static_cast<uint8_t*>(ab->Data()),
                  byte_size, text, *codec, nullptr);

    return v8::Uint8Array::New(ab, 0, byte_size);
}

ffi::RetLocal<v8::Value> encodeTextInto(v8::Local<v8::String> text,
                                        const ffi::Enum<TextCodec>& codec,
                                        const ffi::Mem<uint8_t>& dst)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    size_t byte_size = string_byte_length(isolate, text, *codec);
    if (byte_size == 0 || dst.ByteSize() == 0)
    {
        return ffi::Cast<ffi::ObjectLiteralMap>::ToChecked(isolate, {
            { "readChars", v8::Uint32::New(isolate, 0) },
            { "writtenBytes", v8::Uint32::New(isolate, 0) }
        });
    }

    int read_chars;
    uint32_t written_bytes = encode_string(isolate, dst.Address(), dst.ByteSize(),
                                           text, *codec, &read_chars);
    return ffi::Cast<ffi::ObjectLiteralMap>::ToChecked(isolate, {
        { "readChars", v8::Uint32::New(isolate, read_chars) },
        { "writtenBytes", v8::Uint32::NewFromUnsigned(isolate, written_bytes) }
    });
}

ffi::RetLocal<v8::Value> decodeText(const ffi::Mem<uint8_t>& buffer,
                                    const ffi::Enum<TextCodec>& codec)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    switch (*codec)
    {
    case TextCodec::kUTF8:
        if (buffer.ByteSize() > v8::String::kMaxLength)
            return ffi::Fail(ffi::kRangeErr, "the length of buffer exceeds the limitation");
        return v8::String::NewFromUtf8(
            isolate,
            reinterpret_cast<const char *>(buffer.Address()),
            v8::NewStringType::kNormal,
            static_cast<int>(buffer.ByteSize())
        ).ToLocalChecked();

    default:
        return ffi::Fail(ffi::kErr, "codec is not supported");
    }
}

GALLIUM_BINDINGS_UTILS_NS_END
