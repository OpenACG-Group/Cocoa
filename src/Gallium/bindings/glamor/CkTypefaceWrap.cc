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
#include "include/core/SkData.h"

#include "Gallium/bindings/core/Exports.h"
#include "Gallium/bindings/glamor/CkTypefaceWrap.h"
#include "Gallium/binder/Convert.h"
#include "Gallium/binder/ThrowExcept.h"
#include "Gallium/binder/Class.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

#define CHECK_ENUM_RANGE(v, last) \
    if (v < 0 || v > static_cast<int32_t>(last)) { \
        g_throw(RangeError, "Invalid enumeration value for arguemnt `" #v "`"); \
    }

CkFontStyle::CkFontStyle(int32_t weight, int32_t width, int32_t slant)
{
    CHECK_ENUM_RANGE(slant, SkFontStyle::Slant::kOblique_Slant)
    font_style_ = SkFontStyle(weight, width, static_cast<SkFontStyle::Slant>(slant));
}

v8::Local<v8::Value> CkFontStyle::MakeNormal()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return binder::Class<CkFontStyle>::create_object(isolate, SkFontStyle::Normal());
}

v8::Local<v8::Value> CkFontStyle::MakeItalic()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return binder::Class<CkFontStyle>::create_object(isolate, SkFontStyle::Italic());
}

v8::Local<v8::Value> CkFontStyle::MakeBold()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return binder::Class<CkFontStyle>::create_object(isolate, SkFontStyle::Bold());
}

v8::Local<v8::Value> CkFontStyle::MakeBoldItalic()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return binder::Class<CkFontStyle>::create_object(isolate, SkFontStyle::BoldItalic());
}

v8::Local<v8::Value> CkTypeface::MakeDefault()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return binder::Class<CkTypeface>::create_object(isolate, SkTypeface::MakeDefault());
}

v8::Local<v8::Value> CkTypeface::MakeFromName(const std::string& name, v8::Local<v8::Value> style)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto *w = binder::Class<CkFontStyle>::unwrap_object(isolate, style);
    if (!w)
        g_throw(TypeError, "Argument `style` must be an instance of `CkFontStyle`");

    sk_sp<SkTypeface> tf = SkTypeface::MakeFromName(name.c_str(), w->GetFontStyle());
    if (!tf)
        g_throw(Error, "Failed to create a typeface from name");

    return binder::Class<CkTypeface>::create_object(isolate, tf);
}

v8::Local<v8::Value> CkTypeface::MakeFromFile(const std::string& file, int32_t index)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    sk_sp<SkTypeface> tf = SkTypeface::MakeFromFile(file.c_str(), index);
    if (!tf)
        g_throw(Error, fmt::format("Failed to create a typeface from file `{}`", file));
    return binder::Class<CkTypeface>::create_object(isolate, tf);
}

v8::Local<v8::Value> CkTypeface::MakeFromData(v8::Local<v8::Value> buffer, int32_t index)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto *w = binder::Class<Buffer>::unwrap_object(isolate, buffer);
    if (!w)
        g_throw(Error, "Argument `buffer` must be an instance of `core.Buffer`");

    sk_sp<SkData> data = SkData::MakeWithCopy(w->addressU8(), w->length());
    sk_sp<SkTypeface> tf = SkTypeface::MakeFromData(data, index);
    if (!tf)
        g_throw(Error, "Failed to create a typeface from provided data");

    return binder::Class<CkTypeface>::create_object(isolate, tf);
}

v8::Local<v8::Value> CkTypeface::getFontStyle()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return binder::Class<CkFontStyle>::create_object(
            isolate, GetSkObject()->fontStyle());
}

std::string CkTypeface::getFamilyName()
{
    SkString str;
    GetSkObject()->getFamilyName(&str);
    return str.c_str();
}

v8::Local<v8::Value> CkTypeface::getPostScriptName()
{
    SkString str;
    if (!GetSkObject()->getPostScriptName(&str))
        return v8::Null(v8::Isolate::GetCurrent());
    return binder::to_v8(v8::Isolate::GetCurrent(), str.c_str());
}

v8::Local<v8::Value> CkTypeface::getBounds()
{
    return NewCkRect(v8::Isolate::GetCurrent(), GetSkObject()->getBounds());
}


