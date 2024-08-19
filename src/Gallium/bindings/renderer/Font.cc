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

#include "include/core/SkFontTypes.h"
#include "include/core/SkFontMetrics.h"

#include "Gallium/bindings/renderer/Typeface.h"
#include "Gallium/bindings/renderer/Font.h"
#include "Gallium/bindings/renderer/Paint.h"
#include "Gallium/bindings/renderer/Path.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

Font::Font(const ffi::Class<Typeface>& typeface, float size, float scale_x, float skew_x)
    : font_(typeface->GetSkTypeface(), size, scale_x, skew_x)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    typeface_cache_.Reset(isolate, typeface->GetThisHandle(isolate));
}

Font::Font(SkFont font, v8::Local<v8::Object> typeface_or_empty)
    : font_(std::move(font))
{
    if (!typeface_or_empty.IsEmpty())
        typeface_cache_.Reset(v8::Isolate::GetCurrent(), typeface_or_empty);
}

ffi::Ret<bool> Font::get_isForceAutoHinting()
{
    return font_.isForceAutoHinting();
}

ffi::Ret<void> Font::set_isForceAutoHinting(bool value)
{
    font_.setForceAutoHinting(value);
    return {};
}

ffi::Ret<bool> Font::get_isEmbeddedBitmaps()
{
    return font_.isEmbeddedBitmaps();
}

ffi::Ret<void> Font::set_isEmbeddedBitmaps(bool value)
{
    font_.setEmbeddedBitmaps(value);
    return {};
}

ffi::Ret<bool> Font::get_isSubpixel()
{
    return font_.isSubpixel();
}

ffi::Ret<void> Font::set_isSubpixel(bool value)
{
    font_.setSubpixel(value);
    return {};
}

ffi::Ret<bool> Font::get_isLinearMetrics()
{
    return font_.isLinearMetrics();
}

ffi::Ret<void> Font::set_isLinearMetrics(bool value)
{
    font_.setLinearMetrics(value);
    return {};
}

ffi::Ret<bool> Font::get_isEmbolden()
{
    return font_.isEmbolden();
}

ffi::Ret<void> Font::set_isEmbolden(bool value)
{
    font_.setEmbolden(value);
    return {};
}

ffi::Ret<bool> Font::get_isBaselineSnap()
{
    return font_.isBaselineSnap();
}

ffi::Ret<void> Font::set_isBaselineSnap(bool value)
{
    font_.setBaselineSnap(value);
    return {};
}

ffi::Ret<ffi::Enum<SkFont::Edging>> Font::get_edging()
{
    return font_.getEdging();
}

ffi::Ret<void> Font::set_edging(ffi::Enum<SkFont::Edging> value)
{
    font_.setEdging(*value);
    return {};
}

ffi::Ret<ffi::Enum<SkFontHinting>> Font::get_hinting()
{
    return font_.getHinting();
}

ffi::Ret<void> Font::set_hinting(ffi::Enum<SkFontHinting> value)
{
    font_.setHinting(*value);
    return {};
}

ffi::Ret<float> Font::get_size()
{
    return font_.getSize();
}

ffi::Ret<void> Font::set_size(float value)
{
    font_.setSize(value);
    return {};
}

ffi::Ret<float> Font::get_scaleX()
{
    return font_.getScaleX();
}

ffi::Ret<void> Font::set_scaleX(float value)
{
    font_.setScaleX(value);
    return {};
}

ffi::Ret<float> Font::get_skewX()
{
    return font_.getSkewX();
}

ffi::Ret<void> Font::set_skewX(float value)
{
    font_.setSkewX(value);
    return {};
}

ffi::Ret<v8::Local<v8::Object>> Font::get_typeface()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!typeface_cache_.IsEmpty())
        return typeface_cache_.Get(isolate);

    sk_sp<SkTypeface> sk_typeface = font_.refTypeface();
    if (!sk_typeface)
        return ffi::Fail(ffi::kErr, "operate on an empty Font instance");

    auto typeface = ffi::JSObject::New<Typeface>(isolate, sk_typeface);
    typeface_cache_.Reset(isolate, typeface);
    return typeface;
}

