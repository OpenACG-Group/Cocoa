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

#include "include/core/SkPoint3.h"

#include "Gallium/bindings/renderer/Picture.h"
#include "Gallium/bindings/renderer/Image.h"
#include "Gallium/bindings/renderer/ImageFilter.h"
#include "Gallium/bindings/renderer/ColorFilter.h"
#include "Gallium/bindings/renderer/Blender.h"
#include "Gallium/bindings/renderer/Shader.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

#define IMFLT(p)  ((p) ? (*p)->GetSkImageFilter() : nullptr)
#define CROP(p)   ((p) ? SkImageFilters::CropRect(**p) : SkImageFilters::CropRect())

#define MAKE(func, ...) \
    sk_sp<SkImageFilter> filter = SkImageFilters::func(__VA_ARGS__);  \
    if (!filter) {                                                    \
        return ffi::Fail(ffi::kErr, "failed to create image filter"); \
    }                                                                 \
    return ffi::JSObject::New<ImageFilter>(v8::Isolate::GetCurrent(), filter);

ffi::RetLocal<v8::Value> ImageFilter::Arithmetic(float k1, float k2, float k3, float k4,
                                                 bool enforce_pm_color,
                                                 ffi::Opt<ffi::Class<ImageFilter>> background,
                                                 ffi::Opt<ffi::Class<ImageFilter>> foreground,
                                                 ffi::Opt<RectAdapter> crop_rect)
{
    MAKE(Arithmetic, k1, k2, k3, k4, enforce_pm_color,
         IMFLT(background), IMFLT(foreground), CROP(crop_rect))
}

ffi::RetLocal<v8::Value> ImageFilter::Blend(ffi::Enum<SkBlendMode> mode,
                                            ffi::Opt<ffi::Class<ImageFilter>> background,
                                            ffi::Opt<ffi::Class<ImageFilter>> foreground,
                                            ffi::Opt<RectAdapter> crop_rect)
{
    MAKE(Blend, *mode, IMFLT(background), IMFLT(foreground), CROP(crop_rect))
}

using Blender_ = Blender;
ffi::RetLocal<v8::Value> ImageFilter::Blender(ffi::Class<Blender_> blender,
                                              ffi::Opt<ffi::Class<ImageFilter>> background,
                                              ffi::Opt<ffi::Class<ImageFilter>> foreground,
                                              ffi::Opt<RectAdapter> crop_rect)
{
    MAKE(Blend, blender->GetSkBlender(), IMFLT(background), IMFLT(foreground), CROP(crop_rect))
}

ffi::RetLocal<v8::Value> ImageFilter::Compose(ffi::Class<ImageFilter> outer,
                                              ffi::Class<ImageFilter> inner)
{
    MAKE(Compose, outer->GetSkImageFilter(), inner->GetSkImageFilter())
}

using ColorFilter_ = ColorFilter;
ffi::RetLocal<v8::Value> ImageFilter::ColorFilter(ffi::Class<ColorFilter_> cf,
                                                  ffi::Opt<ffi::Class<ImageFilter>> input,
                                                  ffi::Opt<RectAdapter> crop_rect)
{
    MAKE(ColorFilter, cf->GetSkColorFilter(), IMFLT(input), CROP(crop_rect))
}

ffi::RetLocal<v8::Value> ImageFilter::Crop(RectAdapter rect,
                                           ffi::Enum<SkTileMode> tile_mode,
                                           ffi::Opt<ffi::Class<ImageFilter>> input)
{
    MAKE(Crop, *rect, *tile_mode, IMFLT(input))
}

ffi::RetLocal<v8::Value> ImageFilter::Blur(float sigma_x, float sigma_y,
                                           ffi::Enum<SkTileMode> tile_mode,
                                           ffi::Opt<ffi::Class<ImageFilter>> input,
                                           ffi::Opt<RectAdapter> crop_rect)
{
    MAKE(Blur, sigma_x, sigma_y, *tile_mode, IMFLT(input), CROP(crop_rect))
}

