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

#include <memory>

#include "include/core/SkTypeface.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"

#include "Core/Utils.h"
#include "Core/Journal.h"
#include "Gallium/Runtime.h"
#include "Gallium/bindings/glamor/CanvasKitTransferContext.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Gallium.bindings.glamor.CanvasKitTransferContext)

std::unique_ptr<CanvasKitTransferContext>
CanvasKitTransferContext::Create(v8::Isolate *isolate)
{
    auto context = std::make_unique<CanvasKitTransferContext>(isolate);
    return context;
}

CanvasKitTransferContext::CanvasKitTransferContext(v8::Isolate *isolate)
    : isolate_(isolate)
{
}

void CanvasKitTransferContext::SetReadBackJSFunction(v8::Local<v8::Function> function)
{
    readback_js_function_.Reset(isolate_, function);
}

void CanvasKitTransferContext::ResetReadBackJSFunction()
{
    readback_js_function_.Reset();
}

namespace {

// boost::hash_combine to combine to hash values
size_t hash_combine(size_t lhs, size_t rhs)
{
    lhs ^= rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2);
    return lhs;
}

} // namespace anonymous

uint64_t CanvasKitTransferContext::TypefaceKey::Hash() const noexcept
{
    auto *style_value_extract = reinterpret_cast<const int32_t*>(&font_style);
    size_t family_hash = std::hash<std::string>()(family_name);
    size_t style_hash = std::hash<int32_t>()(*style_value_extract);
    return hash_combine(family_hash, style_hash);
}

namespace {

// NOLINTNEXTLINE
static std::unordered_map<SkFontStyle::Slant, char> g_slant_name_map = {
        { SkFontStyle::Slant::kItalic_Slant,  'I' },
        { SkFontStyle::Slant::kOblique_Slant, 'O' },
        { SkFontStyle::Slant::kUpright_Slant, 'U' }
};

} // namespace anonymous

std::string CanvasKitTransferContext::TypefaceKey::ToString() const noexcept
{
    // Font Signature Format: @TF:<family name>:<weight>:<width>:<slant>
    // where <slant> can be: `I` for italic slant
    //                       `O` for oblique slant
    //                       `U` for upright slant

    CHECK(g_slant_name_map.count(font_style.slant()) > 0);
    return fmt::format("@TF:{}:{}:{}:{}", family_name, font_style.weight(),
                font_style.width(), g_slant_name_map[font_style.slant()]);
}

auto CanvasKitTransferContext::TypefaceKey::ParseFromSignature(const std::string_view& view)
    -> ParseResult
{
    auto sv = utils::SplitString(view, ':');
    if (sv.size() != 5)
        return {};

    if (sv[0] != "@TF")
        return {};

    std::string family_name(sv[1]);
    int weight, width;

    try {
        weight = std::stoi(std::string(sv[2]));
        width = std::stoi(std::string(sv[3]));
    } catch (const std::exception& e) {
        return {};
    }

    if (sv[4].length() != 1)
        return {};

    SkFontStyle::Slant slant;
    bool found_slant = false;
    for (const auto& slant_name_pair : g_slant_name_map)
    {
        if (slant_name_pair.second == sv[4][0])
        {
            slant = slant_name_pair.first;
            found_slant = true;
            break;
        }
    }

    if (!found_slant)
        return {};

    return TypefaceKey {
        .family_name = family_name,
        .font_style = SkFontStyle(weight, width, slant)
    };
}

