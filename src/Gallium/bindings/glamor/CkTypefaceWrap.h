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

#ifndef COCOA_GALLIUM_BINDINGS_GLAMOR_CKTYPEFACEWRAP_H
#define COCOA_GALLIUM_BINDINGS_GLAMOR_CKTYPEFACEWRAP_H

#include "include/core/SkTypeface.h"
#include "include/v8.h"

#include "Gallium/bindings/ExportableObjectBase.h"
#include "Gallium/bindings/glamor/TrivialInterface.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

//! TSDecl: class CkFontStyle
class CkFontStyle : public ExportableObjectBase
{
public:
    //! TSDecl: function MakeNormal(): CkFontStyle
    static v8::Local<v8::Value> MakeNormal();

    //! TSDecl: function MakeBold(): CkFontStyle
    static v8::Local<v8::Value> MakeBold();

    //! TSDecl: function MakeItalic(): CkFontStyle
    static v8::Local<v8::Value> MakeItalic();

    //! TSDecl: function MakeBoldItalic(): CkFontStyle
    static v8::Local<v8::Value> MakeBoldItalic();

    //! TSDecl: constructor(weight: number, width: number, slant: Enum<FontStyleSlant>)
    CkFontStyle(int32_t weight, int32_t width, int32_t slant);

    explicit CkFontStyle(const SkFontStyle& style) : font_style_(style) {}
    ~CkFontStyle() = default;

    g_nodiscard g_inline SkFontStyle& GetFontStyle() {
        return font_style_;
    }

    //! TSDecl: readonly weight: number
    g_inline int32_t getWeight() {
        return font_style_.weight();
    }

    //! TSDecl: readonly width: number
    g_inline int32_t getWidth() {
        return font_style_.width();
    }

    //! TSDecl: readonly slant: Enum<FontStyleSlant>
    g_inline int32_t getSlant() {
        return font_style_.slant();
    }

private:
    SkFontStyle font_style_;
};

//! TSDecl: class CkTypeface
class CkTypeface : public ExportableObjectBase,
                   public SkiaObjectWrapper<SkTypeface>
{
public:
    //! TSDecl: function MakeDefault(): CkTypeface
    static v8::Local<v8::Value> MakeDefault();

    //! TSDecl: function MakeFromName(name: string, style: CkFontStyle): CkTypeface
    static v8::Local<v8::Value> MakeFromName(const std::string& name, v8::Local<v8::Value> style);

    //! TSDecl: function MakeFromFile(file: string, index: number): CkTypeface
    static v8::Local<v8::Value> MakeFromFile(const std::string& file, int32_t index);

    //! TSDecl: function MakeFromData(buffer: core.Buffer, index: number): CkTypeface
    static v8::Local<v8::Value> MakeFromData(v8::Local<v8::Value> buffer, int32_t index);

    using SkiaObjectWrapper::SkiaObjectWrapper;

    //! TSDecl: readonly fontStyle: CkFontStyle
    v8::Local<v8::Value> getFontStyle();

    //! TSDecl: readonly bold: boolean
    g_inline bool getBold() {
        return GetSkObject()->isBold();
    }

    //! TSDecl: readonly italic: boolean
    g_inline bool getItalic() {
        return GetSkObject()->isItalic();
    }

    //! TSDecl: readonly fixedPitch: boolean
    g_inline bool getFixedPitch() {
        return GetSkObject()->isFixedPitch();
    }

    //! TSDecl: readonly uniqueID: number
    g_inline SkTypefaceID getUniqueID() {
        return GetSkObject()->uniqueID();
    }

    //! TSDecl: readonly unitsPerEm: number
    g_inline int32_t getUnitsPerEm() {
        return GetSkObject()->getUnitsPerEm();
    }

    //! TSDecl: readonly familyName: string
    std::string getFamilyName();

    //! TSDecl: readonly postScriptName: string | null
    v8::Local<v8::Value> getPostScriptName();

    //! TSDecl: readonly bounds: CkRect
    v8::Local<v8::Value> getBounds();

    //! TSDecl: function getKerningPairAdjustments(glyphs: Uint16Array): Array<number> | null
    v8::Local<v8::Value> getKerningPairAdjustments(v8::Local<v8::Value> glyphs);

    //! TSDecl: function unicharsToGlyphs(unichars: Uint32Array): Uint16Array
    v8::Local<v8::Value> unicharsToGlyphs(v8::Local<v8::Value> unichars);

    //! TSDecl: function textToGlyphs(buffer: Uint8Array, encoding: Enum<TextEncoding>): Uint16Array | null
    v8::Local<v8::Value> textToGlyphs(v8::Local<v8::Value> buffer, int32_t encoding);

    //! TSDecl: function unicharToGlyph(unichar: number): number
    int32_t unicharToGlyph(int32_t unichar);

    //! TSDecl: function countGlyphs(): number
    int32_t countGlyphs();

    //! TSDecl: function countTables(): number
    int32_t countTables();

    //! TSDecl: function getTableTags(): Uint32Array
    v8::Local<v8::Value> getTableTags();

    //! TSDecl: function getTableSize(tag: number): number
    uint32_t getTableSize(uint32_t tag);

    //! TSDecl: function copyTableData(tag: number): Uint8Array
    v8::Local<v8::Value> copyTableData(uint32_t tag);
};

GALLIUM_BINDINGS_GLAMOR_NS_END
#endif //COCOA_GALLIUM_BINDINGS_GLAMOR_CKTYPEFACEWRAP_H
