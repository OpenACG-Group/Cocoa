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

#include "include/core/SkString.h"

// For Linux platform, libfontconfig is used.
#include "include/ports/SkFontMgr_fontconfig.h"

#include "Gallium/ffi/Context.h"
#include "Gallium/bindings/renderer/Module.h"
#include "Gallium/bindings/renderer/Typeface.h"
#include "Gallium/bindings/renderer/FontMgr.h"
#include "Gallium/bindings/renderer/BufferUtils.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

ffi::RetLocal<v8::Value> FontStyleSet::getStyle(int32_t index)
{
    if (index < 0 || index >= style_set_->count())
        return ffi::Fail(ffi::kRangeErr, "index of font style is out of range");
    SkFontStyle style;
    SkString name;
    style_set_->getStyle(index, &style, &name);

    using Tuple = std::tuple<v8::Local<v8::Value>, std::string>;
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::Cast<Tuple>::ToChecked(isolate, std::make_tuple(
        FontStyle::CreateJS(style), std::string(name.c_str())));
}

ffi::RetLocal<v8::Value> FontStyleSet::createTypeface(int32_t index)
{
    if (index < 0 || index >= style_set_->count())
        return ffi::Fail(ffi::kRangeErr, "index of font style is out of range");
    sk_sp<SkTypeface> tf = style_set_->createTypeface(index);
    if (!tf)
        return ffi::Fail(ffi::kErr, "failed to create typeface with the specified font style");

    return ffi::JSObject::New<Typeface>(v8::Isolate::GetCurrent(), tf);
}

ffi::RetLocal<v8::Value> FontStyleSet::matchStyle(const ffi::IFace<FontStyle>& pattern)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    sk_sp<SkTypeface> tf = style_set_->matchStyle(pattern->Extract());
    if (!tf)
        return v8::Null(isolate);
    return ffi::JSObject::New<Typeface>(isolate, tf);
}

ffi::RetLocal<v8::Value> FontStyleSet::CreateEmpty()
{
    return ffi::JSObject::New<FontStyleSet>(v8::Isolate::GetCurrent(), SkFontStyleSet::CreateEmpty());
}

ffi::RetLocal<v8::Value> FontMgr::GetGlobal()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto *storage = ffi::TypeContext::FromIsolate(isolate)->GetValueStorage();

    v8::Local<v8::Value> instance = storage->Load(FFI_GVSTORE_USE_ID(instance_fontmgr_global));
    if (instance.IsEmpty())
    {
        sk_sp<SkFontMgr> mgr = SkFontMgr_New_FontConfig(nullptr);
        if (!mgr)
            return ffi::Fail(ffi::kErr, "failed to create the global instance of FontMgr (libfontconfig)");
        instance = ffi::JSObject::New<FontMgr>(isolate, mgr);
        // Store the newly-created instance into the global storage so that
        // we don't need to recreate again in the next time. The instance will
        // be directly loaded from the global value storage.
        storage->Store(FFI_GVSTORE_USE_ID(instance_fontmgr_global), instance);
    }

    return instance;
}

ffi::RetLocal<v8::Value> FontMgr::GetEmpty()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto *storage = ffi::TypeContext::FromIsolate(isolate)->GetValueStorage();

    v8::Local<v8::Value> instance = storage->Load(FFI_GVSTORE_USE_ID(instance_fontmgr_empty));
    if (instance.IsEmpty())
    {
        instance = ffi::JSObject::New<FontMgr>(isolate, SkFontMgr::RefEmpty());
        storage->Store(FFI_GVSTORE_USE_ID(instance_fontmgr_empty), instance);
    }

    return instance;
}

ffi::Ret<int32_t> FontMgr::countFamilies()
{
    return font_mgr_->countFamilies();
}

ffi::RetLocal<v8::Value> FontMgr::getFamilyName(int32_t index)
{
    if (index < 0 || index >= font_mgr_->countFamilies())
        return ffi::Fail(ffi::kRangeErr, "index of font family is not of range");
    SkString name;
    font_mgr_->getFamilyName(index, &name);
    return v8::String::NewFromUtf8(v8::Isolate::GetCurrent(), name.c_str()).ToLocalChecked();
}