auto CanvasKitTransferContext::TypefaceKey::ParseFromBinary(SkStream *stream)
    -> ParseResult
{
#define READ_CHECKED(suffix, var) if (!stream->read##suffix(var)) { return {}; }

    uint16_t magic;
    // length of family name without the terminator '\0'
    uint16_t family_name_size;
    int32_t weight;
    int32_t width;
    signed char slant_char;

    // Read and verify the magic code
    READ_CHECKED(U16, &magic);
    if (magic != kTFMagic)
        return {};

    // Read family name
    READ_CHECKED(U16, &family_name_size);
    if (family_name_size == 0)
        return {};

    auto family_name = std::make_unique<char[]>(family_name_size + 1);
    std::memset(family_name.get(), '\0', family_name_size + 1);
    if (stream->read(family_name.get(), family_name_size) != family_name_size)
        return {};

    // Read font style data
    READ_CHECKED(S32, &weight);
    READ_CHECKED(S32, &width);
    READ_CHECKED(S8, &slant_char);

    SkFontStyle::Slant slant;
    bool found_slant = false;
    for (const auto& slant_name_pair : g_slant_name_map)
    {
        if (slant_name_pair.second == slant_char)
        {
            slant = slant_name_pair.first;
            found_slant = true;
            break;
        }
    }

    if (!found_slant)
        return {};

    // Read the end tag
    READ_CHECKED(U16, &magic);
    if (magic != kEndOfTFMagic)
        return {};

    return TypefaceKey {
        .family_name = family_name.get(),
        .font_style = SkFontStyle(weight, width, slant)
    };

#undef READ_CHECKED
}

sk_sp<SkTypeface> CanvasKitTransferContext::RequestTypeface(const TypefaceKey& key)
{
    if (typeface_hash_cache_.count(key) > 0)
        return typeface_hash_cache_[key];

    if (readback_js_function_.IsEmpty())
        return nullptr;

    v8::HandleScope scope(isolate_);
    v8::Local<v8::Context> context = isolate_->GetCurrentContext();
    v8::Local<v8::Function> callback = readback_js_function_.Get(isolate_);

    v8::TryCatch script_try_catch(isolate_);

    CHECK(g_slant_name_map.count(key.font_style.slant()) > 0);

    std::string slant_string;
    slant_string.push_back(g_slant_name_map[key.font_style.slant()]);
    std::map<std::string_view, v8::Local<v8::Value>> signature_object{
        { "family", binder::to_v8(isolate_, key.family_name) },
        { "weight", binder::to_v8(isolate_, key.font_style.weight()) },
        { "width", binder::to_v8(isolate_, key.font_style.width()) },
        { "slant", binder::to_v8(isolate_, slant_string) }
    };

    v8::Local<v8::Value> argv[] = { binder::to_v8(isolate_, signature_object) };
    v8::Local<v8::Value> result = callback->Call(context,
                                                 context->Global(),
                                                 sizeof(argv) / sizeof(argv[0]),
                                                 argv)
                                            .FromMaybe<v8::Value>({});

    if (script_try_catch.HasCaught())
    {
        QLOG(LOG_ERROR, "Failed to cache Typeface [{}]:", key.ToString());
        QLOG(LOG_ERROR, "  Uncaught exception in Typeface handler callback, which will be reported sooner");
        Runtime::GetBareFromIsolate(isolate_)->ReportUncaughtExceptionInCallback(script_try_catch);
        return nullptr;
    }

    if (result.IsEmpty() || !result->IsUint8Array())
    {
        QLOG(LOG_ERROR, "Failed to cache Typeface [{}]: callback returned an invalid value", key.ToString());
        return nullptr;
    }

    // Extract datas and deserialize them as Typeface object
    auto buffer_array = v8::Local<v8::Uint8Array>::Cast(result);
    std::shared_ptr<v8::BackingStore> backing_store = buffer_array->Buffer()->GetBackingStore();

    SkMemoryStream stream(backing_store->Data(), backing_store->ByteLength());
    sk_sp<SkTypeface> typeface = SkTypeface::MakeDeserialize(&stream);

    if (!typeface)
    {
        QLOG(LOG_ERROR, "Failed to cache Typeface [{}]: invalid serialized data", key.ToString());
        return nullptr;
    }

    // TODO(sora): Drop caches that are not used for a long time

    typeface_hash_cache_[key] = typeface;
    return typeface;
}

GALLIUM_BINDINGS_GLAMOR_NS_END
