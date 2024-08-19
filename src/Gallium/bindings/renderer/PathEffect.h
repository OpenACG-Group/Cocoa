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

#ifndef COCOA_GALLIUM_BINDINGS_RENDERER_PATHEFFECT_H
#define COCOA_GALLIUM_BINDINGS_RENDERER_PATHEFFECT_H

#include "include/core/SkPathEffect.h"
#include "include/effects/Sk1DPathEffect.h"

#include "Gallium/ffi/JSObject.h"
#include "Gallium/ffi/Class.h"
#include "Gallium/bindings/renderer/Types.h"
#include "Gallium/bindings/renderer/Flattenable.h"
#include "Gallium/bindings/renderer/Path.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

//! TSDecl: @class @nonconstructible @extends(Flattenable) PathEffect
class PathEffect : public Flattenable
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    explicit PathEffect(sk_sp<SkPathEffect> effect) : effect_(std::move(effect)) {}
    ~PathEffect() override = default;

    g_nodiscard sk_sp<SkPathEffect> GetSkPathEffect() const {
        return effect_;
    }

    //! TSDecl: @method @static MakeSum(first: PathEffect, second: PathEffect): PathEffect
    static ffi::RetLocal<v8::Value> MakeSum(const ffi::Class<PathEffect>& first,
                                            const ffi::Class<PathEffect>& second);

    //! TSDecl: @method @static MakeCompose(outer: PathEffect, inner: PathEffect): PathEffect
    static ffi::RetLocal<v8::Value> MakeCompose(const ffi::Class<PathEffect>& outer,
                                                const ffi::Class<PathEffect>& inner);

    //! TSDecl: @method @static Make1D(path: Path, advance: f32, phase: f32,
    //! TSDecl:                        style: Path1DEffectStyle): PathEffect
    static ffi::RetLocal<v8::Value> Make1D(const ffi::Class<Path>& path, float advance, float phase,
                                           const ffi::Enum<SkPath1DPathEffect::Style>& style);

    //! TSDecl: @method @static MakeLine2D(width: f32, matrix: Mat3x3): PathEffect
    static ffi::RetLocal<v8::Value> MakeLine2D(float width, const Mat3x3Adapter& matrix);

    //! TSDecl: @method @static MakePath2D(matrix: Mat3x3, path: Path): PathEffect
    static ffi::RetLocal<v8::Value> MakePath2D(const Mat3x3Adapter& matrix, const ffi::Class<Path>& path);

    //! TSDecl: @method @static MakeCorner(radius: f32): PathEffect
    static ffi::RetLocal<v8::Value> MakeCorner(float radius);

    //! TSDecl: @method @static MakeTrim(startT: f32, stopT: f32, inverted: boolean): PathEffect
    static ffi::RetLocal<v8::Value> MakeTrim(float start_t, float stop_t, bool inverted);

    //! TSDecl: @method @static MakeDiscrete(segLength: f32, dev: f32, seedAssist: u32): PathEffect
    static ffi::RetLocal<v8::Value> MakeDiscrete(float seg_length, float dev, uint32_t seed_assist);

    //! TSDecl: @method @static MakeDash(intervals: @mem(f32), phase: f32): PathEffect
    static ffi::RetLocal<v8::Value> MakeDash(const ffi::Mem<float>& intervals, float phase);

    // TODO(sora): filterPath?

    //! TSDecl: @method @static Deserialize(memory: @mem(u8),
    //! TSDecl:                             deserializers: @union(Deserializers, null))
    //! TSDecl:                             : @union(null, PathEffect)
    FLATTENABLE_IMPL_DESERIALIZE(PathEffect)

private:
    sk_sp<SkData> OnSerializeImpl(SkSerialProcs *procs) override;

    sk_sp<SkPathEffect> effect_;
};
//! TSDecl: @end

GALLIUM_BINDINGS_RENDERER_NS_END
#endif //COCOA_GALLIUM_BINDINGS_RENDERER_PATHEFFECT_H
