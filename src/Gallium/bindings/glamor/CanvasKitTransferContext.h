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

#ifndef COCOA_GALLIUM_BINDINGS_GLAMOR_CANVASKITTRANSFERCONTEXT_H
#define COCOA_GALLIUM_BINDINGS_GLAMOR_CANVASKITTRANSFERCONTEXT_H

#include <unordered_map>

#include "include/core/SkFontStyle.h"

#include "Gallium/bindings/glamor/Exports.h"

class SkTypeface;

GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

class CanvasKitTransferContext
{
public:
    static constexpr uint16_t kTFMagic = 0x22fa;
    static constexpr uint16_t kEndOfTFMagic = 0x22ea;

    struct TypefaceKey
    {
        static constexpr size_t kFontStyleClassSize = sizeof(SkFontStyle);

        // Supposing that the `SkFontStyle` class is standard layout, we can serialize
        // it and calculate hash value for it.
        static_assert(std::is_standard_layout<SkFontStyle>::value,
                      "SkFontStyle is not a standard layout class: unsupported version of Skia");
        static_assert(kFontStyleClassSize == sizeof(int32_t),
                      "SkFontStyle is in wrong size: unsupported version of Skia");

        std::string family_name;
        SkFontStyle font_style;

        g_nodiscard uint64_t Hash() const noexcept;
        g_nodiscard std::string ToString() const noexcept;

        bool operator==(const TypefaceKey& key) const noexcept {
            return (this->Hash() == key.Hash());
        }

        using ParseResult = std::optional<TypefaceKey>;

        // Font Signature Format: @TF:<family name>:<weight>,<width>:<slant>
        static ParseResult ParseFromSignature(const std::string_view& view);

        // Font Binary format in little endian:
        //  Header:      [U16 kTFMagic]
        //  Family name: [U16 family_name_size | bytes family_name]
        //  Font style:  [S32 weight | S32 width | S8 slant]
        //  End tag:     [U16 kEndOfTFMagic]
        static ParseResult ParseFromBinary(SkStream *stream);
    };

    struct TypefaceKeyHasher
    {
        g_nodiscard uint64_t operator()(const TypefaceKey& key) const noexcept {
            return key.Hash();
        }
    };

    explicit CanvasKitTransferContext(v8::Isolate *isolate);
    ~CanvasKitTransferContext() = default;

    static std::unique_ptr<CanvasKitTransferContext> Create(v8::Isolate *isolate);

    void SetReadBackJSFunction(v8::Local<v8::Function> function);
    void ResetReadBackJSFunction();

    // Query for a typeface in the cache map. If the cache is missed,
    // we dive into the JavaScript world and request for the typeface object.
    sk_sp<SkTypeface> RequestTypeface(const TypefaceKey& key);

private:
    v8::Isolate                         *isolate_;
    v8::Global<v8::Function>             readback_js_function_;
    std::unordered_map<TypefaceKey, sk_sp<SkTypeface>, TypefaceKeyHasher>
                                         typeface_hash_cache_;
};

GALLIUM_BINDINGS_GLAMOR_NS_END
#endif //COCOA_GALLIUM_BINDINGS_GLAMOR_CANVASKITTRANSFERCONTEXT_H
