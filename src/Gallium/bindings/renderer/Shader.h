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

#ifndef COCOA_GALLIUM_BINDINGS_RENDERER_SHADER_H
#define COCOA_GALLIUM_BINDINGS_RENDERER_SHADER_H

#include "include/core/SkShader.h"
#include "include/effects/SkGradientShader.h"

#include "Gallium/bindings/renderer/Types.h"
#include "Gallium/bindings/renderer/Flattenable.h"
#include "Gallium/bindings/renderer/Color.h"
#include "Gallium/bindings/renderer/Matrix.h"
#include "Gallium/bindings/renderer/Rect.h"
#include "Gallium/bindings/renderer/SamplingOptions.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

class Blender;
class Image;
class ColorFilter;

//! TSDecl: @interface GradientInterpolation
struct GradientInterpolation
{
    SkGradientShader::Interpolation Unwrap() const;

    //! TSDecl: @property @optional inPremul: boolean
    ffi::Opt<bool> in_premul;

    //! TSDecl: @property @optional colorSpace: GradientInterpColorSpace
    ffi::Opt<ffi::Enum<SkGradientShader::Interpolation::ColorSpace>> color_space;

    //! TSDecl: @property @optional hueMethod: GradientInterpHueMethod
    ffi::Opt<ffi::Enum<SkGradientShader::Interpolation::HueMethod>> hue_method;
};
//! TSDecl: @end

