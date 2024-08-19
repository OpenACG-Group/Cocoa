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

#ifndef COCOA_GALLIUM_BINDINGS_RENDERER_FONT_H
#define COCOA_GALLIUM_BINDINGS_RENDERER_FONT_H

#include <vector>

#include "include/core/SkFont.h"

#include "Gallium/ffi/Enum.h"
#include "Gallium/ffi/JSObject.h"
#include "Gallium/ffi/Class.h"
#include "Gallium/bindings/renderer/Types.h"
#include "Gallium/bindings/renderer/Rect.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

class Typeface;
class Paint;
class Path;

//! TSDecl: @interface MeasureTextResult
//! TSDecl: @property advanceWidth: f32
//! TSDecl: @property @optional bounds: Rect
//! TSDecl: @end

//! TSDecl: @interface MeasureGlyphsResult
//! TSDecl: @property @optional widths: @array(f32)
//! TSDecl: @property @optional bounds: @array(Rect)
//! TSDecl: @end

//! TSDecl: @interface FontMetrics
//! TSDecl: @property hasUnderlineThickness: boolean
//! TSDecl: @property hasUnderlinePosition: boolean
//! TSDecl: @property hasStrikeoutThickness: boolean
//! TSDecl: @property hasStrikeoutPosition: boolean
//! TSDecl: @property hasBounds: boolean
//! TSDecl: @property top: f32
//! TSDecl: @property ascent: f32
//! TSDecl: @property descent: f32
//! TSDecl: @property bottom: f32
//! TSDecl: @property leading: f32
//! TSDecl: @property avgCharWidth: f32
//! TSDecl: @property maxCharWidth: f32
//! TSDecl: @property xMin: f32
//! TSDecl: @property xMax: f32
//! TSDecl: @property xHeight: f32
//! TSDecl: @property capHeight: f32
//! TSDecl: @property underlineThickness: f32
//! TSDecl: @property underlinePosition: f32
//! TSDecl: @property strikeoutThickness: f32
//! TSDecl: @property strikeoutPosition: f32
//! TSDecl: @end

//! TSDecl: @class Font
class Font : public ffi::JSObject
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    //! TSDecl: @constructor(typeface: Typeface, size: f32, scaleX: f32, skewX: f32)
    Font(const ffi::Class<Typeface>& typeface, float size, float scale_x, float skew_x);

    Font(SkFont font, v8::Local<v8::Object> typeface_or_empty);
    ~Font() override = default;

    g_nodiscard SkFont& GetSkFont() {
        return font_;
    }

    //! TSDecl: @method equalTo(font: Font): boolean
    ffi::Ret<bool> equalTo(const ffi::Class<Font>& font);

#define FONT_RW_PROPERTIES_MAP(V)           \
    V(isForceAutoHinting, bool)             \
    V(isEmbeddedBitmaps, bool)              \
    V(isSubpixel, bool)                     \
    V(isLinearMetrics, bool)                \
    V(isEmbolden, bool)                     \
    V(isBaselineSnap, bool)                 \
    V(edging, ffi::Enum<SkFont::Edging>)    \
    V(hinting, ffi::Enum<SkFontHinting>)    \
    V(size, float)                          \
    V(scaleX, float)                        \
    V(skewX, float)                         \
    V(typeface, v8::Local<v8::Object>)

#define FONT_DECL_RW_PROPS(name, type) \
    ffi::Ret<type> get_##name();       \
    ffi::Ret<void> set_##name(type value);

    //! TSDecl: @property isForceAutoHinting: boolean
    //! TSDecl: @property isEmbeddedBitmaps: boolean
    //! TSDecl: @property isSubpixel: boolean
    //! TSDecl: @property isLinearMetrics: boolean
    //! TSDecl: @property isEmbolden: boolean
    //! TSDecl: @property isBaselineSnap: boolean
    //! TSDecl: @property edging: FontEdging
    //! TSDecl: @property hinting: FontHinting
    //! TSDecl: @property size: f32
    //! TSDecl: @property scaleX: f32
    //! TSDecl: @property skewX: f32
    //! TSDecl: @property typeface: Typeface
    FONT_RW_PROPERTIES_MAP(FONT_DECL_RW_PROPS)

#undef FONT_DECL_RW_PROPS

    //! TSDecl: @method makeWithSize(size: f32): Font
    ffi::RetLocal<v8::Value> makeWithSize(float size);

    //! TSDecl: @method measureText(text: string, requireBounds: boolean,
    //! TSDecl:                     paint: @union(Paint, null)): MeasureTextResult
    ffi::RetLocal<v8::Value> measureText(v8::Local<v8::String> text, bool require_bounds,
                                         const ffi::Opt<ffi::Class<Paint>>& paint);

    //! TSDecl: @method measureGlyphs(glyphs: @mem(u16), requireWidths: boolean, requireBounds: boolean,
    //! TSDecl:                       paint: @union(Paint, null)): MeasureGlyphsResult
    ffi::RetLocal<v8::Value> measureGlyphs(const ffi::Mem<uint16_t>& glyphs, bool require_widths,
                                           bool require_bounds, const ffi::Opt<ffi::Class<Paint>>& paint);

    //! TSDecl: @method getPos(glyphs: @mem(u16), points: @mem(f32), originX: f32, originY: f32): void
    ffi::Ret<void> getPos(const ffi::Mem<uint16_t>& glyphs, const ffi::Mem<float>& points,
                          float origin_x, float origin_y);

    //! TSDecl: @method getXPos(glyphs: @mem(u16), xpos: @mem(f32), originX: f32): void
    ffi::Ret<void> getXPos(const ffi::Mem<uint16_t>& glyphs, const ffi::Mem<float>& xpos, float origin_x);

    //! TSDecl: @method getIntercepts(glyphs: @mem(u16), points: @mem(f32), top: f32, bottom: f32,
    //! TSDecl:                       paint: @union(Paint, null)): @array(f32)
    ffi::RetLocal<v8::Value> getIntercepts(const ffi::Mem<uint16_t>& glyphs, const ffi::Mem<float>& points,
                                           float top, float bottom, const ffi::Opt<ffi::Class<Paint>>& paint);

    //! TSDecl: @method getPath(glyph: u16, path: Path): boolean
    ffi::Ret<bool> getPath(uint16_t glyph, const ffi::Class<Path>& path);

    //! TSDecl: @property @readonly spacing: boolean
    ffi::Ret<float> getSpacing() {
        return font_.getSpacing();
    }

    //! TSDecl: @property @readonly metrics: FontMetrics
    ffi::RetLocal<v8::Value> getMetrics();

private:
    v8::Global<v8::Object> typeface_cache_;
    SkFont font_;
};
//! TSDecl: @end

GALLIUM_BINDINGS_RENDERER_NS_END
#endif //COCOA_GALLIUM_BINDINGS_RENDERER_FONT_H
