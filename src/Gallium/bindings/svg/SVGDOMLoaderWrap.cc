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

#include "modules/svg/include/SkSVGDOM.h"
#include "include/core/SkStream.h"
#include "include/v8.h"

#include "Core/Errors.h"
#include "Gallium/bindings/svg/Exports.h"
#include "Gallium/bindings/glamor/CkFontMgrWrap.h"
#include "Gallium/bindings/resources/Exports.h"
#include "Gallium/binder/Class.h"
GALLIUM_BINDINGS_SVG_NS_BEGIN

v8::Local<v8::Object> SVGDOMLoaderWrap::ReturnThis()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Object> self =
            binder::Class<SVGDOMLoaderWrap>::find_object(isolate, this);

    CHECK(!self.IsEmpty());
    return self;
}

v8::Local<v8::Value> SVGDOMLoaderWrap::setFontManager(v8::Local<v8::Value> mgr)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    glamor_wrap::CkFontMgr *wrap =
            binder::UnwrapObject<glamor_wrap::CkFontMgr>(isolate, mgr);
    if (!wrap)
        g_throw(TypeError, "Argument `mgr` must be an instance of `CkFontMgr`");

    builder_.setFontManager(wrap->GetSkObject());

    return ReturnThis();
}

v8::Local<v8::Value> SVGDOMLoaderWrap::setResourceProvider(v8::Local<v8::Value> rp)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    resources_wrap::ResourceProviderWrap *wrap =
            binder::UnwrapObject<resources_wrap::ResourceProviderWrap>(isolate, rp);
    if (!wrap)
        g_throw(TypeError, "Argument `rp` must be an instance of `ResourceProvider`");

    builder_.setResourceProvider(wrap->Get());

    return ReturnThis();
}

v8::Local<v8::Value> SVGDOMLoaderWrap::makeFromString(v8::Local<v8::Value> str)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!str->IsString())
        g_throw(TypeError, "Argument `str` must be a string");

    v8::String::Utf8Value str_val(isolate, str);

    // As the `stream` will only be used in this scope,
    // we can create a reference of data by passing the pointer
    // directly, avoiding copying data.
    std::unique_ptr<SkMemoryStream> stream =
            SkMemoryStream::MakeDirect(*str_val, str_val.length());
    CHECK(stream);

    sk_sp<SkSVGDOM> dom = builder_.make(*stream);
    if (!dom)
        g_throw(Error, "Failed to create SVG DOM from given string");

    return binder::NewObject<SVGDOMWrap>(isolate, std::move(dom));
}

v8::Local<v8::Value> SVGDOMLoaderWrap::makeFromData(v8::Local<v8::Value> data)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!data->IsUint8Array())
        g_throw(TypeError, "Argument `data` must be a Uint8Array");

    v8::Local<v8::Uint8Array> arr = data.As<v8::Uint8Array>();
    uint8_t *ptr = reinterpret_cast<uint8_t*>(arr->Buffer()->Data())
                    + arr->ByteOffset();
    CHECK(ptr);

    std::unique_ptr<SkMemoryStream> stream =
            SkMemoryStream::MakeDirect(ptr, arr->ByteLength());
    CHECK(stream);

    sk_sp<SkSVGDOM> dom = builder_.make(*stream);
    if (!dom)
        g_throw(Error, "Failed to create SVG DOM from given data");

    return binder::NewObject<SVGDOMWrap>(isolate, std::move(dom));
}

v8::Local<v8::Value> SVGDOMLoaderWrap::makeFromFile(const std::string& path)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    std::unique_ptr<SkFILEStream> stream =
            SkFILEStream::Make(path.c_str());
    if (!stream || !stream->isValid())
        g_throw(Error, "Failed to open file " + path);

    sk_sp<SkSVGDOM> dom = builder_.make(*stream);
    stream->close();

    if (!dom)
        g_throw(Error, "Failed to create SVG DOM from file " + path);

    return binder::NewObject<SVGDOMWrap>(isolate, std::move(dom));
}

GALLIUM_BINDINGS_SVG_NS_END