//! TSDecl: @class @nonconstructible @extends(Flattenable) Shader
class Shader : public Flattenable
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    explicit Shader(sk_sp<SkShader> shader) : shader_(std::move(shader)) {}
    ~Shader() override = default;

    g_nodiscard sk_sp<SkShader> GetSkShader() const {
        return shader_;
    }

    //! TSDecl: @method @static Empty(): Shader
    static ffi::RetLocal<v8::Value> Empty();

    //! TSDecl: @method @static Color(color: Color4f, cs: ColorSpace): Shader
    static ffi::RetLocal<v8::Value> Color(Color4fQuadruple color,
                                          const ffi::Class<ColorSpace>& cs);

    //! TSDecl: @method @static Blend(mode: BlendMode, dst: Shader, src: Shader): Shader
    static ffi::RetLocal<v8::Value> Blend(const ffi::Enum<SkBlendMode>& mode,
                                          const ffi::Class<Shader>& dst,
                                          const ffi::Class<Shader>& src);

    //! TSDecl: @method @static Blender(blender: Blender, dst: Shader, src: Shader): Shader
    static ffi::RetLocal<v8::Value> Blender(const ffi::Class<Blender>& blender,
                                            const ffi::Class<Shader>& dst,
                                            const ffi::Class<Shader>& src);

    //! TSDecl: @method @static CoordClamp(shader: Shader, subset: Rect): Shader
    static ffi::RetLocal<v8::Value> CoordClamp(const ffi::Class<Shader>& shader,
                                               const RectAdapter& subset);

    //! TSDecl: @method @static Image(image: Image, tmx: TileMode, tmy: TileMode,
    //! TSDecl:                       options: SamplingOptions,
    //! TSDecl:                       localMatrix: @union(Mat3x3, null)): Shader
    static ffi::RetLocal<v8::Value> Image(const ffi::Class<renderer::Image>& image,
                                          const ffi::Enum<SkTileMode>& tmx,
                                          const ffi::Enum<SkTileMode>& tmy,
                                          const SamplingOptionsAdapter& options,
                                          const ffi::Opt<Mat3x3Adapter>& local_matrix);

    //! TSDecl: @method @static RawImage(image: Image, tmx: TileMode, tmy: TileMode,
    //! TSDecl:                          options: SamplingOptions,
    //! TSDecl:                          localMatrix: @union(Mat3x3, null)): Shader
    static ffi::RetLocal<v8::Value> RawImage(const ffi::Class<renderer::Image>& image,
                                             const ffi::Enum<SkTileMode>& tmx,
                                             const ffi::Enum<SkTileMode>& tmy,
                                             const SamplingOptionsAdapter& options,
                                             const ffi::Opt<Mat3x3Adapter>& local_matrix);

    //! TSDecl: @method @static LinearGradient(pts: @tuple(f32, f32, f32, f32),
    //! TSDecl:                                colors: @array(Color4f),
    //! TSDecl:                                colorSpace: ColorSpace,
    //! TSDecl:                                pos: @array(f32),
    //! TSDecl:                                mode: TileMode,
    //! TSDecl:                                interpolation: GradientInterpolation,
    //! TSDecl:                                localMatrix: @union(Mat3x3, null)): Shader
    static ffi::RetLocal<v8::Value> LinearGradient(const std::tuple<float, float, float, float>& pts,
                                                   v8::Local<v8::Array> colors,
                                                   const ffi::Class<ColorSpace>& color_space,
                                                   const std::vector<float>& pos,
                                                   const ffi::Enum<SkTileMode>& mode,
                                                   const ffi::IFace<GradientInterpolation>& interpolation,
                                                   const ffi::Opt<Mat3x3Adapter>& local_matrix);

    //! TSDecl: @method @static RadialGradient(center: @tuple(f32, f32),
    //! TSDecl:                                radius: f32,
    //! TSDecl:                                colors: @array(Color4f),
    //! TSDecl:                                colorSpace: ColorSpace,
    //! TSDecl:                                pos: @array(f32),
    //! TSDecl:                                mode: TileMode,
    //! TSDecl:                                interpolation: GradientInterpolation,
    //! TSDecl:                                localMatrix: @union(Mat3x3, null)): Shader
    static ffi::RetLocal<v8::Value> RadialGradient(const std::tuple<float, float>& center,
                                                   float radius,
                                                   v8::Local<v8::Array> colors,
                                                   const ffi::Class<ColorSpace>& color_space,
                                                   const std::vector<float>& pos,
                                                   const ffi::Enum<SkTileMode>& mode,
                                                   const ffi::IFace<GradientInterpolation>& interpolation,
                                                   const ffi::Opt<Mat3x3Adapter>& local_matrix);

    //! TSDecl: @method @static TwoPointConicalGradient(start: @tuple(f32, f32), startRadius: f32,
    //! TSDecl:                                         end: @tuple(f32, f32), endRadius: f32,
    //! TSDecl:                                         colors: @array(Color4f),
    //! TSDecl:                                         colorSpace: ColorSpace,
    //! TSDecl:                                         pos: @array(f32),
    //! TSDecl:                                         mode: TileMode,
    //! TSDecl:                                         interpolation: GradientInterpolation,
    //! TSDecl:                                         localMatrix: @union(Mat3x3, null)): Shader
    static ffi::RetLocal<v8::Value> TwoPointConicalGradient(const std::tuple<float, float>& start,
                                                            float start_radius,
                                                            const std::tuple<float, float>& end,
                                                            float end_radius,
                                                            v8::Local<v8::Array> colors,
                                                            const ffi::Class<ColorSpace>& color_space,
                                                            const std::vector<float>& pos,
                                                            const ffi::Enum<SkTileMode>& mode,
                                                            const ffi::IFace<GradientInterpolation>& interpolation,
                                                            const ffi::Opt<Mat3x3Adapter>& local_matrix);

    //! TSDecl: @method @static SweepGradient(center: @tuple(f32, f32),
    //! TSDecl:                               startAngle: f32, endAngle: f32,
    //! TSDecl:                               colors: @array(Color4f),
    //! TSDecl:                               colorSpace: ColorSpace,
    //! TSDecl:                               pos: @array(f32),
    //! TSDecl:                               mode: TileMode,
    //! TSDecl:                               interpolation: GradientInterpolation,
    //! TSDecl:                               localMatrix: @union(Mat3x3, null)): Shader
    static ffi::RetLocal<v8::Value> SweepGradient(const std::tuple<float, float>& center,
                                                  float start_angle,
                                                  float end_angle,
                                                  v8::Local<v8::Array> colors,
                                                  const ffi::Class<ColorSpace>& color_space,
                                                  const std::vector<float>& pos,
                                                  const ffi::Enum<SkTileMode>& mode,
                                                  const ffi::IFace<GradientInterpolation>& interpolation,
                                                  const ffi::Opt<Mat3x3Adapter>& local_matrix);

    //! TSDecl: @method @static FractalNoise(baseFrequencyX: f32, baseFrequencyY: f32,
    //! TSDecl:                              numOctaves: i32, seed: f32,
    //! TSDecl:                              tileSize: @union(@tuple(f32, f32), null)): Shader
    static ffi::RetLocal<v8::Value> FractalNoise(float base_freq_x, float base_freq_y,
                                                 int num_octaves, float seed,
                                                 const ffi::Opt<std::tuple<float, float>>& tile_size);

    //! TSDecl: @method @static Turbulence(baseFrequencyX: f32, baseFrequencyY: f32,
    //! TSDecl:                           numOctaves: i32, seed: f32,
    //! TSDecl:                           tileSize: @union(@tuple(f32, f32), null)): Shader
    static ffi::RetLocal<v8::Value> Turbulence(float base_freq_x, float base_freq_y,
                                               int num_octaves, float seed,
                                               const ffi::Opt<std::tuple<float, float>>& tile_size);


    //! TSDecl: @method @static Deserialize(memory: @mem(u8),
    //! TSDecl:                             deserializers: @union(Deserializers, null))
    //! TSDecl:                             : @union(null, Shader)
    FLATTENABLE_IMPL_DESERIALIZE(Shader)

    //! TSDecl: @property @readonly isOpaque: boolean
    ffi::Ret<bool> getIsOpaque() {
        return shader_->isOpaque();
    }

    //! TSDecl: @method dispose(): void
    ffi::Ret<void> dispose();

    //! TSDecl: @method makeWithLocalMatrix(matrix: Mat3x3): Shader
    ffi::RetLocal<v8::Value> makeWithLocalMatrix(const Mat3x3Adapter& matrix);

    //! TSDecl: @method makeWithColorFilter(cf: ColorFilter): Shader
    ffi::RetLocal<v8::Value> makeWithColorFilter(const ffi::Class<ColorFilter>& cf);

    //! TSDecl: @method makeWithWorkingColorSpace(cs: ColorSpace): Shader
    ffi::RetLocal<v8::Value> makeWithWorkingColorSpace(const ffi::Class<ColorSpace>& cs);

private:
    sk_sp<SkData> OnSerializeImpl(SkSerialProcs *procs) override;

    sk_sp<SkShader> shader_;
};
//! TSDecl: @end

GALLIUM_BINDINGS_RENDERER_NS_END
#endif //COCOA_GALLIUM_BINDINGS_RENDERER_SHADER_H
