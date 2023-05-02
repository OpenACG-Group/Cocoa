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

#include "modules/skparagraph/src/ParagraphBuilderImpl.h"

#include "Gallium/bindings/paragraph/Exports.h"
#include "Gallium/bindings/glamor/CkFontMgrWrap.h"
#include "Gallium/binder/Convert.h"
#include "Gallium/binder/Class.h"
GALLIUM_BINDINGS_PARAGRAPH_NS_BEGIN

namespace para = skia::textlayout;

v8::Local<v8::Value> ParagraphBuilderWrap::Self()
{
    return self_.Get(isolate_);
}

v8::Local<v8::Value> ParagraphBuilderWrap::Make(v8::Local<v8::Value> paragraph_style,
                                                v8::Local<v8::Value> font_mgr)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto *parastyle = binder::Class<ParagraphStyleWrap>::unwrap_object(isolate, paragraph_style);
    if (!parastyle)
        g_throw(TypeError, "Argument `paragraphStyle` must be a `ParagraphStyle` object");

    auto *fontmgr = binder::Class<glamor_wrap::CkFontMgr>::unwrap_object(isolate, font_mgr);
    if (!fontmgr)
        g_throw(TypeError, "Argument `fontMgr` must be a `glamor.CkFontMgr` object");

    auto collection = sk_make_sp<para::FontCollection>();
    collection->setDefaultFontManager(fontmgr->GetSkObject());

    // FIXME(sora): This API is just until Skia fixes all code
    auto builder = para::ParagraphBuilderImpl::make(parastyle->GetStyle(), collection);

    auto obj = binder::Class<ParagraphBuilderWrap>::create_object(
            isolate, isolate, std::move(builder));

    auto *wrapper = binder::Class<ParagraphBuilderWrap>::unwrap_object(isolate, obj);
    CHECK(wrapper);
    wrapper->self_.Reset(isolate, obj);

    return obj;
}

v8::Local<v8::Value> ParagraphBuilderWrap::pushStyle(v8::Local<v8::Value> style)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto *w = binder::Class<TextStyleWrap>::unwrap_object(isolate, style);
    if (!w)
        g_throw(TypeError, "Argument `style` must be a `TextStyle` object");

    builder_->pushStyle(w->GetTextStyle());

    return Self();
}

v8::Local<v8::Value> ParagraphBuilderWrap::pop()
{
    builder_->pop();
    return Self();
}

v8::Local<v8::Value> ParagraphBuilderWrap::addText(v8::Local<v8::Value> text)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!text->IsString())
        g_throw(TypeError, "Invalid text");

    v8::String::Utf8Value value(isolate, text);
    builder_->addText(*value, value.length());

    return Self();
}

v8::Local<v8::Value> ParagraphBuilderWrap::addPlaceholder(v8::Local<v8::Value> style)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    builder_->addPlaceholder(ExtractPlaceholderStyle(isolate, style));
    return Self();
}

v8::Local<v8::Value> ParagraphBuilderWrap::reset()
{
    builder_->Reset();
    return Self();
}

v8::Local<v8::Value> ParagraphBuilderWrap::build()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    std::unique_ptr<para::Paragraph> result = builder_->Build();
    if (!result)
        g_throw(Error, "Failed to build paragraph");

    return binder::Class<ParagraphWrap>::create_object(isolate, std::move(result));
}

GALLIUM_BINDINGS_PARAGRAPH_NS_END
