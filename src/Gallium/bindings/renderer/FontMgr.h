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

#ifndef COCOA_GALLIUM_BINDINGS_RENDERER_FONTMGR_H
#define COCOA_GALLIUM_BINDINGS_RENDERER_FONTMGR_H

#include "include/core/SkFontMgr.h"

#include "Gallium/ffi/JSObject.h"
#include "Gallium/ffi/Interface.h"
#include "Gallium/ffi/ArrayBuffer.h"
#include "Gallium/bindings/renderer/Types.h"
#include "Gallium/bindings/renderer/FontStyle.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

//! TSDecl: @class @nonconstructible FontStyleSet
class FontStyleSet : public ffi::JSObject
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    explicit FontStyleSet(sk_sp<SkFontStyleSet> style_set) : style_set_(std::move(style_set)) {}
    ~FontStyleSet() override = default;

    //! TSDecl: @method @static CreateEmpty(): FontStyleSet
    static ffi::RetLocal<v8::Value> CreateEmpty();

    //! TSDecl: @property @readonly count: i32
    ffi::Ret<int32_t> getCount() {
        return style_set_->count();
    }

    //! TSDecl: @method getStyle(index: i32): @tuple(FontStyle, string)
    ffi::RetLocal<v8::Value> getStyle(int32_t index);

    //! TSDecl: @method createTypeface(index: i32): Typeface
    ffi::RetLocal<v8::Value> createTypeface(int32_t index);

    //! TSDecl: @method matchStyle(pattern: FontStyle): @union(Typeface, null)
    ffi::RetLocal<v8::Value> matchStyle(const ffi::IFace<FontStyle>& pattern);

private:
    sk_sp<SkFontStyleSet> style_set_;
};
//! TSDecl: @end

//! TSDecl: @class @nonconstructible FontMgr
class FontMgr : public ffi::JSObject
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    explicit FontMgr(sk_sp<SkFontMgr> font_mgr) : font_mgr_(std::move(font_mgr)) {}
    ~FontMgr() override = default;

    g_nodiscard sk_sp<SkFontMgr> GetSkFontMgr() const {
        return font_mgr_;
    }

    //! TSDecl: @method @static GetGlobal(): FontMgr
    static ffi::RetLocal<v8::Value> GetGlobal();

    //! TSDecl: @method @static GetEmpty(): FontMgr
    static ffi::RetLocal<v8::Value> GetEmpty();

    //! TSDecl: @method countFamilies(): i32
    ffi::Ret<int32_t> countFamilies();

    //! TSDecl: @method getFamilyName(index: i32): string
    ffi::RetLocal<v8::Value> getFamilyName(int32_t index);

    //! TSDecl: @method createStyleSet(index: i32): FontStyleSet
    ffi::RetLocal<v8::Value> createStyleSet(int32_t index);

    //! TSDecl: @method matchFamily(familyName: @union(string, null)): FontStyleSet
    ffi::RetLocal<v8::Value> matchFamily(const ffi::Opt<std::string>& family_name);

    //! TSDecl: @method matchFamilyStyle(familyName: @union(string, null),
    //! TSDecl:                          style: FontStyle): @union(Typeface, null)
    ffi::RetLocal<v8::Value> matchFamilyStyle(const ffi::Opt<std::string>& family_name,
                                              const ffi::IFace<FontStyle>& style);

    //! TSDecl: @method matchFamilyStyleCharacter(familyName: @union(string, null),
    //! TSDecl:                                   style: FontStyle,
    //! TSDecl:                                   bcp47: @array(string),
    //! TSDecl:                                   unicodeChar: i32): @union(Typeface, null)
    ffi::RetLocal<v8::Value> matchFamilyStyleCharacter(const ffi::Opt<std::string>& family_name,
                                                       const ffi::IFace<FontStyle>& style,
                                                       const std::vector<std::string>& bcp47,
                                                       int32_t unicode_char);

    //! TSDecl: @method makeFromData(data: @mem(u8), ttcIndex: i32): Typeface
    ffi::RetLocal<v8::Value> makeFromData(const ffi::Mem<uint8_t>& data, int32_t ttc_index);

    //! TSDecl: @method makeFromFile(path: string, ttcIndex: i32): Typeface
    ffi::RetLocal<v8::Value> makeFromFile(const std::string& path, int32_t ttc_index);

private:
    sk_sp<SkFontMgr> font_mgr_;
};
//! TSDecl: @end

GALLIUM_BINDINGS_RENDERER_NS_END
#endif //COCOA_GALLIUM_BINDINGS_RENDERER_FONTMGR_H