ffi::RetLocal<v8::Value>
ImageFilter::DisplacementMap(ffi::Enum<SkColorChannel> x_channel_selector,
                             ffi::Enum<SkColorChannel> y_channel_selector,
                             float scale,
                             ffi::Opt<ffi::Class<ImageFilter>> displacement,
                             ffi::Opt<ffi::Class<ImageFilter>> color,
                             ffi::Opt<RectAdapter> crop_rect)
{
    MAKE(DisplacementMap, *x_channel_selector, *y_channel_selector, scale,
         IMFLT(displacement), IMFLT(color), CROP(crop_rect))
}

ffi::RetLocal<v8::Value> ImageFilter::DropShadow(float dx, float dy,
                                                 float sigma_x, float sigma_y, uint32_t color,
                                                 ffi::Opt<ffi::Class<ImageFilter>> input,
                                                 ffi::Opt<RectAdapter> crop_rect)
{
    MAKE(DropShadow, dx, dy, sigma_x, sigma_y, color, IMFLT(input), CROP(crop_rect))
}

ffi::RetLocal<v8::Value> ImageFilter::DropShadowOnly(float dx, float dy,
                                                     float sigma_x, float sigma_y, uint32_t color,
                                                     ffi::Opt<ffi::Class<ImageFilter>> input,
                                                     ffi::Opt<RectAdapter> crop_rect)
{
    MAKE(DropShadowOnly, dx, dy, sigma_x, sigma_y, color, IMFLT(input), CROP(crop_rect))
}

ffi::RetLocal<v8::Value> ImageFilter::Empty()
{
    MAKE(Empty)
}

using Image_ = Image;
ffi::RetLocal<v8::Value> ImageFilter::Image(ffi::Class<Image_> image,
                                            RectAdapter src_rect,
                                            RectAdapter dst_rect,
                                            SamplingOptionsAdapter sampling)
{
    MAKE(Image, image->GetSkImage(), *src_rect, *dst_rect, *sampling)
}

ffi::RetLocal<v8::Value> ImageFilter::Magnifier(RectAdapter lens_bounds,
                                                float zoom_amount, float inset,
                                                SamplingOptionsAdapter sampling,
                                                ffi::Opt<ffi::Class<ImageFilter>> input,
                                                ffi::Opt<RectAdapter> crop_rect)
{
    MAKE(Magnifier, *lens_bounds, zoom_amount, inset, *sampling, IMFLT(input), CROP(crop_rect))
}

ffi::RetLocal<v8::Value> ImageFilter::MatrixConvolution(std::tuple<int32_t, int32_t> kernel_size_tuple,
                                                        const ffi::Mem<float>& kernel,
                                                        float gain, float bias,
                                                        std::tuple<int32_t, int32_t> kernel_offset_tuple,
                                                        ffi::Enum<SkTileMode> tile_mode,
                                                        bool convolve_alpha,
                                                        ffi::Opt<ffi::Class<ImageFilter>> input,
                                                        ffi::Opt<RectAdapter> crop_rect)
{
    auto [ks_width, ks_height] = kernel_size_tuple;
    if (ks_width <= 0 || ks_height <= 0)
        return ffi::Fail(ffi::kRangeErr, "invalid kernel size");

    auto [koff_x, koff_y] = kernel_offset_tuple;
    if (kernel.Size() != ks_width * ks_height)
        return ffi::Fail(ffi::kErr, "kernel matrix does not fit the kernel size");

    MAKE(MatrixConvolution, SkISize::Make(ks_width, ks_height), kernel.Address(),
         gain, bias, SkIPoint::Make(koff_x, koff_y), *tile_mode, convolve_alpha,
         IMFLT(input), CROP(crop_rect))
}

ffi::RetLocal<v8::Value> ImageFilter::MatrixTransform(Mat3x3Adapter matrix,
                                                      SamplingOptionsAdapter sampling,
                                                      ffi::Opt<ffi::Class<ImageFilter>> input)
{
    MAKE(MatrixTransform, *matrix, *sampling, IMFLT(input))
}

