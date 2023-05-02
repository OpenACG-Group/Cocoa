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

#include <vector>

#include "include/v8.h"
#include "fmt/format.h"

#include "Gallium/bindings/paragraph/Exports.h"
#include "Gallium/bindings/glamor/CkTypefaceWrap.h"
#include "Gallium/bindings/glamor/TrivialInterface.h"
#include "Gallium/binder/Convert.h"
#include "Gallium/binder/ThrowExcept.h"
#include "Gallium/binder/Class.h"
GALLIUM_BINDINGS_PARAGRAPH_NS_BEGIN

namespace {

namespace para = skia::textlayout;

template<typename T>
T get_owned_prop(v8::Isolate *isolate, v8::Local<v8::Object> obj,
                 const std::string_view& name, const std::string_view& objname)
{
    v8::HandleScope scope(isolate);
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    auto key = binder::to_v8(isolate, name);
    if (!obj->HasOwnProperty(ctx, key).ToChecked())
        g_throw(TypeError, fmt::format("Missing required property `{}` for object `{}`", name, objname));

    v8::Local<v8::Value> prop = obj->Get(ctx, key).ToLocalChecked();
    return binder::from_v8<T>(isolate, prop);
}

template<typename T>
v8::Local<T> get_owned_prop_jsobj(v8::Isolate *isolate, v8::Local<v8::Object> obj,
                                  const std::string_view& name, const std::string_view& objname)
{
    v8::EscapableHandleScope scope(isolate);
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    auto key = binder::to_v8(isolate, name);
    if (!obj->HasOwnProperty(ctx, key).ToChecked())
        g_throw(TypeError, fmt::format("Missing required property `{}` for object `{}`", name, objname));

    auto value = v8::Local<T>::Cast(obj->Get(ctx, key).ToLocalChecked());
    return scope.Escape(value);
}

#define GET_OWNED_PROP_VALUE(type, name, objname) \
    get_owned_prop_jsobj<type>(isolate, obj, name, objname)

#define GET_OWNED_PROP(type, name, objname) \
    get_owned_prop<type>(isolate, obj, name, objname)

} // namespace anonymous

para::StrutStyle ExtractStrutStyle(v8::Isolate *isolate, v8::Local<v8::Value> v)
{
    constexpr static std::string_view kIName = "StrutStyle";

    if (!v->IsObject())
        g_throw(TypeError, "Invalid `StrutStyle` object");
    auto obj = v8::Local<v8::Object>::Cast(v);

    skia::textlayout::StrutStyle res;

    v8::Local<v8::Value> font_families = GET_OWNED_PROP_VALUE(
            v8::Value, "fontFamilies", kIName);
    if (!font_families->IsArray())
    {
        g_throw(TypeError, "Property `fontFamilies` of `StrutStyle` object must be "
                           "an array of string");
    }

    v8::Local<v8::Array> arr = v8::Local<v8::Array>::Cast(font_families);
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    uint32_t length = arr->Length();
    if (length > 0)
    {
        std::vector<SkString> vec(length);
        for (uint32_t i = 0; i < arr->Length(); i++)
        {
            v8::Local<v8::Value> value = arr->Get(ctx, i).ToLocalChecked();
            vec[i].append(binder::from_v8<std::string_view>(isolate, value));
        }
        res.setFontFamilies(std::move(vec));
    }

    v8::Local<v8::Value> font_style = GET_OWNED_PROP_VALUE(v8::Value, "fontStyle", kIName);
    auto *font_style_wrap = binder::Class<glamor_wrap::CkFontStyle>::unwrap_object(isolate, font_style);
    if (!font_style_wrap)
    {
        g_throw(TypeError, "Property `fontStyle` of `StrutStyle` object must be an "
                           "instance of `glamor.CkFontStyle`");
    }
    res.setFontStyle(font_style_wrap->GetFontStyle());

    res.setFontSize(GET_OWNED_PROP(SkScalar, "fontSize", kIName));
    res.setHeight(GET_OWNED_PROP(SkScalar, "height", kIName));
    res.setLeading(GET_OWNED_PROP(SkScalar, "leading", kIName));
    res.setForceStrutHeight(GET_OWNED_PROP(bool, "forceHeight", kIName));
    res.setStrutEnabled(GET_OWNED_PROP(bool, "enabled", kIName));
    res.setHeightOverride(GET_OWNED_PROP(bool, "heightOverride", kIName));
    res.setHalfLeading(GET_OWNED_PROP(bool, "halfLeading", kIName));

    return res;
}

v8::Local<v8::Value> WrapStrutStyle(v8::Isolate *isolate, const para::StrutStyle& style)
{
    std::vector<v8::Local<v8::String>> font_families;
    for (const auto& family : style.getFontFamilies())
    {
        font_families.push_back(v8::String::NewFromUtf8(
                isolate, family.c_str()).ToLocalChecked());
    }

    std::unordered_map<std::string_view, v8::Local<v8::Value>> map{
        { "fontFamilies", binder::to_v8(isolate, font_families) },
        { "fontStyle", binder::Class<glamor_wrap::CkFontStyle>::create_object(isolate, style.getFontStyle()) },
        { "fontSize", binder::to_v8(isolate, style.getFontSize()) },
        { "height", binder::to_v8(isolate, style.getHeight()) },
        { "leading", binder::to_v8(isolate, style.getLeading()) },
        { "forceHeight", binder::to_v8(isolate, style.getForceStrutHeight()) },
        { "enabled", binder::to_v8(isolate, style.getStrutEnabled()) },
        { "heightOverride", binder::to_v8(isolate, style.getHeightOverride()) },
        { "halfLeading", binder::to_v8(isolate, style.getHalfLeading()) }
    };

    return binder::to_v8(isolate, map);
}

