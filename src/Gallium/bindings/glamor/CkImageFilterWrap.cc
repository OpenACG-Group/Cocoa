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

#include "include/core/SkData.h"
#include "include/core/SkPoint3.h"
#include "include/effects/SkImageFilters.h"
#include "fmt/format.h"

#include "Gallium/bindings/glamor/Exports.h"
#include "Gallium/bindings/glamor/CkMatrixWrap.h"
#include "Gallium/bindings/glamor/EffectDSLParser.h"
#include "Gallium/bindings/glamor/EffectDSLBuilderHelperMacros.h"
#include "Gallium/binder/TypeTraits.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

namespace {

template<typename T>
using Nullable = EffectStackOperand::Nullable<T>;

//! FilterDecl: crop(Rect crop, Int? tile_mode, ImageFilter input)
DEF_BUILDER(crop)
{
    CHECK_ARGC(3, crop)

    // Arguments in the stack should be taken in the reverse order
    POP_ARGUMENT_CHECKED(input, ImageFilter, crop)
    POP_ARGUMENT(tile_mode_int, Integer)
    POP_ARGUMENT_CHECKED(crop, Rect, crop)

    if (tile_mode_int)
    {
        if (*tile_mode_int < 0 || *tile_mode_int > static_cast<int>(SkTileMode::kLastTileMode))
            g_throw(RangeError, "Invalid enumeration value in argument `tile_mode` for `crop` filter");
    }

    return SkImageFilters::Crop(
        *crop,
        tile_mode_int ? static_cast<SkTileMode>(*tile_mode_int) : SkTileMode::kDecal,
        *input
    );
}

//! FilterDecl: blur(Float sigma_x, Float sigma_y, Int tile_mode?, ImageFilter input?)
DEF_BUILDER(blur)
{
    CHECK_ARGC(4, blur)

    POP_ARGUMENT(input, ImageFilter)
    POP_ARGUMENT(tile_mode_int, Integer)

    if (tile_mode_int)
    {
        if (*tile_mode_int < 0 || *tile_mode_int > static_cast<int>(SkTileMode::kLastTileMode))
            g_throw(RangeError, "Invalid enumeration value in argument `tile_mode` for `blur` filter");
    }

    POP_ARGUMENT_CHECKED(sigma_y, Float, blur)
    POP_ARGUMENT_CHECKED(sigma_x, Float, blur)

    return SkImageFilters::Blur(
        *sigma_x, *sigma_y,
        tile_mode_int ? static_cast<SkTileMode>(*tile_mode_int) : SkTileMode::kClamp,
        AUTO_SELECT(input)
    );
}

//! FilterDecl: arithmetic(Float k1, Float k2, Float k3, Float k4,
//!                        Bool<Int> enforce_pm_color,
//!                        ImageFilter? background,
//!                        ImageFilter? foreground)
DEF_BUILDER(arithmetic)
{
    CHECK_ARGC(7, arithmetic)

    POP_ARGUMENT(fg, ImageFilter)
    POP_ARGUMENT(bg, ImageFilter)
    POP_ARGUMENT_CHECKED(enforce_pm_color, Integer, arithmetic)

    if (enforce_pm_color < 0)
        g_throw(RangeError, "Argument `enforce_pm_color` must be a boolean integer for `arithmetic`");

    Nullable<SkScalar> k[4];
    for (int32_t i = 3; i >= 0; i--)
    {
        k[i] = st.top()->ToFloatSafe();
        st.pop();
        if (!k[i].has_value())
        {
            g_throw(Error, fmt::format("Argument `k{}` for `arithmetic` cannot be null",
                                       i + 1));
        }
    }

    return SkImageFilters::Arithmetic(*k[0], *k[1], *k[2], *k[3],
                                      *enforce_pm_color,
                                      AUTO_SELECT(bg), AUTO_SELECT(fg));
}

//! FilterDecl: compose(ImageFilter outer, ImageFilter inner)
DEF_BUILDER(compose)
{
    CHECK_ARGC(2, compose)

    POP_ARGUMENT_CHECKED(inner, ImageFilter, compose)
    POP_ARGUMENT_CHECKED(outer, ImageFilter, compose)

    return SkImageFilters::Compose(*outer, *inner);
}

//! FilterDecl: erode(Float radius_x, Float radius_y, ImageFilter? input)
DEF_BUILDER(erode)
{
    CHECK_ARGC(3, erode)

    POP_ARGUMENT(input, ImageFilter)
    POP_ARGUMENT_CHECKED(radius_y, Float, erode)
    POP_ARGUMENT_CHECKED(radius_x, Float, erode)

    return SkImageFilters::Erode(*radius_x, *radius_y, AUTO_SELECT(input));
}

//! FilterDecl: dilate(Float radius_x, Float radius_y, ImageFilter? input)
DEF_BUILDER(dilate)
{
    CHECK_ARGC(3, dilate)

    POP_ARGUMENT(input, ImageFilter)
    POP_ARGUMENT_CHECKED(radius_y, Float, erode)
    POP_ARGUMENT_CHECKED(radius_x, Float, erode)

    return SkImageFilters::Dilate(*radius_x, *radius_y, AUTO_SELECT(input));
}

//! FilterDecl: image(Image image, Int sampling, Rect? src, Rect? dst)
DEF_BUILDER(image)
{
    CHECK_ARGC(4, image)

    POP_ARGUMENT(dst, Rect)
    POP_ARGUMENT(src, Rect)
    POP_ARGUMENT_CHECKED(sampling_v, Integer, image)
    POP_ARGUMENT_CHECKED(image, Image, image)

    if (dst && src)
    {
        return SkImageFilters::Image(*image, *src, *dst,
                                     SamplingToSamplingOptions(*sampling_v));
    }
    else if (!dst && !src)
    {
        return SkImageFilters::Image(*image, SamplingToSamplingOptions(*sampling_v));
    }
    else
    {
        g_throw(Error, "Arguments `dst` and `src` must be null or non-null at the same time");
    }
}

//! FilterDecl: blend_mode(Integer mode, ImageFilter? bg, ImageFilter? fg)
DEF_BUILDER(blend_mode)
{
    CHECK_ARGC(3, blend_mode)

    POP_ARGUMENT(fg, ImageFilter)
    POP_ARGUMENT(bg, ImageFilter)
    POP_ARGUMENT_CHECKED(mode, Integer, blend_mode)

    if (*mode < 0 || *mode > static_cast<int32_t>(SkBlendMode::kLastMode))
        g_throw(RangeError, "Argument `mode` has an invalid enumeration value");

    return SkImageFilters::Blend(static_cast<SkBlendMode>(*mode),
                                 AUTO_SELECT(bg), AUTO_SELECT(fg));
}

//! FilterDecl: blender(Blender blender, ImageFilter? bg, ImageFilter? fg)
DEF_BUILDER(blender)
{
    CHECK_ARGC(3, blender)

    POP_ARGUMENT(fg, ImageFilter)
    POP_ARGUMENT(bg, ImageFilter)
    POP_ARGUMENT_CHECKED(blender, Blender, blender)

    return SkImageFilters::Blend(*blender, AUTO_SELECT(bg), AUTO_SELECT(fg));
}

//! FilterDecl: drop_shadow(Float dx, Float dy, Float sigma_x, Float sigma_y,
//!                         Color color, ImageFilter? input)
DEF_BUILDER(drop_shadow)
{
    CHECK_ARGC(6, drop_shadow)

    POP_ARGUMENT(input, ImageFilter)
    POP_ARGUMENT_CHECKED(color, Color, drop_shadow)
    POP_ARGUMENT_CHECKED(sigma_y, Float, drop_shadow)
    POP_ARGUMENT_CHECKED(sigma_x, Float, drop_shadow)
    POP_ARGUMENT_CHECKED(dy, Float, drop_shadow)
    POP_ARGUMENT_CHECKED(dx, Float, drop_shadow)

    return SkImageFilters::DropShadow(*dx, *dy, *sigma_x, *sigma_y,
                                      *color, AUTO_SELECT(input));
}

//! FilterDecl: drop_shadow_only(Float dx, Float dy, Float sigma_x, Float sigma_y,
//!                              Color color, ImageFilter? input)
DEF_BUILDER(drop_shadow_only)
{
    CHECK_ARGC(6, drop_shadow_only)

    POP_ARGUMENT(input, ImageFilter)
    POP_ARGUMENT_CHECKED(color, Color, drop_shadow_only)
    POP_ARGUMENT_CHECKED(sigma_y, Float, drop_shadow_only)
    POP_ARGUMENT_CHECKED(sigma_x, Float, drop_shadow_only)
    POP_ARGUMENT_CHECKED(dy, Float, drop_shadow_only)
    POP_ARGUMENT_CHECKED(dx, Float, drop_shadow_only)

    return SkImageFilters::DropShadowOnly(*dx, *dy, *sigma_x, *sigma_y,
                                          *color, AUTO_SELECT(input));
}

//! FilterDecl: matrix_convolution(IVector2 kernel_size, Array<Float> kernel, Float gain,
//!                                Float bias, IVector2 kernel_offset, Int tile_mode,
//!                                Bool<Int> convolve_alpha, ImageFilter? input)
DEF_BUILDER(matrix_convolution)
{
    CHECK_ARGC(8, matrix_convolution)

    POP_ARGUMENT(input, ImageFilter)
    POP_ARGUMENT_CHECKED(convolve_alpha, Integer, matrix_convolution)
    POP_ARGUMENT_CHECKED(tile_mode_v, Integer, matrix_convolution)
    POP_ARGUMENT_CHECKED(kernel_offset, IVector2, matrix_convolution)
    POP_ARGUMENT_CHECKED(bias, Float, matrix_convolution)
    POP_ARGUMENT_CHECKED(gain, Float, matrix_convolution)

    using Op = const EffectStackOperand::Ptr&;
    auto kernel = st.top()->ToMonoTypeArraySafe<SkScalar>([](Op op) -> Nullable<SkScalar> {
        return op->ToFloatSafe();
    });
    st.pop();
    THROW_IF_NULL(kernel, kernel, matrix_convolution);

    POP_ARGUMENT_CHECKED(kernel_size, IVector2, matrix_convolution)
    if (kernel_size->x() * kernel_size->y() != kernel->size())
        g_throw(Error, "Array of convolution kernel has an invalid length");

    if (*tile_mode_v < 0 || *tile_mode_v > static_cast<int32_t>(SkTileMode::kLastTileMode))
    {
        g_throw(RangeError, "Invalid enumeration value in argument `tile_mode`"
                            " for `matrix_convolution` filter");
    }

    return SkImageFilters::MatrixConvolution(SkISize::Make(kernel_size->x(), kernel_size->y()),
                                             kernel->data(),
                                             *gain,
                                             *bias,
                                             *kernel_offset,
                                             static_cast<SkTileMode>(*tile_mode_v),
                                             *convolve_alpha,
                                             AUTO_SELECT(input));
}

//! FilterDecl: color_filter(ColorFilter cf, ImageFilter? input)
DEF_BUILDER(color_filter)
{
    CHECK_ARGC(2, color_filter)

    POP_ARGUMENT(input, ImageFilter)
    POP_ARGUMENT_CHECKED(cf, ColorFilter, color_filter)
    return SkImageFilters::ColorFilter(*cf, AUTO_SELECT(input));
}

//! FilterDecl: point_lit_diffuse(Vector3 location, Color color, Float height_scale,
//!                               Float kd, ImageFilter? input)
DEF_BUILDER(point_lit_diffuse)
{
    CHECK_ARGC(5, point_lit_diffuse)

    POP_ARGUMENT(input, ImageFilter)
    POP_ARGUMENT_CHECKED(kd, Float, point_lit_diffuse)
    POP_ARGUMENT_CHECKED(height_scale, Float, point_lit_diffuse)
    POP_ARGUMENT_CHECKED(color, Color, point_lit_diffuse)
    POP_ARGUMENT_CHECKED(location, Vector3, point_lit_diffuse)

    return SkImageFilters::PointLitDiffuse(*location, *color, *height_scale,
                                           *kd, AUTO_SELECT(input));
}

//! FilterDecl: point_lit_specular(Vector3 location, Color color, Float height_scale,
//!                                Float kd, Float shininess, ImageFilter? input)
DEF_BUILDER(point_lit_specular)
{
    CHECK_ARGC(6, point_lit_specular)

    POP_ARGUMENT(input, ImageFilter)
    POP_ARGUMENT(shininess, Float)
    POP_ARGUMENT_CHECKED(kd, Float, point_lit_specular)
    POP_ARGUMENT_CHECKED(height_scale, Float, point_lit_specular)
    POP_ARGUMENT_CHECKED(color, Color, point_lit_specular)
    POP_ARGUMENT_CHECKED(location, Vector3, point_lit_specular)

    return SkImageFilters::PointLitSpecular(*location, *color, *height_scale,
                                            *kd, *shininess, AUTO_SELECT(input));
}

//! FilterDecl: distant_lit_diffuse(Vector3 direction, Color color, Float height_scale,
//!                                 Float kd, ImageFilter? input)
DEF_BUILDER(distant_lit_diffuse)
{
    CHECK_ARGC(5, distant_lit_diffuse)

    POP_ARGUMENT(input, ImageFilter)
    POP_ARGUMENT_CHECKED(kd, Float, distant_lit_diffuse)
    POP_ARGUMENT_CHECKED(height_scale, Float, distant_lit_diffuse)
    POP_ARGUMENT_CHECKED(color, Color, distant_lit_diffuse)
    POP_ARGUMENT_CHECKED(direction, Vector3, distant_lit_diffuse)

    return SkImageFilters::DistantLitDiffuse(*direction, *color, *height_scale,
                                             *kd, AUTO_SELECT(input));
}

//! FilterDecl: distant_lit_specular(Vector3 direction, Color color, Float height_scale,
//!                                  Float kd, Float shininess, ImageFilter? input)
DEF_BUILDER(distant_lit_specular)
{
    CHECK_ARGC(6, distant_lit_specular)

    POP_ARGUMENT(input, ImageFilter)
    POP_ARGUMENT(shininess, Float)
    POP_ARGUMENT_CHECKED(kd, Float, distant_lit_specular)
    POP_ARGUMENT_CHECKED(height_scale, Float, distant_lit_specular)
    POP_ARGUMENT_CHECKED(color, Color, distant_lit_specular)
    POP_ARGUMENT_CHECKED(direction, Vector3, distant_lit_specular)

    return SkImageFilters::DistantLitSpecular(*direction, *color, *height_scale,
                                              *kd, *shininess, AUTO_SELECT(input));
}

//! FilterDecl: spot_lit_diffuse(Vector3 location, Vector3 target, Float falloff_exponent,
//!                              Float cutoff_angle, Color color, Float height_scale,
//!                              Float kd, ImageFilter? input)
DEF_BUILDER(spot_lit_diffuse)
{
    CHECK_ARGC(8, spot_lit_diffuse)

    POP_ARGUMENT(input, ImageFilter)
    POP_ARGUMENT_CHECKED(kd, Float, spot_lit_diffuse)
    POP_ARGUMENT_CHECKED(height_scale, Float, spot_lit_diffuse)
    POP_ARGUMENT_CHECKED(color, Color, spot_lit_diffuse)
    POP_ARGUMENT_CHECKED(cutoff_angle, Float, spot_lit_diffuse)
    POP_ARGUMENT_CHECKED(falloff_exponent, Float, spot_lit_diffuse)
    POP_ARGUMENT_CHECKED(target, Vector3, spot_lit_diffuse)
    POP_ARGUMENT_CHECKED(location, Vector3, spot_lit_diffuse)

    return SkImageFilters::SpotLitDiffuse(*location, *target, *falloff_exponent, *cutoff_angle,
                                          *color, *height_scale, *kd, AUTO_SELECT(input));
}

//! FilterDecl: spot_lit_specular(Vector3 location, Vector3 target, Float falloff_exponent,
//!                               Float cutoff_angle, Color color, Float height_scale,
//!                               Float kd, Float shininess, ImageFilter? input)
DEF_BUILDER(spot_lit_specular)
{
    CHECK_ARGC(9, spot_lit_specular)

    POP_ARGUMENT(input, ImageFilter)
    POP_ARGUMENT_CHECKED(shininess, Float, spot_lit_specular)
    POP_ARGUMENT_CHECKED(kd, Float, spot_lit_specular)
    POP_ARGUMENT_CHECKED(height_scale, Float, spot_lit_specular)
    POP_ARGUMENT_CHECKED(color, Color, spot_lit_specular)
    POP_ARGUMENT_CHECKED(cutoff_angle, Float, spot_lit_specular)
    POP_ARGUMENT_CHECKED(falloff_exponent, Float, spot_lit_specular)
    POP_ARGUMENT_CHECKED(target, Vector3, spot_lit_specular)
    POP_ARGUMENT_CHECKED(location, Vector3, spot_lit_specular)

    return SkImageFilters::SpotLitSpecular(*location, *target, *falloff_exponent, *cutoff_angle,
                                           *color, *height_scale, *kd, *shininess, AUTO_SELECT(input));
}

#define ENTRY(N)    { #N, builder_##N }
EffectDSLParser::EffectorBuildersMap g_image_filter_builders_map = {
    ENTRY(crop),
    ENTRY(blur),
    ENTRY(compose),
    ENTRY(arithmetic),
    ENTRY(image),
    ENTRY(blend_mode),
    ENTRY(blender),
    ENTRY(drop_shadow),
    ENTRY(drop_shadow_only),
    ENTRY(matrix_convolution),
    ENTRY(color_filter),
    ENTRY(erode),
    ENTRY(dilate),
    ENTRY(point_lit_diffuse),
    ENTRY(point_lit_specular),
    ENTRY(distant_lit_diffuse),
    ENTRY(distant_lit_specular),
    ENTRY(spot_lit_diffuse),
    ENTRY(spot_lit_specular)
};
#undef ENTRY

} // namespace anonymous

