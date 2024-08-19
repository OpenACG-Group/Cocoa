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

#ifndef COCOA_GALLIUM_BINDINGS_RENDERER_PAINT_H
#define COCOA_GALLIUM_BINDINGS_RENDERER_PAINT_H

#include "include/core/SkPaint.h"

#include "Gallium/ffi/JSObject.h"
#include "Gallium/ffi/Class.h"
#include "Gallium/ffi/Enum.h"
#include "Gallium/bindings/renderer/Types.h"
#include "Gallium/bindings/renderer/Color.h"
#include "Gallium/bindings/renderer/Rect.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

//! TSDecl: @class Paint
class Paint : public ffi::JSObject
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    //! TSDecl: @constructor()
    Paint() = default;
    ~Paint() override = default;

    explicit Paint(SkPaint paint) : paint_(std::move(paint)) {}

    g_nodiscard SkPaint& GetSkPaint() {
        return paint_;
    }

    //! TSDecl: @method clone(): Paint
    ffi::RetLocal<v8::Value> clone();

    //! TSDecl: @method equalTo(other: Paint): boolean
    ffi::Ret<bool> equalTo(ffi::Class<Paint> other);

    //! TSDecl: @method reset(): void
    ffi::Ret<void> reset();

    //! TSDecl: @property antiAlias: boolean
    //! TSDecl: @property dither: boolean
    //! TSDecl: @property style: Style
    //! TSDecl: @property color: u32
    //! TSDecl: @property alphaf: f32
    //! TSDecl: @property alpha: u8
    //! TSDecl: @property strokeWidth: f32
    //! TSDecl: @property strokeMiter: f32
    //! TSDecl: @property strokeCap: LineCap
    //! TSDecl: @property strokeJoin: LineJoin
    //! TSDecl: @property blendMode: @union(null, BlendMode)
    //! TSDecl: @property pathEffect: @union(null, PathEffect)
    //! TSDecl: @property imageFilter: @union(null, ImageFilter)
    //! TSDecl: @property colorFilter: @union(null, ColorFilter)
    //! TSDecl: @property blender: @union(null, Blender)
    //! TSDecl: @property shader: @union(null, Shader)
#define PAINT_PROPERTIES_MAP(V)                 \
    V(bool, antiAlias)                          \
    V(bool, dither)                             \
    V(ffi::Enum<SkPaint::Style>, style)         \
    V(uint32_t, color)                          \
    V(float, alphaf)                            \
    V(uint8_t, alpha)                           \
    V(float, strokeWidth)                       \
    V(float, strokeMiter)                       \
    V(ffi::Enum<SkPaint::Cap>, strokeCap)       \
    V(ffi::Enum<SkPaint::Join>, strokeJoin)     \
    V(v8::Local<v8::Value>, blendMode)          \
    V(v8::Local<v8::Value>, pathEffect)         \
    V(v8::Local<v8::Value>, imageFilter)        \
    V(v8::Local<v8::Value>, colorFilter)        \
    V(v8::Local<v8::Value>, blender)            \
    V(v8::Local<v8::Value>, shader)

#define ACCESSORS(type, property)               \
    ffi::Ret<type> get_##property();            \
    ffi::Ret<void> set_##property(type value);

    PAINT_PROPERTIES_MAP(ACCESSORS)
#undef ACCESSORS
    // TODO(sora): accessors: MaskFilter?

    //! TSDecl: @method getColor4f(): Color4f
    ffi::Ret<Color4fQuadruple> getColor4f();

    //! TSDecl: @method setColor4f(color: Color4f, cs: @union(ColorSpace, null)): void
    ffi::Ret<void> setColor4f(Color4fQuadruple color, ffi::Opt<ffi::Class<ColorSpace>> cs);

    //! TSDecl: @method nothingToDraw(): boolean
    ffi::Ret<bool> nothingToDraw();

    //! TSDecl: @method canComputeFastBounds(): boolean
    ffi::Ret<bool> canComputeFastBounds();

    //! TSDecl: @method computeFastBounds(orig: Rect): Rect
    ffi::RetLocal<v8::Value> computeFastBounds(const RectAdapter& orig);

private:
    SkPaint paint_;
    v8::Global<v8::Object> path_effect_cache_;
    v8::Global<v8::Object> image_filter_cache_;
    v8::Global<v8::Object> color_filter_cache_;
    v8::Global<v8::Object> blender_cache_;
    v8::Global<v8::Object> shader_cache_;
};
//! TSDecl: @end

GALLIUM_BINDINGS_RENDERER_NS_END
#endif //COCOA_GALLIUM_BINDINGS_RENDERER_PAINT_H
