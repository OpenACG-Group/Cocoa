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

#include "include/core/SkStream.h"
#include "include/core/SkData.h"
#include "include/core/SkFontTypes.h"

#include "Gallium/ffi/Interface.h"
#include "Gallium/bindings/renderer/FontStyle.h"
#include "Gallium/bindings/renderer/FontMgr.h"
#include "Gallium/bindings/renderer/Typeface.h"
#include "Gallium/bindings/renderer/Rect.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

ffi::RetLocal<v8::Value> Typeface::MakeEmpty()
{
    return ffi::JSObject::New<Typeface>(v8::Isolate::GetCurrent(), SkTypeface::MakeEmpty());
}

ffi::RetLocal<v8::Value> Typeface::getLocalizedFamilyNames()
{
    // The return value is exactly identical whenever this is called, but we don't
    // use cache for the following two considerations:
    //   1. JavaScript array is not an immutable object, which means if a cache is used,
    //      the user's code may modify its content through the return value.
    //      Using `Object.freeze()` can be a solution but that also makes the array
    //      nontrivial, which may not be the user's expectation.
    //   2. In most cases, there is no reason to call `getLocalizedFamilyNames()` for many
    //      times in a performance-sensitive situation. If it does need, user can cache
    //      the return value by himself simply, as the return value won't change.

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    std::vector<v8::Local<v8::Value>> result;
    v8::Local<v8::Name> keys[] = {
        v8::String::NewFromUtf8Literal(isolate, "name"),
        v8::String::NewFromUtf8Literal(isolate, "language")
    };
    SkTypeface::LocalizedStrings *iterator = typeface_->createFamilyNameIterator();
    SkTypeface::LocalizedString localized;
    while (iterator->next(&localized))
    {
        v8::Local<v8::Value> values[] = {
            v8::String::NewFromUtf8(isolate, localized.fString.c_str()).ToLocalChecked(),
            v8::String::NewFromUtf8(isolate, localized.fLanguage.c_str()).ToLocalChecked()
        };
        result.emplace_back(v8::Object::New(isolate, v8::Null(isolate), keys, values, 2));
    }
    iterator->unref();

    return v8::Array::New(isolate, result.data(), result.size());
}

namespace {

void fill_opentype_tag_str(uint32_t tag, char out[4])
{
    uint8_t *ptr = reinterpret_cast<uint8_t*>(out);
    ptr[3] = tag & 0xff;
    ptr[2] = (tag >> 8) & 0xff;
    ptr[1] = (tag >> 16) & 0xff;
    ptr[0] = (tag >> 24) & 0xff;
}

std::string stringify_opentype_tag(uint32_t tag)
{
    std::string str = "XXXX";
    fill_opentype_tag_str(tag, str.data());
    return str;
}

uint32_t encode_opentype_tag(const char in[4])
{
    return ((uint32_t)in[0] << 24) | ((uint32_t)in[1] << 16) | ((uint32_t)in[2] << 8) | (uint32_t)in[3];
}

} // namespace anonymous

ffi::RetLocal<v8::Value> Typeface::getVariationDesignPosition()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    int nb_axes = typeface_->getVariationDesignPosition(nullptr, 0);
    if (nb_axes < 0)
        return ffi::Fail(ffi::kErr, "failed to get design variation coordinates of typeface");

    if (nb_axes == 0)
        return v8::Map::New(isolate);

    std::vector<SkFontArguments::VariationPosition::Coordinate> coordinates(nb_axes);
    if (typeface_->getVariationDesignPosition(coordinates.data(), nb_axes) < 0)
        return ffi::Fail(ffi::kErr, "failed to get design variation coordinates of typeface");

    v8::Local<v8::Map> result_map = v8::Map::New(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    for (const auto& coordinate : coordinates)
    {
        std::string tag = stringify_opentype_tag(coordinate.axis);
        result_map->Set(context, v8::String::NewFromUtf8(isolate, tag.c_str()).ToLocalChecked(),
                        v8::Number::New(isolate, coordinate.value)).ToLocalChecked();
    }

    return result_map;
}

