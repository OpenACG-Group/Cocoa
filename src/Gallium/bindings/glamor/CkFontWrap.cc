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

#include "Gallium/bindings/glamor/CkTypefaceWrap.h"
#include "Gallium/bindings/glamor/CkFontWrap.h"
#include "Gallium/bindings/glamor/CkPathWrap.h"
#include "Gallium/bindings/glamor/CkPaintWrap.h"
#include "Gallium/bindings/glamor/TrivialSkiaExportedTypes.h"
#include "Gallium/bindings/core/Exports.h"
#include "Gallium/binder/Class.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

#define EXTRACT_PATH_CHECKED(arg, result) \
    auto *result = binder::Class<CkPath>::unwrap_object(isolate, arg); \
    if (!result) {                       \
        g_throw(TypeError, "Argument `" #arg "` must be an instance of `CkPath`"); \
    }

#define EXTRACT_PAINT_CHECKED(arg, result) \
    auto *result = binder::Class<CkPaint>::unwrap_object(isolate, arg); \
    if (!result) {                       \
        g_throw(TypeError, "Argument `" #arg "` must be an instance of `CkPaint`"); \
    }

#define EXTRACT_TF_CHECKED(arg, result) \
    auto *result = binder::Class<CkTypeface>::unwrap_object(isolate, arg); \
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

    auto *w = binder::Class<CkPaint>::unwrap_object(isolate, v);
    if (!w)
        g_throw(TypeError, fmt::format("Argument `{}` must be an instance of `CkPaint`", argname));

    return &w->GetPaint();
}

} // namespace anonymous

v8::Local<v8::Value> CkFont::Make(v8::Local<v8::Value> typeface)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    EXTRACT_TF_CHECKED(typeface, tf)
    return binder::Class<CkFont>::create_object(isolate, SkFont(tf->getSkiaObject()));
}

v8::Local<v8::Value> CkFont::MakeFromSize(v8::Local<v8::Value> typeface, SkScalar size)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    EXTRACT_TF_CHECKED(typeface, tf)
    return binder::Class<CkFont>::create_object(isolate, SkFont(tf->getSkiaObject(), size));
}

v8::Local<v8::Value> CkFont::MakeTransformed(v8::Local<v8::Value> typeface,
                                             SkScalar size, SkScalar scaleX, SkScalar skewX)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    EXTRACT_TF_CHECKED(typeface, tf)
    return binder::Class<CkFont>::create_object(isolate, SkFont(tf->getSkiaObject(),
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

int32_t CkFont::countText(v8::Local<v8::Value> text, int32_t encoding)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    CHECK_ENUM_RANGE(encoding, SkTextEncoding::kGlyphID)
    auto *pbuf = binder::Class<Buffer>::unwrap_object(isolate, text);
    if (!pbuf)
        g_throw(TypeError, "Argument `text` must be an instance of `core.Buffer`");

    return font_.countText(pbuf->addressU8(), pbuf->length(),
                           static_cast<SkTextEncoding>(encoding));
}

SkScalar CkFont::measureText(v8::Local<v8::Value> text, int32_t encoding,
                             v8::Local<v8::Value> paint)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    CHECK_ENUM_RANGE(encoding, SkTextEncoding::kGlyphID)
    auto *pbuf = binder::Class<Buffer>::unwrap_object(isolate, text);
    if (!pbuf)
        g_throw(TypeError, "Argument `text` must be an instance of `core.Buffer`");

    return font_.measureText(pbuf->addressU8(), pbuf->length(),
                             static_cast<SkTextEncoding>(encoding),
                             nullptr, extract_maybe_paint(isolate, paint, "paint"));
}

v8::Local<v8::Value> CkFont::measureTextBounds(v8::Local<v8::Value> text, int32_t encoding,
                                               v8::Local<v8::Value> paint)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    CHECK_ENUM_RANGE(encoding, SkTextEncoding::kGlyphID)
    auto *pbuf = binder::Class<Buffer>::unwrap_object(isolate, text);
    if (!pbuf)
        g_throw(TypeError, "Argument `text` must be an instance of `core.Buffer`");

    SkRect bounds = SkRect::MakeEmpty();
    font_.measureText(pbuf->addressU8(), pbuf->length(),
                      static_cast<SkTextEncoding>(encoding),
                      &bounds, extract_maybe_paint(isolate, paint, "paint"));

    return WrapCkRect(isolate, bounds);
}

#define GET_TA_WRPTR_CHECKED(type, arg, result_ptr, result_len) \
    if (!arg->Is##type()) { \
        g_throw(TypeError, "Argument `" #arg "` must be a `" #type "`"); \
    }                                           \
    auto arg##_arr = v8::Local<v8::type>::Cast(arg);            \
    size_t result_len = arg##_arr->Length();                   \
    void *result_ptr = (uint8_t*)arg##_arr->Buffer()->Data() + arg##_arr->ByteOffset();

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
        out->Set(ctx, i, WrapCkRect(isolate, rects[i])).Check();
    }

    return out;
}

v8::Local<v8::Value> CkFont::getPos(v8::Local<v8::Value> glyphs, v8::Local<v8::Value> origin)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    GET_TA_WRPTR_CHECKED(Uint16Array, glyphs, glyphs_ptr, nb_glyphs)

    std::vector<SkPoint> outpts;
    font_.getPos(reinterpret_cast<const SkGlyphID*>(glyphs_ptr),
                 static_cast<int32_t>(nb_glyphs), outpts.data(),
                 ExtractCkPoint(isolate, origin));

    v8::Local<v8::Array> out = v8::Array::New(isolate, static_cast<int32_t>(nb_glyphs));
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    for (size_t i = 0; i < nb_glyphs; i++)
    {
        out->Set(ctx, 1, WrapCkPoint(isolate, outpts[i])).Check();
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

    return binder::Class<CkPath>::create_object(isolate, path);
}

GALLIUM_BINDINGS_GLAMOR_NS_END
