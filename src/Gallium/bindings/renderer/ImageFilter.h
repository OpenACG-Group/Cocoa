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

#ifndef COCOA_GALLIUM_BINDINGS_RENDERER_IMAGEFILTER_H
#define COCOA_GALLIUM_BINDINGS_RENDERER_IMAGEFILTER_H

#include "include/core/SkImageFilter.h"
#include "include/effects/SkImageFilters.h"

#include "Gallium/ffi/Enum.h"
#include "Gallium/ffi/Class.h"
#include "Gallium/bindings/renderer/Types.h"
#include "Gallium/bindings/renderer/Rect.h"
#include "Gallium/bindings/renderer/Matrix.h"
#include "Gallium/bindings/renderer/Flattenable.h"
#include "Gallium/bindings/renderer/SamplingOptions.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

class Image;
class Picture;
class ColorFilter;
class Blender;
class Shader;

//! TSDecl: @class @nonconstructible @extends(Flattenable) ImageFilter
class ImageFilter : public Flattenable
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    explicit ImageFilter(sk_sp<SkImageFilter> filter) : filter_(std::move(filter)) {}
    ~ImageFilter() override = default;

    g_nodiscard const sk_sp<SkImageFilter>& GetSkImageFilter() const {
        return filter_;
    }

    //! TSDecl: @method @static Arithmetic(k1: f32, k2: f32, k3: f32, k4: f32,
    //! TSDecl:                            enforcePMColor: boolean,
    //! TSDecl:                            background: @union(ImageFilter, null),
    //! TSDecl:                            foreground: @union(ImageFilter, null),
    //! TSDecl:                            cropRect: @union(Rect, null)): ImageFilter
    static ffi::RetLocal<v8::Value> Arithmetic(float k1, float k2, float k3, float k4,
                                               bool enforce_pm_color,
                                               ffi::Opt<ffi::Class<ImageFilter>> background,
                                               ffi::Opt<ffi::Class<ImageFilter>> foreground,
                                               ffi::Opt<RectAdapter> crop_rect);

    //! TSDecl: @method @static Blend(mode: BlendMode,
    //! TSDecl:                       background: @union(ImageFilter, null),
    //! TSDecl:                       foreground: @union(ImageFilter, null),
    //! TSDecl:                       cropRect: @union(Rect, null)): ImageFilter
    static ffi::RetLocal<v8::Value> Blend(ffi::Enum<SkBlendMode> mode,
                                          ffi::Opt<ffi::Class<ImageFilter>> background,
                                          ffi::Opt<ffi::Class<ImageFilter>> foreground,
                                          ffi::Opt<RectAdapter> crop_rect);

    //! TSDecl: @method @static Blender(blender: Blender,
    //! TSDecl:                         background: @union(ImageFilter, null),
    //! TSDecl:                         foreground: @union(ImageFilter, null),
    //! TSDecl:                         cropRect: @union(Rect, null)): ImageFilter
    static ffi::RetLocal<v8::Value> Blender(ffi::Class<Blender> blender,
                                            ffi::Opt<ffi::Class<ImageFilter>> background,
                                            ffi::Opt<ffi::Class<ImageFilter>> foreground,
                                            ffi::Opt<RectAdapter> crop_rect);

    //! TSDecl: @method @static Blur(sigmaX: f32, sigmaY: f32, tileMode: TileMode,
    //! TSDecl:                      input: @union(null, ImageFilter),
    //! TSDecl:                      cropRect: @union(null, Rect)): ImageFilter
    static ffi::RetLocal<v8::Value> Blur(float sigma_x, float sigma_y,
                                         ffi::Enum<SkTileMode> tile_mode,
                                         ffi::Opt<ffi::Class<ImageFilter>> input,
                                         ffi::Opt<RectAdapter> crop_rect);

    //! TSDecl: @method @static ColorFilter(cf: ColorFilter,
    //! TSDecl:                             input: @union(null, ImageFilter),
    //! TSDecl:                             cropRect: @union(null, Rect)): ImageFilter
    static ffi::RetLocal<v8::Value> ColorFilter(ffi::Class<ColorFilter> cf,
                                                ffi::Opt<ffi::Class<ImageFilter>> input,
                                                ffi::Opt<RectAdapter> crop_rect);

    //! TSDecl: @method @static Compose(outer: ImageFilter, inner: ImageFilter): ImageFilter
    static ffi::RetLocal<v8::Value> Compose(ffi::Class<ImageFilter> outer,
                                            ffi::Class<ImageFilter> inner);

    //! TSDecl: @method @static Crop(rect: Rect, tileMode: TileMode,
    //! TSDecl:                      input: @union(ImageFilter, null)): ImageFilter
    static ffi::RetLocal<v8::Value> Crop(RectAdapter rect,
                                         ffi::Enum<SkTileMode> tile_mode,
                                         ffi::Opt<ffi::Class<ImageFilter>> input);

    //! TSDecl: @method @static DisplacementMap(xChannelSelector: ColorChannel,
    //! TSDecl:                                 yChannelSelector: ColorChannel,
    //! TSDecl:                                 scale: f32,
    //! TSDecl:                                 displacement: @union(ImageFilter, null),
    //! TSDecl:                                 color: @union(ImageFilter, null),
    //! TSDecl:                                 cropRect: @union(Rect, null)): ImageFilter
    static ffi::RetLocal<v8::Value> DisplacementMap(ffi::Enum<SkColorChannel> x_channel_selector,
                                                    ffi::Enum<SkColorChannel> y_channel_selector,
                                                    float scale,
                                                    ffi::Opt<ffi::Class<ImageFilter>> displacement,
                                                    ffi::Opt<ffi::Class<ImageFilter>> color,
                                                    ffi::Opt<RectAdapter> crop_rect);

    //! TSDecl: @method @static DropShadow(dx: f32, dy: f32, sigmaX: f32, sigmaY: f32,
    //! TSDecl:                            color: u32, input: @union(ImageFilter, null),
    //! TSDecl:                            cropRect: @union(Rect, null)): ImageFilter
    static ffi::RetLocal<v8::Value> DropShadow(float dx, float dy, float sigma_x, float sigma_y,
                                               uint32_t color, ffi::Opt<ffi::Class<ImageFilter>> input,
                                               ffi::Opt<RectAdapter> crop_rect);

    //! TSDecl: @method @static DropShadowOnly(dx: f32, dy: f32, sigmaX: f32, sigmaY: f32,
    //! TSDecl:                                color: u32, input: @union(ImageFilter, null),
    //! TSDecl:                                cropRect: @union(Rect, null)): ImageFilter
    static ffi::RetLocal<v8::Value> DropShadowOnly(float dx, float dy, float sigma_x, float sigma_y,
                                                   uint32_t color, ffi::Opt<ffi::Class<ImageFilter>> input,
                                                   ffi::Opt<RectAdapter> crop_rect);

    //! TSDecl: @method @static Empty(): ImageFilter
    static ffi::RetLocal<v8::Value> Empty();

    //! TSDecl: @method @static Image(image: Image, srcRect: Rect, dstRect: Rect,
    //! TSDecl:                       sampling: SamplingOptions): ImageFilter
    static ffi::RetLocal<v8::Value> Image(ffi::Class<Image> image, RectAdapter src_rect,
                                          RectAdapter dst_rect, SamplingOptionsAdapter sampling);

    //! TSDecl: @method @static Magnifier(lensBounds: Rect, zoomAmount: f32, inset: f32,
    //! TSDecl:                           sampling: SamplingOptions, input: @union(ImageFilter, null),
    //! TSDecl:                           cropRect: @union(Rect, null)): ImageFilter
    static ffi::RetLocal<v8::Value> Magnifier(RectAdapter lens_bounds,
                                              float zoom_amount,
                                              float inset,
                                              SamplingOptionsAdapter sampling,
                                              ffi::Opt<ffi::Class<ImageFilter>> input,
                                              ffi::Opt<RectAdapter> crop_rect);

    //! TSDecl: @method @static MatrixConvolution(kernelSize: @tuple(i32, i32),
    //! TSDecl:                                   kernel: @mem(f32), gain: f32, bias: f32,
    //! TSDecl:                                   kernelOffset: @tuple(i32, i32),
    //! TSDecl:                                   tileMode: TileMode,
    //! TSDecl:                                   convolveAlpha: boolean,
    //! TSDecl:                                   input: @union(ImageFilter, null),
    //! TSDecl:                                   cropRect: @union(Rect, null)): ImageFilter
    static ffi::RetLocal<v8::Value> MatrixConvolution(std::tuple<int32_t, int32_t> kernel_size,
                                                      const ffi::Mem<float>& kernel,
                                                      float gain, float bias,
                                                      std::tuple<int32_t, int32_t> kernel_offset,
                                                      ffi::Enum<SkTileMode> tile_mode,
                                                      bool convolve_alpha,
                                                      ffi::Opt<ffi::Class<ImageFilter>> input,
                                                      ffi::Opt<RectAdapter> crop_rect);

    //! TSDecl: @method @static MatrixTransform(matrix: Mat3x3, sampling: SamplingOptions,
    //! TSDecl:                                 input: @union(ImageFilter, null)): ImageFilter
    static ffi::RetLocal<v8::Value> MatrixTransform(Mat3x3Adapter matrix, SamplingOptionsAdapter sampling,
                                                    ffi::Opt<ffi::Class<ImageFilter>> input);

    //! TSDecl: @method @static Merge(filters: @array(ImageFilter), cropRect: @union(Rect, null)): ImageFilter
    static ffi::RetLocal<v8::Value> Merge(v8::Local<v8::Array> filters,
                                          ffi::Opt<RectAdapter> crop_rect);

    //! TSDecl: @method @static Offset(dx: f32, dy: f32, input: @union(ImageFilter, null),
    //! TSDecl:                        cropRect: @union(Rect, null)): ImageFilter
    static ffi::RetLocal<v8::Value> Offset(float dx, float dy, ffi::Opt<ffi::Class<ImageFilter>> input,
                                           ffi::Opt<RectAdapter> crop_rect);

    //! TSDecl: @method @static Picture(pic: Picture, targetRect: Rect): ImageFilter
    static ffi::RetLocal<v8::Value> Picture(ffi::Class<Picture> pic, RectAdapter target_rect);

    // TODO(sora): implement RuntimeShader()

    //! TSDecl: @method @static Shader(shader: Shader, dither: boolean, cropRect: @union(null, Rect)): ImageFilter
    static ffi::RetLocal<v8::Value> Shader(ffi::Class<Shader> shader, bool dither, ffi::Opt<RectAdapter> crop_rect);

    //! TSDecl: @method @static Tile(src: Rect, dst: Rect, input: @union(null, ImageFilter)): ImageFilter
    static ffi::RetLocal<v8::Value> Tile(RectAdapter src, RectAdapter dst,
                                         ffi::Opt<ffi::Class<ImageFilter>> input);

    //! TSDecl: @method @static Dilate(radiusX: f32, radiusY: f32, input: @union(null, ImageFilter),
    //! TSDecl:                        cropRect: @union(Rect, null)): ImageFilter
    static ffi::RetLocal<v8::Value> Dilate(float radius_x, float radius_y,
                                           ffi::Opt<ffi::Class<ImageFilter>> input,
                                           ffi::Opt<RectAdapter> crop_rect);

    //! TSDecl: @method @static Erode(radiusX: f32, radiusY: f32, input: @union(null, ImageFilter),
    //! TSDecl:                       cropRect: @union(Rect, null)): ImageFilter
    static ffi::RetLocal<v8::Value> Erode(float radius_x, float radius_y,
                                          ffi::Opt<ffi::Class<ImageFilter>> input,
                                          ffi::Opt<RectAdapter> crop_rect);

    //! TSDecl: @method @static DistantLitDiffuse(direction: @tuple(f32, f32, f32),
    //! TSDecl:                                   lightColor: u32, surfaceScale: f32, kd: f32,
    //! TSDecl:                                   input: @union(null, ImageFilter),
    //! TSDecl:                                   cropRect: @union(Rect, null)): ImageFilter
    static ffi::RetLocal<v8::Value> DistantLitDiffuse(std::tuple<float, float, float> direction,
                                                      uint32_t light_color, float surface_scale,
                                                      float kd,
                                                      ffi::Opt<ffi::Class<ImageFilter>> input,
                                                      ffi::Opt<RectAdapter> crop_rect);

    //! TSDecl: @method @static PointLitDiffuse(location: @tuple(f32, f32, f32),
    //! TSDecl:                                 lightColor: u32, surfaceScale: f32, kd: f32,
    //! TSDecl:                                 input: @union(null, ImageFilter),
    //! TSDecl:                                 cropRect: @union(Rect, null)): ImageFilter
    static ffi::RetLocal<v8::Value> PointLitDiffuse(std::tuple<float, float, float> location,
                                                    uint32_t light_color, float surface_scale,
                                                    float kd,
                                                    ffi::Opt<ffi::Class<ImageFilter>> input,
                                                    ffi::Opt<RectAdapter> crop_rect);

    //! TSDecl: @method @static SpotLitDiffuse(location: @tuple(f32, f32, f32),
    //! TSDecl:                                target: @tuple(f32, f32, f32),
    //! TSDecl:                                falloffExponent: f32, cutoffAngle: f32,
    //! TSDecl:                                lightColor: u32, surfaceScale: f32, kd: f32,
    //! TSDecl:                                input: @union(null, ImageFilter),
    //! TSDecl:                                cropRect: @union(Rect, null)): ImageFilter
    static ffi::RetLocal<v8::Value> SpotLitDiffuse(std::tuple<float, float, float> location,
                                                   std::tuple<float, float, float> target,
                                                   float falloff_exponent,
                                                   float cutoff_angle,
                                                   uint32_t light_color,
                                                   float surface_scale,
                                                   float kd,
                                                   ffi::Opt<ffi::Class<ImageFilter>> input,
                                                   ffi::Opt<RectAdapter> crop_rect);

    //! TSDecl: @method @static DistantLitSpecular(direction: @tuple(f32, f32, f32),
    //! TSDecl:                                    lightColor: u32, surfaceScale: f32, ks: f32,
    //! TSDecl:                                    shininess: f32,
    //! TSDecl:                                    input: @union(null, ImageFilter),
    //! TSDecl:                                    cropRect: @union(Rect, null)): ImageFilter
    static ffi::RetLocal<v8::Value> DistantLitSpecular(std::tuple<float, float, float> direction,
                                                       uint32_t light_color, float surface_scale,
                                                       float ks, float shininess,
                                                       ffi::Opt<ffi::Class<ImageFilter>> input,
                                                       ffi::Opt<RectAdapter> crop_rect);

    //! TSDecl: @method @static PointLitSpecular(location: @tuple(f32, f32, f32),
    //! TSDecl:                                  lightColor: u32, surfaceScale: f32, ks: f32,
    //! TSDecl:                                  shininess: f32,
    //! TSDecl:                                  input: @union(null, ImageFilter),
    //! TSDecl:                                  cropRect: @union(Rect, null)): ImageFilter
    static ffi::RetLocal<v8::Value> PointLitSpecular(std::tuple<float, float, float> location,
                                                     uint32_t light_color, float surface_scale,
                                                     float ks, float shininess,
                                                     ffi::Opt<ffi::Class<ImageFilter>> input,
                                                     ffi::Opt<RectAdapter> crop_rect);

    //! TSDecl: @method @static SpotLitSpecular(location: @tuple(f32, f32, f32),
    //! TSDecl:                                 target: @tuple(f32, f32, f32),
    //! TSDecl:                                 falloffExponent: f32, cutoffAngle: f32,
    //! TSDecl:                                 lightColor: u32, surfaceScale: f32, ks: f32,
    //! TSDecl:                                 shininess: f32,
    //! TSDecl:                                 input: @union(null, ImageFilter),
    //! TSDecl:                                 cropRect: @union(Rect, null)): ImageFilter
    static ffi::RetLocal<v8::Value> SpotLitSpecular(std::tuple<float, float, float> location,
                                                    std::tuple<float, float, float> target,
                                                    float falloff_exponent,
                                                    float cutoff_angle,
                                                    uint32_t light_color,
                                                    float surface_scale,
                                                    float ks,
                                                    float shininess,
                                                    ffi::Opt<ffi::Class<ImageFilter>> input,
                                                    ffi::Opt<RectAdapter> crop_rect);

    //! TSDecl: @method @static Deserialize(memory: @mem(u8),
    //! TSDecl:                             deserializers: @union(Deserializers, null))
    //! TSDecl:                             : @union(null, ImageFilter)
    FLATTENABLE_IMPL_DESERIALIZE(ImageFilter)

    //! TSDecl: @method filterBounds(src: Rect, ctm: Mat3x3, dir: MapDirection,
    //! TSDecl:                      inputRect: @union(Rect, null)): Rect
    ffi::RetLocal<v8::Value> filterBounds(RectAdapter src,
                                          Mat3x3Adapter ctm,
                                          ffi::Enum<SkImageFilter::MapDirection> dir,
                                          ffi::Opt<RectAdapter> input_rect);

    //! TSDecl: @method computeFastBounds(bounds: Rect): Rect
    ffi::RetLocal<v8::Value> computeFastBounds(const RectAdapter& bounds);

    //! TSDecl: @property @readonly canComputeFastBounds: boolean
    ffi::Ret<bool> getCanComputeFastBounds() {
        return filter_->canComputeFastBounds();
    }

    //! TSDecl: @method makeWithLocalMatrix(matrix: Mat3x3): @union(null, ImageFilter)
    ffi::RetLocal<v8::Value> makeWithLocalMatrix(const Mat3x3Adapter& matrix);

private:
    sk_sp<SkData> OnSerializeImpl(SkSerialProcs *procs) override;

    sk_sp<SkImageFilter> filter_;
};
//! TSDecl: @end

GALLIUM_BINDINGS_RENDERER_NS_END
#endif //COCOA_GALLIUM_BINDINGS_RENDERER_IMAGEFILTER_H
