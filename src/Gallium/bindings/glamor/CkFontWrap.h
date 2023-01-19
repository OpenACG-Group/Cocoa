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

#ifndef COCOA_GALLIUM_BINDINGS_GLAMOR_CKFONTWRAP_H
#define COCOA_GALLIUM_BINDINGS_GLAMOR_CKFONTWRAP_H

#include <utility>

#include "include/core/SkFont.h"
#include "include/v8.h"

#include "Core/Project.h"
#include "Gallium/bindings/glamor/Types.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

//! TSDecl: class CkFont
class CkFont
{
public:
    explicit CkFont(SkFont font) : font_(std::move(font)) {}
    ~CkFont() = default;

    //! TSDecl: function Make(typeface: CkTypeface): CkFont
    static v8::Local<v8::Value> Make(v8::Local<v8::Value> typeface);

    //! TSDecl: function MakeFromSize(typeface: CkTypeface, size: number): CkFont
    static v8::Local<v8::Value> MakeFromSize(v8::Local<v8::Value> typeface, SkScalar size);

    //! TSDecl: function MakeTransformed(typeface: CkTypeface, size: number,
    //!                                  scaleX: number, skewX: number): CkFont
    static v8::Local<v8::Value> MakeTransformed(v8::Local<v8::Value> typeface, SkScalar size,
                                                SkScalar scaleX, SkScalar skewX);

    g_nodiscard g_inline SkFont& GetFont() {
        return font_;
    }

#define DECL_GETTER_SETTER_BOOL(prop) \
    g_nodiscard g_inline bool get##prop() const { return font_.is##prop(); } \
    g_inline void set##prop(bool v) { font_.set##prop(v); }

    //! TSDecl: forceAutoHinting: boolean
    DECL_GETTER_SETTER_BOOL(ForceAutoHinting)

    //! TSDecl: embeddedBitmaps: boolean
    DECL_GETTER_SETTER_BOOL(EmbeddedBitmaps)

    //! TSDecl: subpixel: boolean
    DECL_GETTER_SETTER_BOOL(Subpixel)

    //! TSDecl: linearMetrics: boolean
    DECL_GETTER_SETTER_BOOL(LinearMetrics)

    //! TSDecl: embolden: boolean
    DECL_GETTER_SETTER_BOOL(Embolden)

    //! TSDecl: baselineSnap: boolean
    DECL_GETTER_SETTER_BOOL(BaselineSnap)

#undef DECL_GETTER_SETTER_BOOL

    //! TSDecl: edging: Enum<FontEdging>
    g_nodiscard g_inline int32_t getEdging() const {
        return static_cast<int32_t>(font_.getEdging());
    }
    void setEdging(int32_t edging);

    //! TSDecl: hinting: Enum<FontHinting>
    g_nodiscard g_inline int32_t getHinting() const {
        return static_cast<int32_t>(font_.getHinting());
    }
    void setHinting(int32_t hinting);

#define DECL_GETTER_SETTER_SCALAR(prop) \
    g_nodiscard g_inline SkScalar get##prop() const { return font_.get##prop(); } \
    g_inline void set##prop(SkScalar v) { font_.set##prop(v); }

    //! TSDecl: size: number
    DECL_GETTER_SETTER_SCALAR(Size)

    //! TSDecl: scaleX: number
    DECL_GETTER_SETTER_SCALAR(ScaleX)

    //! TSDecl: skewX: number
    DECL_GETTER_SETTER_SCALAR(SkewX)

#undef DECL_GETTER_SETTER_SCALAR

    //! TSDecl: readonly spacing: number
    g_nodiscard g_inline SkScalar getSpacing() const {
        return font_.getSpacing();
    }

    //! TSDecl: function countText(text: core.Buffer, encoding: Enum<TextEncoding>): number
    int32_t countText(v8::Local<v8::Value> text, int32_t encoding);

    //! TSDecl: function measureText(text: core.Buffer, encoding: Enum<TextEncoding>,
    //!                              paint: null | CkPaint): number
    SkScalar measureText(v8::Local<v8::Value> text, int32_t encoding, v8::Local<v8::Value> paint);

    //! TSDecl: function measureTextBounds(text: core.Buffer, encoding: Enum<TextEncoding>,
    //!                                    paint: null | CkPaint): CkRect
    v8::Local<v8::Value> measureTextBounds(v8::Local<v8::Value> text, int32_t encoding,
                                           v8::Local<v8::Value> paint);

    //! TSDecl: function getBounds(glyphs: Uint16Array, paint: null | CkPaint): Array<CkRect>
    v8::Local<v8::Value> getBounds(v8::Local<v8::Value> glyphs, v8::Local<v8::Value> paint);

    //! TSDecl: function getPos(glyphs: Uint16Array, origin: CkPoint): Array<CkPoint>
    v8::Local<v8::Value> getPos(v8::Local<v8::Value> glyphs, v8::Local<v8::Value> origin);

    //! TSDecl: function getIntercepts(glyphs: Uint16Array, pos: Array<CkPoint>,
    //!                                top: number, bottom: number, paint: null | CkPaint): Float32Array
    v8::Local<v8::Value> getIntercepts(v8::Local<v8::Value> glyphs, v8::Local<v8::Value> pos,
                                       SkScalar top, SkScalar bottom, v8::Local<v8::Value> paint);

    //! TSDecl: function getPath(glyph: number): null | CkPath
    v8::Local<v8::Value> getPath(int32_t glyph);

private:
    SkFont font_;
};

GALLIUM_BINDINGS_GLAMOR_NS_END
#endif //COCOA_GALLIUM_BINDINGS_GLAMOR_CKFONTWRAP_H
