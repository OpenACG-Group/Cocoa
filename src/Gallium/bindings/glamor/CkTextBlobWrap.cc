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

#include "include/core/SkTextBlob.h"
#include "fmt/format.h"

#include "Gallium/binder/Class.h"
#include "Gallium/binder/ThrowExcept.h"
#include "Gallium/bindings/glamor/CkTextBlobWrap.h"
#include "Gallium/bindings/glamor/CkFontWrap.h"
#include "Gallium/bindings/glamor/CkPaintWrap.h"
#include "Gallium/bindings/core/Exports.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

#define CHECK_ENUM_RANGE(v, last) \
    if (v < 0 || v > static_cast<int32_t>(last)) { \
        g_throw(RangeError, "Invalid enumeration value for arguemnt `" #v "`"); \
    }

#define EXTRACT_FONT_CHECKED(arg, result) \
    auto *result = binder::Class<CkFont>::unwrap_object(isolate, arg); \
    if (!result) {                       \
        g_throw(TypeError, "Argument `" #arg "` must be an instance of `CkFont`"); \
    }

#define CHECK_CREATED_BLOB(v) \
    if (!v) {                 \
        g_throw(Error, "Failed to create a text blob");  \
    }

namespace {

std::tuple<void*, size_t> extract_text_buffer_pair(v8::Isolate *isolate,
                                                   v8::Local<v8::Value> b,
                                                   const char *argname)
{
    auto *w = binder::Class<Buffer>::unwrap_object(isolate, b);
    if (!w)
    {
        g_throw(TypeError, fmt::format("Argument `{}` must be"
                                       " an instance of `core.Buffer`", argname));
    }
    return {w->addressU8(), w->length()};
}

SkPaint *extract_maybe_paint(v8::Isolate *isolate, v8::Local<v8::Value> v, const char *argname)
{
    if (v->IsNullOrUndefined())
        return nullptr;

    auto *w = binder::Class<CkPaint>::unwrap_object(isolate, v);
    if (!w)
        g_throw(TypeError, fmt::format("Argument `{}` must be an instance of `CkPaint`", argname));

    return &w->GetPaint();
}

} // namespace anonymous

v8::Local<v8::Value> CkTextBlob::MakeFromText(v8::Local<v8::Value> text,
                                              v8::Local<v8::Value> font,
                                              int32_t encoding)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    CHECK_ENUM_RANGE(encoding, SkTextEncoding::kGlyphID)
    EXTRACT_FONT_CHECKED(font, ft)
    auto [ptr, byte_length] = extract_text_buffer_pair(isolate, text, "text");

    sk_sp<SkTextBlob> blob = SkTextBlob::MakeFromText(ptr, byte_length, ft->GetFont(),
                                                      static_cast<SkTextEncoding>(encoding));
    CHECK_CREATED_BLOB(blob)

    return binder::Class<CkTextBlob>::create_object(isolate, blob);
}

v8::Local<v8::Value> CkTextBlob::MakeFromPosText(v8::Local<v8::Value> text,
                                                 v8::Local<v8::Value> pos,
                                                 v8::Local<v8::Value> font,
                                                 int32_t encoding)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    CHECK_ENUM_RANGE(encoding, SkTextEncoding::kGlyphID)
    EXTRACT_FONT_CHECKED(font, ft)
    auto [ptr, byte_length] = extract_text_buffer_pair(isolate, text, "text");

    if (!pos->IsArray())
        g_throw(TypeError, "Argument `pos` must be an array of `CkPoint`");

    // TODO(sora): Length of `pos` should be strictly equal to
    //             number of character points in `text`

    auto arr = v8::Local<v8::Array>::Cast(pos);
    auto nb_points = static_cast<int32_t>(arr->Length());
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    std::vector<SkPoint> ptsvec(nb_points);
    for (int32_t i = 0; i < nb_points; i++)
    {
        auto v = arr->Get(ctx, i).ToLocalChecked();
        ptsvec[i] = ExtractCkPoint(isolate, v);
    }

    sk_sp<SkTextBlob> blob = SkTextBlob::MakeFromPosText(
            ptr, byte_length, ptsvec.data(), ft->GetFont(),
            static_cast<SkTextEncoding>(encoding));
    CHECK_CREATED_BLOB(blob)

    return binder::Class<CkTextBlob>::create_object(isolate, blob);
}

#define GET_TA_WRPTR_CHECKED(type, arg, result_ptr, result_len) \
    if (!arg->Is##type()) { \
        g_throw(TypeError, "Argument `" #arg "` must be a `" #type "`"); \
    }                                           \
    auto arg##_arr = v8::Local<v8::type>::Cast(arg);            \
    size_t result_len = arg##_arr->Length();                   \
    void *result_ptr = (uint8_t*)arg##_arr->Buffer()->Data() + arg##_arr->ByteOffset();

v8::Local<v8::Value> CkTextBlob::MakeFromPosTextH(v8::Local<v8::Value> text,
                                                  v8::Local<v8::Value> xpos,
                                                  SkScalar constY,
                                                  v8::Local<v8::Value> font,
                                                  int32_t encoding)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    CHECK_ENUM_RANGE(encoding, SkTextEncoding::kGlyphID)
    EXTRACT_FONT_CHECKED(font, ft)
    auto [ptr, byte_length] = extract_text_buffer_pair(isolate, text, "text");

    GET_TA_WRPTR_CHECKED(Float32Array, xpos, xpos_ptr, xpos_length)

    // TODO(sora): Length of `pos` should be strictly equal to
    //             number of character points in `text`

    sk_sp<SkTextBlob> blob = SkTextBlob::MakeFromPosTextH(ptr, byte_length,
                                                          reinterpret_cast<SkScalar*>(xpos_ptr),
                                                          constY,
                                                          ft->GetFont(),
                                                          static_cast<SkTextEncoding>(encoding));

    CHECK_CREATED_BLOB(blob)

    return binder::Class<CkTextBlob>::create_object(isolate, blob);
}

v8::Local<v8::Value> CkTextBlob::getBounds()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return WrapCkRect(isolate, getSkiaObject()->bounds());
}

uint32_t CkTextBlob::getUniqueID()
{
    return getSkiaObject()->uniqueID();
}

v8::Local<v8::Value> CkTextBlob::getIntercepts(SkScalar upperBound, SkScalar lowerBound,
                                               v8::Local<v8::Value> paint)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    SkPaint *p = extract_maybe_paint(isolate, paint, "paint");

    SkScalar bounds[2] = {upperBound, lowerBound};
    int32_t nb_intervals = getSkiaObject()->getIntercepts(bounds, nullptr, p);

    auto out = v8::Float32Array::New(v8::ArrayBuffer::New(isolate, nb_intervals * sizeof(SkScalar)),
                          0, nb_intervals * sizeof(SkScalar));

    getSkiaObject()->getIntercepts(
            bounds, reinterpret_cast<SkScalar*>(out->Buffer()->Data()), p);

    return out;
}

GALLIUM_BINDINGS_GLAMOR_NS_END