ffi::RetLocal<v8::Value> ImageFilter::Merge(v8::Local<v8::Array> filters,
                                            ffi::Opt<RectAdapter> crop_rect)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    uint32_t count = filters->Length();
    if (count == 0)
        return ffi::Fail(ffi::kErr, "no filters are provided");

    std::vector<sk_sp<SkImageFilter>> filters_vec(count);
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    for (uint32_t i = 0; i < count; i++)
    {
        v8::Local<v8::Value> element;
        if (!filters->Get(ctx, i).ToLocal(&element))
            return ffi::FreePropagate();

        auto res = ffi::Cast<ffi::Class<ImageFilter>>::From(isolate, element);
        if (res.HasError())
            return res.GetError();

        filters_vec[i] = res.Extract()->GetSkImageFilter();
    }

    MAKE(Merge, filters_vec.data(), count, CROP(crop_rect))
}

ffi::RetLocal<v8::Value> ImageFilter::Offset(float dx, float dy,
                                             ffi::Opt<ffi::Class<ImageFilter>> input,
                                             ffi::Opt<RectAdapter> crop_rect)
{
    MAKE(Offset, dx, dy, IMFLT(input), CROP(crop_rect))
}

using Picture_ = Picture;
ffi::RetLocal<v8::Value> ImageFilter::Picture(ffi::Class<Picture_> pic, RectAdapter target_rect)
{
    MAKE(Picture, pic->GetSkPicture(), *target_rect)
}

using Shader_ = Shader;
ffi::RetLocal<v8::Value> ImageFilter::Shader(ffi::Class<Shader_> shader, bool dither,
                                             ffi::Opt<RectAdapter> crop_rect)
{
    MAKE(Shader, shader->GetSkShader(),
         dither ? SkImageFilters::Dither::kYes : SkImageFilters::Dither::kNo,
         CROP(crop_rect))
}

ffi::RetLocal<v8::Value> ImageFilter::Tile(RectAdapter src, RectAdapter dst,
                                           ffi::Opt<ffi::Class<ImageFilter>> input)
{
    MAKE(Tile, *src, *dst, IMFLT(input))
}

ffi::RetLocal<v8::Value> ImageFilter::Dilate(float radius_x, float radius_y,
                                             ffi::Opt<ffi::Class<ImageFilter>> input,
                                             ffi::Opt<RectAdapter> crop_rect)
{
    MAKE(Dilate, radius_x, radius_y, IMFLT(input), CROP(crop_rect))
}

ffi::RetLocal<v8::Value> ImageFilter::Erode(float radius_x, float radius_y,
                                            ffi::Opt<ffi::Class<ImageFilter>> input,
                                            ffi::Opt<RectAdapter> crop_rect)
{
    MAKE(Erode, radius_x, radius_y, IMFLT(input), CROP(crop_rect))
}

ffi::RetLocal<v8::Value> ImageFilter::DistantLitDiffuse(std::tuple<float, float, float> direction,
                                                        uint32_t light_color,
                                                        float surface_scale,
                                                        float kd,
                                                        ffi::Opt<ffi::Class<ImageFilter>> input,
                                                        ffi::Opt<RectAdapter> crop_rect)
{
    auto [d_x, d_y, d_z] = direction;
    MAKE(DistantLitDiffuse, SkPoint3::Make(d_x, d_y, d_z), light_color,
         surface_scale, kd, IMFLT(input), CROP(crop_rect))
}

ffi::RetLocal<v8::Value> ImageFilter::PointLitDiffuse(std::tuple<float, float, float> location,
                                                      uint32_t light_color,
                                                      float surface_scale,
                                                      float kd,
                                                      ffi::Opt<ffi::Class<ImageFilter>> input,
                                                      ffi::Opt<RectAdapter> crop_rect)
{
    auto [lx, ly, lz] = location;
    MAKE(PointLitDiffuse, SkPoint3::Make(lx, ly, lz), light_color,
         surface_scale, kd, IMFLT(input), CROP(crop_rect))
}