ffi::Ret<void> Font::set_typeface(v8::Local<v8::Object> value)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    Typeface *typeface = ffi::JSObject::Unwrap<Typeface>(isolate, value);
    if (!typeface)
        return ffi::Fail(ffi::kErr, "property `typeface` only accepts an instance of Typeface");

    font_.setTypeface(typeface->GetSkTypeface());
    typeface_cache_.Reset(isolate, typeface->GetThisHandle(isolate));
    return {};
}

ffi::Ret<bool> Font::equalTo(const ffi::Class<Font>& font)
{
    return font_ == font->font_;
}

ffi::RetLocal<v8::Value> Font::makeWithSize(float size)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<Font>(isolate, font_.makeWithSize(size),
        typeface_cache_.IsEmpty() ? v8::Local<v8::Object>{}
                                  : typeface_cache_.Get(isolate));
}

ffi::RetLocal<v8::Value> Font::measureText(v8::Local<v8::String> text, bool require_bounds,
                                           const ffi::Opt<ffi::Class<Paint>>& paint)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::String::Value utf16_value(isolate, text);

    SkRect bounds = SkRect::MakeEmpty();
    auto construct_ret = [&bounds, require_bounds, isolate](float widths) {
        ffi::ObjectLiteralMap map;
        map["advanceWidth"] = v8::Number::New(isolate, widths);
        if (require_bounds)
            map["bounds"] = CreateJSRect(isolate, bounds);
        return ffi::Cast<ffi::ObjectLiteralMap>::ToChecked(isolate, map);
    };

    if (utf16_value.length() == 0)
        return construct_ret(0);

    float advance_width = font_.measureText(
            *utf16_value, utf16_value.length() * sizeof(uint16_t),
            SkTextEncoding::kUTF16, &bounds,
            paint ? &(*paint)->GetSkPaint() : nullptr);

    return construct_ret(advance_width);
}

ffi::RetLocal<v8::Value> Font::measureGlyphs(const ffi::Mem<uint16_t>& glyphs,
                                             bool require_widths, bool require_bounds,
                                             const ffi::Opt<ffi::Class<Paint>>& paint)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!require_widths && !require_bounds)
        return v8::Object::New(isolate);

    if (glyphs.Size() == 0)
    {
        ffi::ObjectLiteralMap map;
        if (require_widths)
            map["widths"] = v8::Array::New(isolate);
        if (require_bounds)
            map["bounds"] = v8::Array::New(isolate);
        return ffi::Cast<ffi::ObjectLiteralMap>::ToChecked(isolate, map);
    }

    std::vector<float> widths_vec;
    std::vector<SkRect> bounds_vec;

    if (require_widths)
        widths_vec.resize(glyphs.Size());
    if (require_bounds)
        bounds_vec.resize(glyphs.Size());

    font_.getWidthsBounds(glyphs.Address(), static_cast<int>(glyphs.Size()),
                          require_widths ? widths_vec.data() : nullptr,
                          require_bounds ? bounds_vec.data() : nullptr,
                          paint ? &(*paint)->GetSkPaint() : nullptr);

    ffi::ObjectLiteralMap map;
    if (require_widths)
        map["widths"] = ffi::Cast<std::vector<float>>::ToChecked(isolate, widths_vec);

    if (require_bounds)
    {
        auto arr = v8::Array::New(isolate, static_cast<int>(bounds_vec.size()));
        auto context = isolate->GetCurrentContext();
        for (int i = 0; i < bounds_vec.size(); i++)
            arr->Set(context, i, CreateJSRect(isolate, bounds_vec[i])).ToChecked();
        map["bounds"] = arr;
    }

    return ffi::Cast<ffi::ObjectLiteralMap>::ToChecked(isolate, map);
}

