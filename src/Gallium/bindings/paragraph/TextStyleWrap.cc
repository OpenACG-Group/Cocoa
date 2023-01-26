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

#include "Gallium/bindings/paragraph/Exports.h"
#include "Gallium/bindings/glamor/TrivialInterface.h"
#include "Gallium/bindings/glamor/CkPaintWrap.h"
#include "Gallium/bindings/glamor/CkTypefaceWrap.h"
#include "Gallium/binder/Class.h"
GALLIUM_BINDINGS_PARAGRAPH_NS_BEGIN

v8::Local<v8::Value> TextStyleWrap::getColor()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return glamor_wrap::WrapColor4f(
            isolate, SkColor4f::FromColor(text_style_.getColor()));
}

void TextStyleWrap::setColor(v8::Local<v8::Value> v)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    text_style_.setColor(glamor_wrap::ExtractColor4f(isolate, v).toSkColor());
}

v8::Local<v8::Value> TextStyleWrap::getForeground()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (text_style_.getForegroundPaintOrID().index() != 0)
        return v8::Null(isolate);
    return binder::Class<glamor_wrap::CkPaint>::create_object(
            isolate, text_style_.getForeground());
}

void TextStyleWrap::setForeground(v8::Local<v8::Value> v)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (v->IsNull())
    {
        text_style_.clearForegroundColor();
        return;
    }

    auto *w = binder::Class<glamor_wrap::CkPaint>::unwrap_object(isolate, v);
    if (!w)
        g_throw(TypeError, "Property `foreground` only can be set a CkPaint or null");

    text_style_.setForegroundColor(w->GetPaint());
}

v8::Local<v8::Value> TextStyleWrap::getBackground()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (text_style_.getBackgroundPaintOrID().index() != 0)
        return v8::Null(isolate);
    return binder::Class<glamor_wrap::CkPaint>::create_object(
            isolate, text_style_.getBackground());
}

void TextStyleWrap::setBackground(v8::Local<v8::Value> v)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (v->IsNull())
    {
        text_style_.clearBackgroundColor();
        return;
    }

    auto *w = binder::Class<glamor_wrap::CkPaint>::unwrap_object(isolate, v);
    if (!w)
        g_throw(TypeError, "Property `background` only can be set a CkPaint or null");

    text_style_.setBackgroundColor(w->GetPaint());
}

v8::Local<v8::Value> TextStyleWrap::getDecoration()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return WrapDecoration(isolate, text_style_.getDecoration());
}

void TextStyleWrap::setDecoration(v8::Local<v8::Value> v)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto deco = ExtractDecoration(isolate, v);
    text_style_.setDecoration(deco.fType);
    text_style_.setDecorationStyle(deco.fStyle);
    text_style_.setDecorationColor(deco.fColor);
    text_style_.setDecorationMode(deco.fMode);
    text_style_.setDecorationThicknessMultiplier(deco.fThicknessMultiplier);
}

v8::Local<v8::Value> TextStyleWrap::getFontStyle()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return binder::Class<glamor_wrap::CkFontStyle>::create_object(
            isolate, text_style_.getFontStyle());
}

void TextStyleWrap::setFontStyle(v8::Local<v8::Value> v)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto *w = binder::Class<glamor_wrap::CkFontStyle>::unwrap_object(isolate, v);
    if (!w)
        g_throw(TypeError, "Property `fontStyle` only can be set a `CkFontStyle`");
    text_style_.setFontStyle(w->GetFontStyle());
}

void TextStyleWrap::addShadow(v8::Local<v8::Value> v)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    text_style_.addShadow(ExtractTextShadow(isolate, v));
}

void TextStyleWrap::resetShadows()
{
    text_style_.resetShadows();
}

void TextStyleWrap::addFontFeature(v8::Local<v8::Value> v)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto feature = ExtractFontFeature(isolate, v);
    text_style_.addFontFeature(feature.fName, feature.fValue);
}

void TextStyleWrap::resetFontFeatures()
{
    text_style_.resetFontFeatures();
}

void TextStyleWrap::setFontFamilies(v8::Local<v8::Value> v)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!v->IsArray())
        g_throw(TypeError, "Argument `fontFamilies` must be string[]");

    auto arr = v8::Local<v8::Array>::Cast(v);
    uint32_t len = arr->Length();
    if (len == 0)
        return;

    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    std::vector<SkString> vec(len);
    for (uint32_t i = 0; i < len; i++)
    {
        auto name = arr->Get(ctx, i).ToLocalChecked();
        vec[i].append(binder::from_v8<std::string_view>(isolate, name));
    }

    text_style_.setFontFamilies(std::move(vec));
}

void TextStyleWrap::setTypeface(v8::Local<v8::Value> v)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto *w = binder::Class<glamor_wrap::CkTypeface>::unwrap_object(isolate, v);
    if (!w)
        g_throw(TypeError, "Argument `tf` must be an instance of `glamor.CkTypeface`");
    text_style_.setTypeface(w->getSkiaObject());
}

v8::Local<v8::Value> TextStyleWrap::clone()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return binder::Class<TextStyleWrap>::create_object(isolate, text_style_);
}

v8::Local<v8::Value> TextStyleWrap::cloneForPlaceholder()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return binder::Class<TextStyleWrap>::create_object(
            isolate, text_style_.cloneForPlaceholder());
}

GALLIUM_BINDINGS_PARAGRAPH_NS_END
