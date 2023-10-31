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
#include "include/core/SkFontTypes.h"
#include "include/core/SkFont.h"

#include "Gallium/bindings/glamor/CkTypefaceWrap.h"
#include "Gallium/bindings/glamor/CkFontWrap.h"
#include "Gallium/bindings/glamor/CkPathWrap.h"
#include "Gallium/bindings/glamor/CkPaintWrap.h"
#include "Gallium/bindings/glamor/TrivialInterface.h"
#include "Gallium/binder/Class.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

#define EXTRACT_PATH_CHECKED(arg, result) \
    auto *result = binder::UnwrapObject<CkPath>(isolate, arg); \
    if (!result) {                       \
        g_throw(TypeError, "Argument `" #arg "` must be an instance of `CkPath`"); \
    }

#define EXTRACT_PAINT_CHECKED(arg, result) \
    auto *result = binder::UnwrapObject<CkPaint>(isolate, arg); \
    if (!result) {                       \
        g_throw(TypeError, "Argument `" #arg "` must be an instance of `CkPaint`"); \
    }

#define EXTRACT_TF_CHECKED(arg, result) \
    auto *result = binder::UnwrapObject<CkTypeface>(isolate, arg); \
    if (!result) {                       \
        g_throw(TypeError, "Argument `" #arg "` must be an instance of `CkTypeface`"); \
    }

#define CHECK_ENUM_RANGE(v, last) \
    if (v < 0 || v > static_cast<int32_t>(last)) { \
        g_throw(RangeError, "Invalid enumeration value for arguemnt `" #v "`"); \
    }


namespace {

SkPaint *extract_maybe_paint(v8::Isolate *isolate, v8::Local<v8::Value> v, const char *argname)
{
    if (v->IsNullOrUndefined())
        return nullptr;

    auto *w = binder::UnwrapObject<CkPaint>(isolate, v);
    if (!w)
        g_throw(TypeError, fmt::format("Argument `{}` must be an instance of `CkPaint`", argname));

    return &w->GetPaint();
}

} // namespace anonymous

v8::Local<v8::Value> CkFont::Make(v8::Local<v8::Value> typeface)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    EXTRACT_TF_CHECKED(typeface, tf)
    return binder::NewObject<CkFont>(isolate, SkFont(tf->GetSkObject()));
}

v8::Local<v8::Value> CkFont::MakeFromSize(v8::Local<v8::Value> typeface, SkScalar size)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    EXTRACT_TF_CHECKED(typeface, tf)
    return binder::NewObject<CkFont>(isolate, SkFont(tf->GetSkObject(), size));
}

v8::Local<v8::Value> CkFont::MakeTransformed(v8::Local<v8::Value> typeface,
                                             SkScalar size, SkScalar scaleX, SkScalar skewX)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    EXTRACT_TF_CHECKED(typeface, tf)
    return binder::NewObject<CkFont>(isolate, SkFont(tf->GetSkObject(),
                                                                size, scaleX, skewX));
}

void CkFont::setEdging(int32_t edging)
{
    CHECK_ENUM_RANGE(edging, SkFont::Edging::kSubpixelAntiAlias)
    font_.setEdging(static_cast<SkFont::Edging>(edging));
}

void CkFont::setHinting(int32_t hinting)
{
    CHECK_ENUM_RANGE(hinting, SkFontHinting::kFull)
    font_.setHinting(static_cast<SkFontHinting>(hinting));
}

#define GET_TA_WRPTR_CHECKED(type, arg, result_ptr, result_len) \
    if (!arg->Is##type()) { \
        g_throw(TypeError, "Argument `" #arg "` must be a `" #type "`"); \
    }                                           \
    auto arg##_arr = v8::Local<v8::type>::Cast(arg);            \
    size_t result_len = arg##_arr->Length();                   \
    void *result_ptr = (uint8_t*)arg##_arr->Buffer()->Data() + arg##_arr->ByteOffset();

int32_t CkFont::countText(v8::Local<v8::Value> text, int32_t encoding)
{
    CHECK_ENUM_RANGE(encoding, SkTextEncoding::kGlyphID)
    GET_TA_WRPTR_CHECKED(Uint8Array, text, text_ptr, text_ptr_len)
    return font_.countText(text_ptr, text_ptr_len, static_cast<SkTextEncoding>(encoding));
}

SkScalar CkFont::measureText(v8::Local<v8::Value> text, int32_t encoding,
                             v8::Local<v8::Value> paint)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    CHECK_ENUM_RANGE(encoding, SkTextEncoding::kGlyphID)
    GET_TA_WRPTR_CHECKED(Uint8Array, text, text_ptr, text_ptr_len)

    return font_.measureText(text_ptr, text_ptr_len, static_cast<SkTextEncoding>(encoding),
                             nullptr, extract_maybe_paint(isolate, paint, "paint"));
}

v8::Local<v8::Value> CkFont::measureTextBounds(v8::Local<v8::Value> text, int32_t encoding,
                                               v8::Local<v8::Value> paint)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    CHECK_ENUM_RANGE(encoding, SkTextEncoding::kGlyphID)
    GET_TA_WRPTR_CHECKED(Uint8Array, text, text_ptr, text_ptr_len)

    SkRect bounds = SkRect::MakeEmpty();
    font_.measureText(text_ptr, text_ptr_len,
                      static_cast<SkTextEncoding>(encoding),
                      &bounds, extract_maybe_paint(isolate, paint, "paint"));

    return NewCkRect(isolate, bounds);
}