ffi::Ret<void> Font::getPos(const ffi::Mem<uint16_t>& glyphs, const ffi::Mem<float>& points,
                            float origin_x, float origin_y)
{
    if (glyphs.Size() == 0)
        return {};
    if (points.Size() < glyphs.Size() * 2)
        return ffi::Fail(ffi::kErr, "size of points buffer is not sufficient");
    font_.getPos(glyphs.Address(), static_cast<int>(glyphs.Size()),
                 reinterpret_cast<SkPoint*>(points.Address()), {origin_x, origin_y});
    return {};
}

ffi::Ret<void> Font::getXPos(const ffi::Mem<uint16_t>& glyphs, const ffi::Mem<float>& xpos,
                             float origin_x)
{
    if (glyphs.Size() == 0)
        return {};
    if (xpos.Size() < glyphs.Size())
        return ffi::Fail(ffi::kErr,  "size of xpos buffer is not sufficient");
    font_.getXPos(glyphs.Address(), static_cast<int>(glyphs.Size()), xpos.Address(), origin_x);
    return {};
}

ffi::RetLocal<v8::Value> Font::getIntercepts(const ffi::Mem<uint16_t>& glyphs, const ffi::Mem<float>& points,
                                             float top, float bottom, const ffi::Opt<ffi::Class<Paint>>& paint)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (glyphs.Size() == 0)
        return v8::Array::New(isolate);
    if (points.Size() != glyphs.Size() * 2)
        return ffi::Fail(ffi::kErr, "size of points buffer does not match the glyphs count");

    std::vector<float> vec = font_.getIntercepts(
            glyphs.Address(), static_cast<int>(glyphs.Size()),
            reinterpret_cast<SkPoint*>(points.Address()), top, bottom,
            paint ? &(*paint)->GetSkPaint() : nullptr);

    return ffi::Cast<std::vector<float>>::ToChecked(isolate, vec);
}

ffi::Ret<bool> Font::getPath(uint16_t glyph, const ffi::Class<Path>& path)
{
    return font_.getPath(glyph, &path->GetSkPath());
}

ffi::RetLocal<v8::Value> Font::getMetrics()
{
    SkFontMetrics sk_metrics{};
    font_.getMetrics(&sk_metrics);

#define F(v) v8::Number::New(isolate, v)
#define B(v) v8::Boolean::New(isolate, v)

    SkScalar dummy;

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::Cast<ffi::ObjectLiteralMap>::ToChecked(isolate, {
        { "hasUnderlineThickness", B(sk_metrics.hasUnderlineThickness(&dummy)) },
        { "hasUnderlinePosition", B(sk_metrics.hasUnderlinePosition(&dummy)) },
        { "hasStrikeoutThickness", B(sk_metrics.hasStrikeoutThickness(&dummy)) },
        { "hasStrikeoutPosition", B(sk_metrics.hasStrikeoutPosition(&dummy)) },
        { "hasBounds", B(sk_metrics.hasBounds()) },
        { "top", F(sk_metrics.fTop) },
        { "ascent", F(sk_metrics.fAscent) },
        { "descent", F(sk_metrics.fDescent) },
        { "bottom", F(sk_metrics.fBottom) },
        { "leading", F(sk_metrics.fLeading) },
        { "avgCharWidth", F(sk_metrics.fAvgCharWidth) },
        { "maxCharWidth", F(sk_metrics.fMaxCharWidth) },
        { "xMin", F(sk_metrics.fXMin) },
        { "xMax", F(sk_metrics.fXMax) },
        { "xHeight", F(sk_metrics.fXHeight) },
        { "capHeight", F(sk_metrics.fCapHeight) },
        { "underlineThickness", F(sk_metrics.fUnderlineThickness) },
        { "underlinePosition", F(sk_metrics.fUnderlinePosition) },
        { "strikeoutThickness", F(sk_metrics.fStrikeoutThickness) },
        { "strikeoutPosition", F(sk_metrics.fStrikeoutPosition) }
    });
}

GALLIUM_BINDINGS_RENDERER_NS_END
