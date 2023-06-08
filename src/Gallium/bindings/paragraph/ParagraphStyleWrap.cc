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
#include "Gallium/binder/Class.h"
GALLIUM_BINDINGS_PARAGRAPH_NS_BEGIN

namespace para = skia::textlayout;

v8::Local<v8::Value> ParagraphStyleWrap::getStrutStyle()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return WrapStrutStyle(isolate, style_.getStrutStyle());
}

void ParagraphStyleWrap::setStrutStyle(v8::Local<v8::Value> v)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    style_.setStrutStyle(ExtractStrutStyle(isolate, v));
}

v8::Local<v8::Value> ParagraphStyleWrap::getTextStyle()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return binder::NewObject<TextStyleWrap>(isolate, style_.getTextStyle());
}

void ParagraphStyleWrap::setTextStyle(v8::Local<v8::Value> v)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto *w = binder::UnwrapObject<TextStyleWrap>(isolate, v);
    if (!w)
        g_throw(TypeError, "Property `textStyle` only can be set a `TextStyle`");
    style_.setTextStyle(w->GetTextStyle());
}

void ParagraphStyleWrap::setTextDirection(int32_t v)
{
    if (v < 0 || v > static_cast<int32_t>(para::TextDirection::kLtr))
        g_throw(RangeError, "Invalid enumeration value for property `textDirection`");
    style_.setTextDirection(static_cast<para::TextDirection>(v));
}

void ParagraphStyleWrap::setTextAlign(int32_t v)
{
    if (v < 0 || v > static_cast<int32_t>(para::TextAlign::kEnd))
        g_throw(RangeError, "Invalid enumeration value for property `textAlign`");
    style_.setTextAlign(static_cast<para::TextAlign>(v));
}

void ParagraphStyleWrap::setEllipsis(v8::Local<v8::Value> value)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!value->IsString())
        g_throw(TypeError, "Argument `value` must be a string");

    v8::String::Utf8Value jstr_utf8_value(isolate, value);
    style_.setEllipsis(SkString(*jstr_utf8_value, jstr_utf8_value.length()));
}

void ParagraphStyleWrap::setTextHeightBehavior(int32_t v)
{
    if (v < 0 || v > static_cast<int32_t>(para::TextHeightBehavior::kDisableAll))
        g_throw(TypeError, "Invalid enumeration value for property `textHeightBehavior`");
    style_.setTextHeightBehavior(static_cast<para::TextHeightBehavior>(v));
}

GALLIUM_BINDINGS_PARAGRAPH_NS_END
