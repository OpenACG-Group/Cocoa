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

#ifndef COCOA_GALLIUM_BINDINGS_RENDERER_COLOR_H
#define COCOA_GALLIUM_BINDINGS_RENDERER_COLOR_H

#include "include/core/SkColorSpace.h"
#include "include/effects/SkColorMatrix.h"

#include "Gallium/ffi/JSObject.h"
#include "Gallium/ffi/Class.h"
#include "Gallium/ffi/Enum.h"
#include "Gallium/ffi/ArrayBuffer.h"
#include "Gallium/bindings/renderer/Types.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

//! TSDecl: @typedef Color4f = @tuple(f32, f32, f32, f32)
using Color4fQuadruple = std::tuple<float, float, float, float>;

//! TSDecl: @typedef ColorU32 = u32

//! TSDecl: @class @nonconstructible ColorSpace
class ColorSpace : public ffi::JSObject
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    explicit ColorSpace(sk_sp<SkColorSpace> cs) : color_space_(std::move(cs)) {}
    ~ColorSpace() override = default;

    g_nodiscard sk_sp<SkColorSpace> GetColorSpace() const {
        return color_space_;
    }

    //! TSDecl: @method @static MakeSRGB(): ColorSpace
    static ffi::RetLocal<v8::Object> MakeSRGB();

    //! TSDecl: @method @static MakeSRGBLinear(): ColorSpace
    static ffi::RetLocal<v8::Object> MakeSRGBLinear();

    //! TSDecl: @method @static Equals(cs1: ColorSpace, cs2: ColorSpace): boolean
    static ffi::Ret<bool> Equals(ffi::Class<ColorSpace> cs1, ffi::Class<ColorSpace> cs2);

    // TODO(sora): implement `Make()` factory

    //! TSDecl: @method dispose(): void
    ffi::Ret<void> dispose();

    //! TSDecl: @method clone(): ColorSpace
    ffi::RetLocal<v8::Object> clone();

    //! TSDecl: @method gammaCloseToSRGB(): boolean
    ffi::Ret<bool> gammaCloseToSRGB();

    //! TSDecl: @method gammaIsLinear(): boolean
    ffi::Ret<bool> gammaIsLinear();

    // TODO(sora): implement `isNumericalTransferFn()`, etc.

    //! TSDecl: @method makeLinearGamma(): ColorSpace
    ffi::RetLocal<v8::Object> makeLinearGamma();

    //! TSDecl: @method makeSRGBGamma(): ColorSpace
    ffi::RetLocal<v8::Object> makeSRGBGamma();

    //! TSDecl: @method makeColorSpin(): ColorSpace
    ffi::RetLocal<v8::Object> makeColorSpin();

    //! TSDecl: @method isSRGB(): boolean
    ffi::Ret<bool> isSRGB();

private:
    sk_sp<SkColorSpace> color_space_;
};
//! TSDecl: @end

//! TSDecl: @class ColorMatrix
class ColorMatrix : public ffi::JSObject
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    //! TSDecl: @constructor(m00: f32, m01: f32, m02: f32, m03: f32, m10: f32,
    //! TSDecl:              m11: f32, m12: f32, m13: f32, m20: f32, m21: f32,
    //! TSDecl:              m22: f32, m23: f32, m30: f32, m31: f32, m32: f32,
    //! TSDecl:              m33: f32, m40: f32, m41: f32, m42: f32, m43: f32)
    ColorMatrix(float m00, float m01, float m02, float m03, float m04,
                float m10, float m11, float m12, float m13, float m14,
                float m20, float m21, float m22, float m23, float m24,
                float m30, float m31, float m32, float m33, float m34)
        : mat_(m00, m01, m02, m03, m04, m10, m11, m12, m13, m14,
               m20, m21, m22, m23, m24, m30, m31, m32, m33, m34) {}

    explicit ColorMatrix(const SkColorMatrix& mat) : mat_(mat) {}
    ~ColorMatrix() override = default;

    g_nodiscard const SkColorMatrix& GetSkColorMatrix() const {
        return mat_;
    }

    //! TSDecl: @method @static RGBtoYUV(cs: YUVColorSpace): ColorMatrix
    static ffi::RetLocal<v8::Value> RGBtoYUV(const ffi::Enum<SkYUVColorSpace>& cs);

    //! TSDecl: @method @static YUVtoRGB(cs: YUVColorSpace): ColorMatrix
    static ffi::RetLocal<v8::Value> YUVtoRGB(const ffi::Enum<SkYUVColorSpace>& cs);

    //! TSDecl: @method setIdentity(): void
    ffi::Ret<void> setIdentity();

    //! TSDecl: @method setScale(sr: f32, sg: f32, sb: f32, sa: f32): void
    ffi::Ret<void> setScale(float sr, float sg, float sb, float sa);

    //! TSDecl: @method postTranslate(dr: f32, dg: f32, db: f32, da: f32): void
    ffi::Ret<void> postTranslate(float dr, float dg, float db, float da);

    //! TSDecl: @method setConcat(a: ColorMatrix, b: ColorMatrix): void
    ffi::Ret<void> setConcat(const ffi::Class<ColorMatrix>& a,
                             const ffi::Class<ColorMatrix>& b);

    //! TSDecl: @method preConcat(mat: ColorMatrix): void
    ffi::Ret<void> preConcat(const ffi::Class<ColorMatrix>& mat);

    //! TSDecl: @method postConcat(mat: ColorMatrix): void
    ffi::Ret<void> postConcat(const ffi::Class<ColorMatrix>& mat);

    //! TSDecl: @method setSaturation(sat: f32): void
    ffi::Ret<void> setSaturation(float sat);

    //! TSDecl: @method setRowMajor(src: @mem(f32)): void
    ffi::Ret<void> setRowMajor(const ffi::Mem<float>& src);

    //! TSDecl: @method getRowMajor(dst: @mem(f32)): void
    ffi::Ret<void> getRowMajor(const ffi::Mem<float>& dst);

private:
    SkColorMatrix mat_;
};
//! TSDecl: @end

GALLIUM_BINDINGS_RENDERER_NS_END
#endif //COCOA_GALLIUM_BINDINGS_RENDERER_COLOR_H