ffi::RetLocal<v8::Value> Typeface::getVariationDesignParameters()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    int nb_axes = typeface_->getVariationDesignParameters(nullptr, 0);
    if (nb_axes < 0)
        return ffi::Fail(ffi::kErr, "failed to get design variation parameters");

    if (nb_axes == 0)
        return v8::Map::New(isolate);

    std::vector<SkFontParameters::Variation::Axis> axes(nb_axes);
    if (typeface_->getVariationDesignParameters(axes.data(), nb_axes) < 0)
        return ffi::Fail(ffi::kErr, "failed to get design variation parameters");

    v8::Local<v8::Map> result_map = v8::Map::New(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    for (const auto& axis : axes)
    {
        v8::Local<v8::Name> iface_keys[] = {
            v8::String::NewFromUtf8Literal(isolate, "min"),
            v8::String::NewFromUtf8Literal(isolate, "def"),
            v8::String::NewFromUtf8Literal(isolate, "max"),
            v8::String::NewFromUtf8Literal(isolate, "hidden")
        };
        v8::Local<v8::Value> iface_values[] = {
            v8::Number::New(isolate, axis.min),
            v8::Number::New(isolate, axis.def),
            v8::Number::New(isolate, axis.max),
            v8::Boolean::New(isolate, axis.isHidden())
        };
        std::string tag = stringify_opentype_tag(axis.tag);
        result_map->Set(context, v8::String::NewFromUtf8(isolate, tag.c_str()).ToLocalChecked(),
                        v8::Object::New(isolate, v8::Null(isolate), iface_keys, iface_values, 4))
                        .ToLocalChecked();
    }

    return result_map;
}

ffi::RetLocal<v8::Value> Typeface::getFontStyle()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    SkFontStyle style = typeface_->fontStyle();
    ffi::IFace<FontStyle> iface = ffi::IFace<FontStyle>::Construct();
    iface->weight = style.weight();
    iface->width = style.width();
    iface->slant = style.slant();
    return ffi::Cast<decltype(iface)>::To(isolate, iface).Extract();
}

ffi::Ret<bool> Typeface::equalTo(const ffi::Class<Typeface>& other)
{
    return SkTypeface::Equal(other->typeface_.get(), typeface_.get());
}

ffi::RetLocal<v8::Value> Typeface::makeClone(const ffi::IFace<FontArguments>& args)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    std::vector<SkFontArguments::VariationPosition::Coordinate> sk_coordinates;
    std::vector<SkFontArguments::Palette::Override> sk_palette_overrides;
    SkFontArguments sk_args;

    if (args->collection_index)
        sk_args.setCollectionIndex(*args->collection_index);

    if (args->variation_design_position)
    {
        v8::Local<v8::Map> map = *args->variation_design_position;
        v8::Local<v8::Array> array = map->AsArray();
        CHECK(array->Length() == map->Size() * 2);

        uint32_t nb_entries = map->Size();
        for (uint32_t i = 0; i < nb_entries; i++)
        {
            v8::Local<v8::Value> tag, value;
            if (!array->Get(context, i * 2).ToLocal(&tag) || !array->Get(context, i * 2 + 1).ToLocal(&value))
                return ffi::FreePropagate();

            if (!tag->IsString() || !value->IsNumber())
                return ffi::Fail(ffi::kTypeErr, "bad data type for specifying variation axis value");
            v8::String::Utf8Value tagstr(isolate, tag);
            if (tagstr.length() != 4)
                return ffi::Fail(ffi::kErr, "axis tag specified must have 4 chars");

            sk_coordinates.push_back({
                encode_opentype_tag(*tagstr),
                static_cast<float>(value.As<v8::Number>()->Value())
            });
        }

        sk_args.setVariationDesignPosition({
            sk_coordinates.data(),
            static_cast<int>(sk_coordinates.size())
        });
    }

    if (args->palette_index && args->palette_overrides)
    {
        v8::Local<v8::Map> map = *args->palette_overrides;
        v8::Local<v8::Array> array = map->AsArray();
        CHECK(array->Length() == map->Size() * 2);

        uint32_t nb_entries = map->Size();
        for (uint32_t i = 0; i < nb_entries; i++)
        {
            v8::Local<v8::Value> index, color;
            if (!array->Get(context, i * 2).ToLocal(&index) || !array->Get(context, i * 2 + 1).ToLocal(&color))
                return ffi::FreePropagate();

            if (!index->IsNumber() || !color->IsNumber())
                return ffi::Fail(ffi::kTypeErr, "bad data type for specifying palette-overrides");

            sk_palette_overrides.push_back({
                static_cast<uint16_t>(index->Int32Value(context).ToChecked()),
                color->Uint32Value(context).ToChecked()
            });
        }

        sk_args.setPalette({
            *args->palette_index,
            sk_palette_overrides.data(),
            static_cast<int>(sk_palette_overrides.size())
        });
    }

    sk_sp<SkTypeface> tf = typeface_->makeClone(sk_args);
    if (!tf)
        return ffi::Fail(ffi::kErr, "failed to make a cloned typeface");
    return ffi::JSObject::New<Typeface>(isolate, tf);
}