v8::Local<v8::Value> CkFont::getBounds(v8::Local<v8::Value> glyphs, v8::Local<v8::Value> paint)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    GET_TA_WRPTR_CHECKED(Uint16Array, glyphs, glyphs_ptr, nb_glyphs)

    std::vector<SkRect> rects(nb_glyphs);
    font_.getBounds(reinterpret_cast<const SkGlyphID*>(glyphs_ptr),
                    static_cast<int32_t>(nb_glyphs), rects.data(),
                    extract_maybe_paint(isolate, paint, "paint"));

    v8::Local<v8::Array> out = v8::Array::New(isolate, static_cast<int32_t>(nb_glyphs));
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    for (size_t i = 0; i < nb_glyphs; i++)
    {
        out->Set(ctx, i, NewCkRect(isolate, rects[i])).Check();
    }

    return out;
}

v8::Local<v8::Value> CkFont::getPos(v8::Local<v8::Value> glyphs, v8::Local<v8::Value> origin)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    GET_TA_WRPTR_CHECKED(Uint16Array, glyphs, glyphs_ptr, nb_glyphs)

    std::vector<SkPoint> outpts(nb_glyphs);
    font_.getPos(reinterpret_cast<const SkGlyphID*>(glyphs_ptr),
                 static_cast<int32_t>(nb_glyphs), outpts.data(),
                 ExtractCkPoint(isolate, origin));

    v8::Local<v8::Array> out = v8::Array::New(isolate, static_cast<int32_t>(nb_glyphs));
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    for (size_t i = 0; i < nb_glyphs; i++)
    {
        out->Set(ctx, i, NewCkPoint(isolate, outpts[i])).Check();
    }

    return out;
}

v8::Local<v8::Value> CkFont::getIntercepts(v8::Local<v8::Value> glyphs,
                                           v8::Local<v8::Value> pos,
                                           SkScalar top, SkScalar bottom,
                                           v8::Local<v8::Value> paint)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    GET_TA_WRPTR_CHECKED(Uint16Array, glyphs, glyphs_ptr, nb_glyphs)

    if (!pos->IsArray())
        g_throw(TypeError, "Argument `pos` must be an array of `CkPoint`");
    auto pos_arr = v8::Local<v8::Array>::Cast(pos);

    if (pos_arr->Length() != nb_glyphs)
        g_throw(Error, "Length of `glyphs` and `pos` are different");

    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    std::vector<SkPoint> pos_vec(nb_glyphs);
    for (size_t i = 0; i < nb_glyphs; i++)
    {
        auto v = pos_arr->Get(ctx, i).ToLocalChecked();
        pos_vec[i] = ExtractCkPoint(isolate, v);
    }

    std::vector<SkScalar> resv = font_.getIntercepts(reinterpret_cast<const SkGlyphID*>(glyphs_ptr),
                                                     static_cast<int32_t>(nb_glyphs),
                                                     pos_vec.data(), top, bottom,
                                                     extract_maybe_paint(isolate, paint, "paint"));

    auto out = v8::Float32Array::New(v8::ArrayBuffer::New(
            isolate, resv.size() * sizeof(SkScalar)), 0, resv.size());
    std::memcpy(out->Buffer()->Data(), resv.data(), resv.size() * sizeof(SkScalar));

    return out;
}

v8::Local<v8::Value> CkFont::getPath(int32_t glyph)
{
    static_assert(std::is_same<SkGlyphID, uint16_t>::value);
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (glyph < 0 || glyph > 0xFFFF)
        g_throw(RangeError, "Invalid glyph ID");

    SkPath path;
    if (!font_.getPath(static_cast<SkGlyphID>(glyph), &path))
        return v8::Null(isolate);

    return binder::NewObject<CkPath>(isolate, path);
}

GALLIUM_BINDINGS_GLAMOR_NS_END
