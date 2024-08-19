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

#ifndef COCOA_GALLIUM_BINDINGS_RENDERER_TYPEFACE_H
#define COCOA_GALLIUM_BINDINGS_RENDERER_TYPEFACE_H

#include "include/core/SkTypeface.h"

#include "Gallium/ffi/JSObject.h"
#include "Gallium/ffi/Class.h"
#include "Gallium/ffi/Interface.h"
#include "Gallium/ffi/ArrayBuffer.h"
#include "Gallium/ffi/Enum.h"
#include "Gallium/bindings/renderer/Types.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

//! TSDecl: @interface LocalizedFamilyName
//! TSDecl: @property name: string
//! TSDecl: @property language: string
//! TSDecl: @end

//! TSDecl: @interface VariationFontAxisInfo
//! TSDecl: @property min: f32
//! TSDecl: @property def: f32
//! TSDecl: @property max: f32
//! TSDecl: @property hidden: boolean
//! TSDecl: @end

//! TSDecl: @interface FontArguments
struct FontArguments
{
    //! TSDecl: @property @optional collectionIndex: i32
    ffi::Opt<int32_t> collection_index;

    //! TSDecl: @property @optional variationDesignPosition: @generic(Map, string, f32)
    ffi::OptLocal<v8::Map> variation_design_position;

    //! TSDecl: @property @optional paletteIndex: i32
    ffi::Opt<int32_t> palette_index;

    //! TSDecl: @property @optional paletteOverrides: @generic(Map, i32, ColorU32)
    ffi::OptLocal<v8::Map> palette_overrides;
};
//! TSDecl: @end

class FontMgr;

//! TSDecl: @class @nonconstructible Typeface
class Typeface : public ffi::JSObject
{
public:
    // `SkTypeface` is immutable and can be shared with other threads.
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kCloneable_Attr)

    explicit Typeface(sk_sp<SkTypeface> tf) : typeface_(std::move(tf)) {}
    ~Typeface() override = default;

    g_nodiscard sk_sp<SkTypeface> GetSkTypeface() const {
        return typeface_;
    }

    //! TSDecl: @method @static MakeEmpty(): Typeface
    static ffi::RetLocal<v8::Value> MakeEmpty();

    //! TSDecl: @method @static Deserialize(data: @mem(u8), lastResortMgr: @union(FontMgr, null)): Typeface
    static ffi::RetLocal<v8::Value> Deserialize(const ffi::Mem<uint8_t>& data,
                                                const ffi::Opt<ffi::Class<FontMgr>>& last_resort_mgr);

    //! TSDecl: @method getLocalizedFamilyNames(): @array(LocalizedFamilyName)
    ffi::RetLocal<v8::Value> getLocalizedFamilyNames();

    //! TSDecl: @property @readonly fontStyle: FontStyle
    ffi::RetLocal<v8::Value> getFontStyle();

    //! TSDecl: @property @readonly isFixedPitch: boolean
    ffi::Ret<bool> getIsFixedPitch() {
        return typeface_->isFixedPitch();
    }

    //! TSDecl: @property @readonly uniqueID: u32
    ffi::Ret<uint32_t> getUniqueID() {
        return typeface_->uniqueID();
    }

    //! TSDecl: @method equalTo(other: Typeface): boolean
    ffi::Ret<bool> equalTo(const ffi::Class<Typeface>& other);

    //! TSDecl: @method getVariationDesignPosition(): @generic(Map, string, f32)
    ffi::RetLocal<v8::Value> getVariationDesignPosition();

    //! TSDecl: @method getVariationDesignParameters(): @generic(Map, string, VariationFontAxisInfo)
    ffi::RetLocal<v8::Value> getVariationDesignParameters();

    //! TSDecl: @method makeClone(arguments: FontArguments): Typeface
    ffi::RetLocal<v8::Value> makeClone(const ffi::IFace<FontArguments>& arguments);

    //! TSDecl: @method serialize(behavior: TypefaceSerializeBehavior): @mem(u8)
    ffi::RetLocal<v8::Value> serialize(const ffi::Enum<SkTypeface::SerializeBehavior>& behavior);

    //! TSDecl: @method unicharsToGlyphs(uni: @mem(i32), outGlyphs: @mem(u16)): void
    ffi::Ret<void> unicharsToGlyphs(const ffi::Mem<int32_t>& uni, const ffi::Mem<uint16_t>& out_glyphs);

    //! TSDecl: @method textToGlyphs(text: string, outGlyphs: @mem(u16)): void
    ffi::Ret<void> textToGlyphs(v8::Local<v8::String> text, const ffi::Mem<uint16_t>& out_glyphs);

    //! TSDecl: @method unicharToGlyph(unichar: i32): u16
    ffi::Ret<uint16_t> unicharToGlyph(int32_t unichar);

    //! TSDecl: @property @readonly glyphsCount: i32
    ffi::Ret<int32_t> getGlyphsCount() {
        return typeface_->countGlyphs();
    }

    //! TSDecl: @property @readonly tablesCount: i32
    ffi::Ret<int32_t> getTablesCount() {
        return typeface_->countTables();
    }

    //! TSDecl: @property @readonly tableTags: @array(string)
    ffi::RetLocal<v8::Value> getTableTags();

    //! TSDecl: @method getTableSize(tag: string): u64
    ffi::Ret<size_t> getTableSize(const std::string& tag);

    //! TSDecl: @method copyTableDataTo(tag: string, dst: @mem(u8), offset: u64, length: u64): u64
    ffi::Ret<size_t> copyTableDataTo(const std::string& tag, const ffi::Mem<uint8_t>& dst,
                                     size_t offset, size_t length);

    //! TSDecl: @property @readonly unitsPerEm: i32
    ffi::Ret<int32_t> getUnitsPerEm() {
        return typeface_->getUnitsPerEm();
    }

    //! TSDecl: @method getKerningPairAdjustments(glyphs: @mem(u16), adjustments: @mem(i32)): boolean
    ffi::Ret<bool> getKerningPairAdjustments(const ffi::Mem<uint16_t>& glyphs,
                                             const ffi::Mem<int32_t>& adjustments);

    //! TSDecl: @property @readonly familyName: string
    ffi::RetLocal<v8::Value> getFamilyName();

    //! TSDecl: @property @readonly postScriptName: @union(string, null)
    ffi::RetLocal<v8::Value> getPostScriptName();

    //! TSDecl: @property @readonly bounds: Rect
    ffi::RetLocal<v8::Value> getBounds();


private:
    std::unique_ptr<ffi::JSTransferData> OnObjectClone(v8::Isolate *isolate) override;

    sk_sp<SkTypeface> typeface_;
};
//! TSDecl: @end

GALLIUM_BINDINGS_RENDERER_NS_END
#endif //COCOA_GALLIUM_BINDINGS_RENDERER_TYPEFACE_H