#define GET_TA_WRPTR_CHECKED(type, arg, result_ptr, result_len) \
    if (!arg->Is##type()) { \
        g_throw(TypeError, "Argument `" #arg "` must be a `" #type "`"); \
    }                                           \
    auto arg##_arr = v8::Local<v8::type>::Cast(arg);            \
    size_t result_len = arg##_arr->Length();                   \
    void *result_ptr = (uint8_t*)arg##_arr->Buffer()->Data() + arg##_arr->ByteOffset();

v8::Local<v8::Value> CkTypeface::getKerningPairAdjustments(v8::Local<v8::Value> glyphs)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!GetSkObject()->getKerningPairAdjustments(nullptr, 0, nullptr))
        return v8::Null(isolate);

    GET_TA_WRPTR_CHECKED(Uint16Array, glyphs, glyphs_ptr, glyphs_count)

    std::vector<int32_t> adjustments(glyphs_count - 1);
    GetSkObject()->getKerningPairAdjustments(reinterpret_cast<const SkGlyphID*>(glyphs_ptr),
                                             static_cast<int32_t>(glyphs_count),
                                             adjustments.data());

    return binder::to_v8(isolate, adjustments);
}

v8::Local<v8::Value> CkTypeface::unicharsToGlyphs(v8::Local<v8::Value> unichars)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    GET_TA_WRPTR_CHECKED(Uint32Array, unichars, unichars_ptr, unichars_len)

    auto out = v8::Uint16Array::New(v8::ArrayBuffer::New(isolate, unichars_len * 2),
                                    0, unichars_len);

    GetSkObject()->unicharsToGlyphs(reinterpret_cast<const SkUnichar*>(unichars_ptr),
                                    static_cast<int32_t>(unichars_len),
                                    reinterpret_cast<SkGlyphID*>(out->Buffer()->Data()));

    return out;
}

v8::Local<v8::Value> CkTypeface::textToGlyphs(v8::Local<v8::Value> buffer, int32_t encoding)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    CHECK_ENUM_RANGE(encoding, SkTextEncoding::kGlyphID)

    GET_TA_WRPTR_CHECKED(Uint8Array, buffer, text_ptr, text_ptr_len)

    int32_t nb_glyphs = GetSkObject()->textToGlyphs(text_ptr, text_ptr_len,
                                                    static_cast<SkTextEncoding>(encoding),
                                                    nullptr, 0);
    if (nb_glyphs == 0)
        return v8::Null(isolate);

    auto out = v8::Uint16Array::New(v8::ArrayBuffer::New(isolate, nb_glyphs * 2),
                                    0, nb_glyphs);

    GetSkObject()->textToGlyphs(text_ptr, text_ptr_len,
                                static_cast<SkTextEncoding>(encoding),
                                static_cast<SkGlyphID*>(out->Buffer()->Data()),
                                nb_glyphs);

    return out;
}

int32_t CkTypeface::unicharToGlyph(int32_t unichar)
{
    return GetSkObject()->unicharToGlyph(unichar);
}

int32_t CkTypeface::countGlyphs()
{
    return GetSkObject()->countGlyphs();
}

int32_t CkTypeface::countTables()
{
    return GetSkObject()->countTables();
}

v8::Local<v8::Value> CkTypeface::getTableTags()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    int32_t nb_tables = GetSkObject()->countTables();

    auto out = v8::Uint32Array::New(v8::ArrayBuffer::New(isolate, nb_tables * 4),
                                    0, nb_tables);

    GetSkObject()->getTableTags(reinterpret_cast<SkFontTableTag*>(out->Buffer()->Data()));

    return out;
}

uint32_t CkTypeface::getTableSize(uint32_t tag)
{
    return GetSkObject()->getTableSize(tag);
}

v8::Local<v8::Value> CkTypeface::copyTableData(uint32_t tag)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    size_t table_size = GetSkObject()->getTableSize(tag);
    if (table_size == 0)
        g_throw(Error, "Invalid table tag");

    v8::Local<v8::ArrayBuffer> array_buffer = v8::ArrayBuffer::New(isolate, table_size);
    v8::Local<v8::Uint8Array> u8array = v8::Uint8Array::New(array_buffer, 0, table_size);

    GetSkObject()->getTableData(tag, 0, table_size, array_buffer->Data());

    return u8array;
}

GALLIUM_BINDINGS_GLAMOR_NS_END