para::FontFeature ExtractFontFeature(v8::Isolate *isolate, v8::Local<v8::Value> v)
{
    constexpr static std::string_view kIName = "FontFeature";

    if (!v->IsObject())
        g_throw(TypeError, "Invalid `FontFeature` object");
    auto obj = v8::Local<v8::Object>::Cast(v);

    auto name = GET_OWNED_PROP(std::string_view, "name", kIName);
    auto value = GET_OWNED_PROP(int, "value", kIName);

    return {SkString(name), value};
}

para::Decoration ExtractDecoration(v8::Isolate *isolate, v8::Local<v8::Value> v)
{
    constexpr static std::string_view kIName = "Decoration";
    if (!v->IsObject())
        g_throw(TypeError, "Invalid `Decoration` object");
    auto obj = v8::Local<v8::Object>::Cast(v);

    auto type = GET_OWNED_PROP(int32_t, "type", kIName);
    auto mode = GET_OWNED_PROP(int32_t, "mode", kIName);
    if (mode < 0 || mode > static_cast<int32_t>(para::TextDecorationMode::kThrough))
        g_throw(RangeError, "Invalid enumeration value for property `mode`");

    SkColor4f color = glamor_wrap::ExtractColor4f(
            isolate, GET_OWNED_PROP_VALUE(v8::Value, "color", kIName));

    auto style = GET_OWNED_PROP(int32_t, "style", kIName);
    if (style < 0 || style > static_cast<int32_t>(para::TextDecorationStyle::kWavy))
        g_throw(RangeError, "Invalid enumeration value for property `style`");

    auto thickness_k = GET_OWNED_PROP(SkScalar, "thicknessMultiplier", kIName);

    return para::Decoration{
        static_cast<para::TextDecoration>(type),
        static_cast<para::TextDecorationMode>(mode),
        color.toSkColor(),
        static_cast<para::TextDecorationStyle>(style),
        thickness_k
    };
}

v8::Local<v8::Value> WrapDecoration(v8::Isolate *isolate, const para::Decoration& deco)
{
    std::unordered_map<std::string_view, v8::Local<v8::Value>> map{
        { "type", binder::to_v8(isolate, static_cast<int32_t>(deco.fType)) },
        { "mode", binder::to_v8(isolate, static_cast<int32_t>(deco.fMode)) },
        { "color", glamor_wrap::NewColor4f(isolate, SkColor4f::FromColor(deco.fColor)) },
        { "style", binder::to_v8(isolate, static_cast<int32_t>(deco.fStyle)) },
        { "thicknessMultiplier", binder::to_v8(isolate, deco.fThicknessMultiplier) }
    };
    return binder::to_v8(isolate, map);
}

para::PlaceholderStyle ExtractPlaceholderStyle(v8::Isolate *isolate, v8::Local<v8::Value> v)
{
    constexpr static std::string_view kIName = "PlaceholderStyle";
    if (!v->IsObject())
        g_throw(TypeError, "Invalid `PlaceholderStyle` object");
    auto obj = v8::Local<v8::Object>::Cast(v);

    auto width = GET_OWNED_PROP(SkScalar, "width", kIName);
    auto height = GET_OWNED_PROP(SkScalar, "height", kIName);
    auto alignment = GET_OWNED_PROP(int32_t, "alignment", kIName);
    if (alignment < 0 || alignment > static_cast<int32_t>(para::PlaceholderAlignment::kMiddle))
        g_throw(RangeError, "Invalid enumeration value for property `alignment`");

    auto baseline = GET_OWNED_PROP(int32_t, "baseline", kIName);
    if (baseline < 0 || alignment > static_cast<int32_t>(para::TextBaseline::kIdeographic))
        g_throw(RangeError, "Invalid enumeration value for property `baseline`");

    auto baseline_offset = GET_OWNED_PROP(SkScalar, "baselineOffset", kIName);

    return para::PlaceholderStyle{
        width,
        height,
        static_cast<para::PlaceholderAlignment>(alignment),
        static_cast<para::TextBaseline>(baseline),
        baseline_offset
    };
}

skia::textlayout::TextShadow
ExtractTextShadow(v8::Isolate *isolate, v8::Local<v8::Value> v)
{
    constexpr static std::string_view kIName = "TextShadow";
    if (!v->IsObject())
        g_throw(TypeError, "Invalid `TextShadow` object");
    auto obj = v8::Local<v8::Object>::Cast(v);

    SkColor4f color = glamor_wrap::ExtractColor4f(
            isolate, GET_OWNED_PROP_VALUE(v8::Value, "color", kIName));

    SkPoint offset = glamor_wrap::ExtractCkPoint(
            isolate, GET_OWNED_PROP_VALUE(v8::Value, "offset", kIName));

    auto sigma = GET_OWNED_PROP(double, "sigma", kIName);

    return para::TextShadow{color.toSkColor(), offset, sigma};
}

GALLIUM_BINDINGS_PARAGRAPH_NS_END