v8::Local<v8::Value> CkImageFilterWrap::MakeFromDSL(v8::Local<v8::Value> dsl,
                                                    v8::Local<v8::Value> kwargs)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    if (!dsl->IsString())
        g_throw(TypeError, "Argument `dsl` must be a string");

    if (!kwargs->IsObject())
        g_throw(TypeError, "Argument `kwargs` must be an object");

    Effector effector = EffectDSLParser::Parse(isolate,
                                               v8::Local<v8::String>::Cast(dsl),
                                               v8::Local<v8::Object>::Cast(kwargs),
                                               g_image_filter_builders_map);

    return binder::NewObject<CkImageFilterWrap>(isolate,
                                                           effector.CheckImageFilter());
}


v8::Local<v8::Value> CkImageFilterWrap::serialize()
{
    sk_sp<SkData> data = GetSkObject()->serialize();
    if (!data)
        g_throw(Error, "Failed to serialize the image filter");

    void *writable_data = data->writable_data();
    auto backing_store = binder::CreateBackingStoreFromSmartPtrMemory(
            data, writable_data, data->size());

    return v8::ArrayBuffer::New(v8::Isolate::GetCurrent(), backing_store);
}

v8::Local<v8::Value> CkImageFilterWrap::Deserialize(v8::Local<v8::Value> buffer)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    auto memory = binder::GetTypedArrayMemory<v8::TypedArray>(buffer);
    if (!memory)
        g_throw(Error, "Argument `buffer` must be an allocated TypedArray");

    auto filter = SkImageFilter::Deserialize(memory->ptr, memory->byte_size);
    if (!filter)
        g_throw(Error, "Failed to deserialize the given buffer as an image filter");

    return binder::NewObject<CkImageFilterWrap>(isolate, filter);
}