ffi::RetLocal<v8::Value> Typeface::serialize(const ffi::Enum<SkTypeface::SerializeBehavior>& behavior)
{
    sk_sp<SkData> data = typeface_->serialize(*behavior);
    CHECK(data);
    void *address = data->writable_data();
    size_t size = data->size();
    return ffi::Mem<uint8_t>(v8::Isolate::GetCurrent(), data, address, size).TypedArray();
}

ffi::RetLocal<v8::Value> Typeface::Deserialize(const ffi::Mem<uint8_t>& data,
                                               const ffi::Opt<ffi::Class<FontMgr>>& last_resort_mgr)
{
    sk_sp<SkFontMgr> sk_last_resort_mgr;
    if (last_resort_mgr)
        sk_last_resort_mgr = (*last_resort_mgr)->GetSkFontMgr();

    // Never transfer this `stream` object into outer scope, as `data` must keep valid
    // during the lifetime of the stream.
    std::unique_ptr<SkStream> stream = SkMemoryStream::MakeDirect(data.Address(), data.ByteSize());
    CHECK(stream);

    sk_sp<SkTypeface> tf = SkTypeface::MakeDeserialize(stream.get(), sk_last_resort_mgr);
    if (!tf)
        return ffi::Fail(ffi::kErr, "failed to deserialize the typeface: corrupted data, or missing font data");
    return ffi::JSObject::New<Typeface>(v8::Isolate::GetCurrent(), tf);
}

ffi::Ret<void> Typeface::unicharsToGlyphs(const ffi::Mem<int32_t>& uni,
                                          const ffi::Mem<uint16_t>& out_glyphs)
{
    if (out_glyphs.Size() < uni.Size())
        return ffi::Fail(ffi::kErr, "size of provided glyph buffer is insufficient");
    typeface_->unicharsToGlyphs(uni.Address(), static_cast<int>(uni.Size()), out_glyphs.Address());
    return {};
}

ffi::Ret<void> Typeface::textToGlyphs(v8::Local<v8::String> text, const ffi::Mem<uint16_t>& out_glyphs)
{
    if (out_glyphs.Size() < text->Length())
        return ffi::Fail(ffi::kErr, "size of provided glyph buffer is insufficient");

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::String::Value utf16_value(isolate, text);
    if (utf16_value.length() == 0)
        return {};

    typeface_->textToGlyphs(
        *utf16_value, utf16_value.length() * sizeof(uint16_t),
        SkTextEncoding::kUTF16,
        out_glyphs.Address(), static_cast<int>(out_glyphs.Size())
    );
    return {};
}