ffi::RetLocal<v8::Value> FontMgr::createStyleSet(int32_t index)
{
    if (index < 0 || index >= font_mgr_->countFamilies())
        return ffi::Fail(ffi::kRangeErr, "index of font family is out of range");
    sk_sp<SkFontStyleSet> style_set = font_mgr_->createStyleSet(index);
    if (!style_set)
        return ffi::Fail(ffi::kErr, "failed to create font style set from the specified font family");
    return ffi::JSObject::New<FontStyleSet>(v8::Isolate::GetCurrent(), style_set);
}

ffi::RetLocal<v8::Value> FontMgr::matchFamily(const ffi::Opt<std::string>& family_name)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    sk_sp<SkFontStyleSet> style_set = font_mgr_->matchFamily(
            family_name ? family_name->c_str() : nullptr);
    // Skia promises `matchFamily()` never returns NULL.
    CHECK(style_set);
    return ffi::JSObject::New<FontStyleSet>(isolate, style_set);
}

ffi::RetLocal<v8::Value> FontMgr::matchFamilyStyle(const ffi::Opt<std::string>& family_name,
                                                   const ffi::IFace<FontStyle>& style)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    sk_sp<SkTypeface> tf = font_mgr_->matchFamilyStyle(
            family_name ? family_name->c_str() : nullptr, style->Extract());
    if (!tf)
        return v8::Null(isolate);
    return ffi::JSObject::New<Typeface>(isolate, tf);
}

ffi::RetLocal<v8::Value>
FontMgr::matchFamilyStyleCharacter(const ffi::Opt<std::string>& family_name,
                                   const ffi::IFace<FontStyle>& style,
                                   const std::vector<std::string>& bcp47,
                                   int32_t unicode_char)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    sk_sp<SkTypeface> tf;
    const char *family_style_p = family_name ? family_name->c_str() : nullptr;
    if (bcp47.empty())
    {
        tf = font_mgr_->matchFamilyStyleCharacter(
                family_style_p, style->Extract(), nullptr, 0, unicode_char);
    }
    else
    {
        if (bcp47.size() > 4096)
            return ffi::Fail(ffi::kRangeErr, "too many bcp47 codes");

        std::vector<const char*> bcp47_codes(bcp47.size());
        for (uint32_t i = 0; i < bcp47.size(); i++)
            bcp47_codes[i] = bcp47[i].c_str();

        tf = font_mgr_->matchFamilyStyleCharacter(
                family_style_p, style->Extract(), bcp47_codes.data(),
                static_cast<int>(bcp47.size()), unicode_char
        );
    }

    if (!tf)
        return v8::Null(isolate);

    return ffi::JSObject::New<Typeface>(isolate, tf);
}

ffi::RetLocal<v8::Value> FontMgr::makeFromData(const ffi::Mem<uint8_t>& data, int32_t ttc_index)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    sk_sp<SkData> copied_data = WrapArrayBufferToSkData(isolate, data.TypedArray(), kNone_MemoryFlag);
    sk_sp<SkTypeface> tf = font_mgr_->makeFromData(copied_data, ttc_index);
    if (!tf)
        return ffi::Fail(ffi::kErr, "failed to make typeface from the specified data");
    return ffi::JSObject::New<Typeface>(isolate, tf);
}

ffi::RetLocal<v8::Value> FontMgr::makeFromFile(const std::string &path, int32_t ttc_index)
{
    sk_sp<SkTypeface> tf = font_mgr_->makeFromFile(path.c_str(), ttc_index);
    if (!tf)
        return ffi::Fail(ffi::kErr, fmt::format("failed to make typeface from file '{}'", path));
    return ffi::JSObject::New<Typeface>(v8::Isolate::GetCurrent(), tf);
}

GALLIUM_BINDINGS_RENDERER_NS_END