v8::Local<v8::Value> CkImageFilterWrap::filterBounds(v8::Local<v8::Value> src,
                                                     v8::Local<v8::Value> ctm,
                                                     int32_t map_direction,
                                                     v8::Local<v8::Value> input_rect)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    if (map_direction != SkImageFilter::kReverse_MapDirection &&
        map_direction != SkImageFilter::kForward_MapDirection)
    {
        g_throw(RangeError, "Argument `mapDirection` has an invalid enumeration value");
    }

    SkIRect store_input_rect;
    SkIRect *input_rect_ptr = nullptr;
    if (!input_rect->IsNullOrUndefined())
    {
        store_input_rect = ExtractCkRect(isolate, input_rect).round();
        input_rect_ptr = &store_input_rect;
    }

    SkIRect result = GetSkObject()->filterBounds(
            ExtractCkRect(isolate, src).round(),
            ExtractCkMat3x3(isolate, ctm),
            static_cast<SkImageFilter::MapDirection>(map_direction),
            input_rect_ptr
    );

    return NewCkRect(isolate, SkRect::Make(result));
}

bool CkImageFilterWrap::canComputeFastBounds()
{
    return GetSkObject()->canComputeFastBounds();
}

v8::Local<v8::Value> CkImageFilterWrap::computeFastBounds(v8::Local<v8::Value> bounds)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return NewCkRect(isolate, GetSkObject()->computeFastBounds(
            ExtractCkRect(isolate, bounds)));
}

v8::Local<v8::Value> CkImageFilterWrap::makeWithLocalMatrix(v8::Local<v8::Value> matrix)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    sk_sp<SkImageFilter> result = GetSkObject()->makeWithLocalMatrix(
            ExtractCkMat3x3(isolate, matrix));
    if (!result)
        return v8::Null(isolate);
    return binder::NewObject<CkImageFilterWrap>(isolate, std::move(result));
}

GALLIUM_BINDINGS_GLAMOR_NS_END