ffi::RetLocal<v8::Value> ImageFilter::SpotLitDiffuse(std::tuple<float, float, float> location,
                                                     std::tuple<float, float, float> target,
                                                     float falloff_exponent,
                                                     float cutoff_angle,
                                                     uint32_t light_color,
                                                     float surface_scale,
                                                     float kd,
                                                     ffi::Opt<ffi::Class<ImageFilter>> input,
                                                     ffi::Opt<RectAdapter> crop_rect)
{
    auto [lx, ly, lz] = location;
    auto [tx, ty, tz] = target;
    MAKE(SpotLitDiffuse, SkPoint3::Make(lx, ly, lz), SkPoint3::Make(tx, ty, tz),
         falloff_exponent, cutoff_angle, light_color, surface_scale, kd,
         IMFLT(input), CROP(crop_rect))
}

ffi::RetLocal<v8::Value> ImageFilter::DistantLitSpecular(std::tuple<float, float, float> direction,
                                                         uint32_t light_color,
                                                         float surface_scale,
                                                         float ks,
                                                         float shininess,
                                                         ffi::Opt<ffi::Class<ImageFilter>> input,
                                                         ffi::Opt<RectAdapter> crop_rect)
{
    auto [dx, dy, dz] = direction;
    MAKE(DistantLitSpecular, SkPoint3::Make(dx, dy, dz), light_color,
         surface_scale, ks, shininess, IMFLT(input), CROP(crop_rect))
}

ffi::RetLocal<v8::Value> ImageFilter::PointLitSpecular(std::tuple<float, float, float> location,
                                                       uint32_t light_color,
                                                       float surface_scale,
                                                       float ks,
                                                       float shininess,
                                                       ffi::Opt<ffi::Class<ImageFilter>> input,
                                                       ffi::Opt<RectAdapter> crop_rect)
{
    auto [lx, ly, lz] = location;
    MAKE(PointLitSpecular, SkPoint3::Make(lz, ly, lz), light_color, surface_scale,
         ks, shininess, IMFLT(input), CROP(crop_rect))
}

ffi::RetLocal<v8::Value> ImageFilter::SpotLitSpecular(std::tuple<float, float, float> location,
                                                      std::tuple<float, float, float> target,
                                                      float falloff_exponent,
                                                      float cutoff_angle,
                                                      uint32_t light_color,
                                                      float surface_scale,
                                                      float ks,
                                                      float shininess,
                                                      ffi::Opt<ffi::Class<ImageFilter>> input,
                                                      ffi::Opt<RectAdapter> crop_rect)
{
    auto [lx, ly, lz] = location;
    auto [tx, ty, tz] = target;
    MAKE(SpotLitSpecular, SkPoint3::Make(lx, ly, lz), SkPoint3::Make(tx, ty, tz),
         falloff_exponent, cutoff_angle, light_color, surface_scale, ks, shininess,
         IMFLT(input), CROP(crop_rect))
}

ffi::RetLocal<v8::Value> ImageFilter::filterBounds(RectAdapter src,
                                                   Mat3x3Adapter ctm,
                                                   ffi::Enum<SkImageFilter::MapDirection> dir,
                                                   ffi::Opt<RectAdapter> input_rect)
{
    SkIRect input_irect;
    SkIRect *input_irect_p = nullptr;
    if (input_rect)
    {
        input_irect = (**input_rect).round();
        input_irect_p = &input_irect;
    }

    return CreateJSRect(v8::Isolate::GetCurrent(),
                        SkRect::Make(filter_->filterBounds((*src).round(), *ctm, *dir, input_irect_p)));
}

ffi::RetLocal<v8::Value> ImageFilter::computeFastBounds(const RectAdapter& bounds)
{
    return CreateJSRect(v8::Isolate::GetCurrent(),
                        filter_->computeFastBounds(*bounds));
}

ffi::RetLocal<v8::Value> ImageFilter::makeWithLocalMatrix(const Mat3x3Adapter& matrix)
{
    sk_sp<SkImageFilter> filter = filter_->makeWithLocalMatrix(*matrix);
    if (!filter)
        return ffi::Fail(ffi::kErr, "this ImageFilter cannot be represented with a matrix");
    return ffi::JSObject::New<ImageFilter>(v8::Isolate::GetCurrent(), filter);
}

sk_sp<SkData> ImageFilter::OnSerializeImpl(SkSerialProcs *procs)
{
    return filter_->serialize(procs);
}


GALLIUM_BINDINGS_RENDERER_NS_END