ffi::Ret<uint16_t> Typeface::unicharToGlyph(int32_t unichar)
{
    return typeface_->unicharToGlyph(unichar);
}

ffi::RetLocal<v8::Value> Typeface::getTableTags()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    // fastpath: just an empty array if there are no any tables
    int nb_tags = typeface_->countTables();
    if (nb_tags == 0)
        return v8::Array::New(isolate);

    std::vector<SkFontTableTag> tags_vec(nb_tags);
    CHECK(typeface_->getTableTags(tags_vec.data()) == nb_tags);

    v8::Local<v8::Array> array = v8::Array::New(isolate, nb_tags);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    for (int i = 0; i < nb_tags; i++)
    {
        std::string strtag = stringify_opentype_tag(tags_vec[i]);
        array->Set(context, i, v8::String::NewFromUtf8(isolate, strtag.c_str()).ToLocalChecked())
            .ToChecked();
    }
    return array;
}

ffi::Ret<size_t> Typeface::getTableSize(const std::string& tag)
{
    if (tag.size() != 4)
        return ffi::Fail(ffi::kErr, "malformed four-byte table tag");
    return typeface_->getTableSize(encode_opentype_tag(tag.c_str()));
}

ffi::Ret<size_t> Typeface::copyTableDataTo(const std::string& tag, const ffi::Mem<uint8_t>& dst,
                                           size_t offset, size_t length)
{
    uint32_t u32tag = encode_opentype_tag(tag.c_str());
    size_t table_size = typeface_->getTableSize(u32tag);
    if (table_size == 0)
        return ffi::Fail(ffi::kErr, "table is not present");

    length = std::min(length, dst.ByteSize());
    return typeface_->getTableData(u32tag, offset, length, dst.Address());
}

ffi::Ret<bool> Typeface::getKerningPairAdjustments(const ffi::Mem<uint16_t>& glyphs,
                                                   const ffi::Mem<int32_t>& adjustments)
{
    if (glyphs.Size() - 1 > adjustments.Size())
        return ffi::Fail(ffi::kErr, "size of `adjustments` buffer is not sufficient");
    return typeface_->getKerningPairAdjustments(
            glyphs.Address(), static_cast<int>(glyphs.Size()), adjustments.Address());
}

ffi::RetLocal<v8::Value> Typeface::getPostScriptName()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    SkString name;
    if (!typeface_->getPostScriptName(&name))
        return v8::Null(isolate);
    return v8::String::NewFromUtf8(isolate, name.c_str()).ToLocalChecked();
}

ffi::RetLocal<v8::Value> Typeface::getFamilyName()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    SkString name;
    typeface_->getFamilyName(&name);
    return v8::String::NewFromUtf8(isolate, name.c_str()).ToLocalChecked();
}

ffi::RetLocal<v8::Value> Typeface::getBounds()
{
    return CreateJSRect(v8::Isolate::GetCurrent(), typeface_->getBounds());
}

namespace {

class TFTransfer : public ffi::JSTransferData
{
public:
    explicit TFTransfer(sk_sp<SkTypeface> tf) : typeface_(std::move(tf)) {}
    ~TFTransfer() override = default;

    ffi::RetLocal<v8::Object> Construct(v8::Isolate *isolate,
                                        v8::Local<v8::Context> ctx) override {
        // `Construct` will only be called once.
        return ffi::JSObject::New<Typeface>(isolate, std::move(typeface_));
    }

private:
    sk_sp<SkTypeface> typeface_;
};

} // namespace anonymous

std::unique_ptr<ffi::JSTransferData> Typeface::OnObjectClone(v8::Isolate *isolate)
{
    return std::make_unique<TFTransfer>(typeface_);
}

GALLIUM_BINDINGS_RENDERER_NS_END
