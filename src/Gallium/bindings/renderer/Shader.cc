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
#include "include/effects/SkPerlinNoiseShader.h"
#include "include/effects/SkGradientShader.h"

#include "Gallium/bindings/renderer/Shader.h"
#include "Gallium/bindings/renderer/Image.h"
#include "Gallium/bindings/renderer/ColorFilter.h"
#include "Gallium/bindings/renderer/Blender.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

#define MAKE(func, ...) \
    sk_sp<SkShader> result = func(__VA_ARGS__);                                     \
    if (!result) {                                                                  \
        return ffi::Fail(ffi::kErr, "failed to create shader: invalid arguments");  \
    }                                                                               \
    return ffi::JSObject::New<Shader>(v8::Isolate::GetCurrent(), result);

#define OPTMAT(x) ((x) ? &(**(x)) : nullptr)

SkGradientShader::Interpolation GradientInterpolation::Unwrap() const
{
    SkGradientShader::Interpolation interp;
    if (in_premul)
    {
        interp.fInPremul = *in_premul ? SkGradientShader::Interpolation::InPremul::kYes
                                      : SkGradientShader::Interpolation::InPremul::kNo;
    }
    if (color_space)
        interp.fColorSpace = **color_space;
    if (hue_method)
        interp.fHueMethod = **hue_method;
    return interp;
}

ffi::RetLocal<v8::Value> Shader::Empty()
{
    MAKE(SkShaders::Empty)
}

ffi::RetLocal<v8::Value> Shader::Color(Color4fQuadruple color, const ffi::Class<ColorSpace> &cs)
{
    auto [r, g, b, a] = color;
    MAKE(SkShaders::Color, {r, g, b, a}, cs->GetColorSpace())
}

ffi::RetLocal<v8::Value> Shader::Blend(const ffi::Enum<SkBlendMode> &mode,
                                       const ffi::Class<Shader> &dst,
                                       const ffi::Class<Shader> &src)
{
    MAKE(SkShaders::Blend, *mode, dst->shader_, src->shader_)
}

ffi::RetLocal<v8::Value> Shader::Blender(const ffi::Class<renderer::Blender> &blender,
                                         const ffi::Class<Shader> &dst,
                                         const ffi::Class<Shader> &src)
{
    MAKE(SkShaders::Blend, blender->GetSkBlender(), dst->shader_, src->shader_)
}

ffi::RetLocal<v8::Value> Shader::CoordClamp(const ffi::Class<Shader> &shader,
                                            const RectAdapter &subset)
{
    MAKE(SkShaders::CoordClamp, shader->shader_, *subset)
}

ffi::RetLocal<v8::Value> Shader::Image(const ffi::Class<renderer::Image> &image,
                                       const ffi::Enum<SkTileMode> &tmx,
                                       const ffi::Enum<SkTileMode> &tmy,
                                       const SamplingOptionsAdapter &options,
                                       const ffi::Opt<Mat3x3Adapter> &local_matrix)
{
    MAKE(SkShaders::Image, image->GetSkImage(), *tmx, *tmy, *options, OPTMAT(local_matrix))
}

ffi::RetLocal<v8::Value> Shader::RawImage(const ffi::Class<renderer::Image> &image,
                                          const ffi::Enum<SkTileMode> &tmx,
                                          const ffi::Enum<SkTileMode> &tmy,
                                          const SamplingOptionsAdapter &options,
                                          const ffi::Opt<Mat3x3Adapter> &local_matrix)
{
    MAKE(SkShaders::RawImage, image->GetSkImage(), *tmx, *tmy, *options, OPTMAT(local_matrix))
}

namespace {

ffi::Ret<std::vector<SkColor4f>> extract_colors_arr(v8::Isolate *isolate,
                                                    v8::Local<v8::Array> colors)
{
    if (colors->Length() == 0)
        return ffi::Fail(ffi::kErr, "no colors are provided");
    uint32_t size = colors->Length();

    std::vector<SkColor4f> result;
    result.reserve(size);

    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    for (uint32_t i = 0; i < size; i++)
    {
        v8::Local<v8::Value> element;
        if (!colors->Get(ctx, i).ToLocal(&element))
            return ffi::FreePropagate();
        auto optional = ffi::Cast<Color4fQuadruple>::From(isolate, element);
        if (!optional)
            return ffi::Fail(ffi::kErr, "failed to interpret the provided colors");

        auto [r, g, b, a] = *optional;
        result.push_back({ r, g, b, a });
    }

    return result;
}

#define EXTRACT_COLORS_VEC \
    v8::Isolate *isolate = v8::Isolate::GetCurrent();                                           \
    ffi::Ret<std::vector<SkColor4f>> colors_vec_wrap = extract_colors_arr(isolate, colors);     \
    if (colors_vec_wrap.HasError())                                                             \
        return colors_vec_wrap.GetError();                                                      \
    std::vector<SkColor4f>& colors_vec = colors_vec_wrap.Extract();                             \
    if (colors_vec.size() != pos.size())                                                        \
        return ffi::Fail(ffi::kErr, "number of colors does not match with the number of positions");

} // namespace anonymous

ffi::RetLocal<v8::Value> Shader::LinearGradient(const std::tuple<float, float, float, float> &pts,
                                                v8::Local<v8::Array> colors,
                                                const ffi::Class<ColorSpace> &color_space,
                                                const std::vector<float> &pos,
                                                const ffi::Enum<SkTileMode> &mode,
                                                const ffi::IFace<GradientInterpolation> &interpolation,
                                                const ffi::Opt<Mat3x3Adapter> &local_matrix)
{
    EXTRACT_COLORS_VEC
    SkPoint pts_arr[] = { { std::get<0>(pts), std::get<1>(pts) },
                          { std::get<2>(pts), std::get<3>(pts) } };
    MAKE(SkGradientShader::MakeLinear, pts_arr, colors_vec.data(), color_space->GetColorSpace(),
         pos.data(), pos.size(), *mode, interpolation->Unwrap(), OPTMAT(local_matrix))
}

ffi::RetLocal<v8::Value> Shader::RadialGradient(const std::tuple<float, float> &center,
                                                float radius, v8::Local<v8::Array> colors,
                                                const ffi::Class<ColorSpace> &color_space,
                                                const std::vector<float> &pos,
                                                const ffi::Enum<SkTileMode> &mode,
                                                const ffi::IFace<GradientInterpolation> &interpolation,
                                                const ffi::Opt<Mat3x3Adapter> &local_matrix)
{
    EXTRACT_COLORS_VEC
    SkPoint center_p{ std::get<0>(center), std::get<1>(center) };
    MAKE(SkGradientShader::MakeRadial, center_p, radius, colors_vec.data(), color_space->GetColorSpace(),
         pos.data(), pos.size(), *mode, interpolation->Unwrap(), OPTMAT(local_matrix))
}

ffi::RetLocal<v8::Value> Shader::TwoPointConicalGradient(const std::tuple<float, float> &start,
                                                         float start_radius,
                                                         const std::tuple<float, float> &end,
                                                         float end_radius,
                                                         v8::Local<v8::Array> colors,
                                                         const ffi::Class<ColorSpace> &color_space,
                                                         const std::vector<float> &pos,
                                                         const ffi::Enum<SkTileMode> &mode,
                                                         const ffi::IFace<GradientInterpolation> &interpolation,
                                                         const ffi::Opt<Mat3x3Adapter> &local_matrix)
{
    SkPoint start_p{ std::get<0>(start), std::get<1>(start) };
    SkPoint end_p{ std::get<0>(end), std::get<1>(end) };
    EXTRACT_COLORS_VEC
    MAKE(SkGradientShader::MakeTwoPointConical, start_p, start_radius, end_p, end_radius,
         colors_vec.data(), color_space->GetColorSpace(), pos.data(), pos.size(),
         *mode, interpolation->Unwrap(), OPTMAT(local_matrix))
}

ffi::RetLocal<v8::Value> Shader::SweepGradient(const std::tuple<float, float> &center,
                                               float start_angle, float end_angle,
                                               v8::Local<v8::Array> colors,
                                               const ffi::Class<ColorSpace> &color_space,
                                               const std::vector<float> &pos,
                                               const ffi::Enum<SkTileMode> &mode,
                                               const ffi::IFace<GradientInterpolation> &interpolation,
                                               const ffi::Opt<Mat3x3Adapter> &local_matrix)
{
    EXTRACT_COLORS_VEC
    MAKE(SkGradientShader::MakeSweep, std::get<0>(center), std::get<1>(center), colors_vec.data(),
         color_space->GetColorSpace(), pos.data(), pos.size(), *mode, start_angle, end_angle,
         interpolation->Unwrap(), OPTMAT(local_matrix))
}

ffi::RetLocal<v8::Value> Shader::FractalNoise(float base_freq_x, float base_freq_y,
                                              int num_octaves, float seed,
                                              const ffi::Opt<std::tuple<float, float>> &tile_size)
{
    SkISize *tile_size_ptr = nullptr;
    SkISize tile_size_store{};
    if (tile_size)
    {
        auto [w, h] = *tile_size;
        tile_size_store = SkSize::Make(w, h).toRound();
        tile_size_ptr = &tile_size_store;
    }
    MAKE(SkShaders::MakeFractalNoise, base_freq_x, base_freq_y, num_octaves, seed, tile_size_ptr)
}

ffi::RetLocal<v8::Value> Shader::Turbulence(float base_freq_x, float base_freq_y,
                                            int num_octaves, float seed,
                                            const ffi::Opt<std::tuple<float, float>> &tile_size)
{
    SkISize *tile_size_ptr = nullptr;
    SkISize tile_size_store{};
    if (tile_size)
    {
        auto [w, h] = *tile_size;
        tile_size_store = SkSize::Make(w, h).toRound();
        tile_size_ptr = &tile_size_store;
    }
    MAKE(SkShaders::MakeTurbulence, base_freq_x, base_freq_y, num_octaves, seed, tile_size_ptr)
}

ffi::RetLocal<v8::Value> Shader::makeWithLocalMatrix(const Mat3x3Adapter &matrix)
{
    sk_sp<SkShader> res = shader_->makeWithLocalMatrix(*matrix);
    if (!res)
        return ffi::Fail(ffi::kErr, "failed to create a shader with specified local matrix");
    return ffi::JSObject::New<Shader>(v8::Isolate::GetCurrent(), res);
}

ffi::RetLocal<v8::Value> Shader::makeWithWorkingColorSpace(const ffi::Class<ColorSpace> &cs)
{
    sk_sp<SkShader> res = shader_->makeWithWorkingColorSpace(cs->GetColorSpace());
    if (!res)
        return ffi::Fail(ffi::kErr, "failed to create a shader with specified colorspace");
    return ffi::JSObject::New<Shader>(v8::Isolate::GetCurrent(), res);
}

ffi::RetLocal<v8::Value> Shader::makeWithColorFilter(const ffi::Class<ColorFilter> &cf)
{
    sk_sp<SkShader> res = shader_->makeWithColorFilter(cf->GetSkColorFilter());
    if (!res)
        return ffi::Fail(ffi::kErr, "failed to create a shader with specified color filter");
    return ffi::JSObject::New<Shader>(v8::Isolate::GetCurrent(), res);
}

sk_sp<SkData> Shader::OnSerializeImpl(SkSerialProcs *procs)
{
    return shader_->serialize(procs);
}

ffi::Ret<void> Shader::dispose()
{
    NotifyDisposeState(DisposeState::kDisposed);
    shader_.reset();
    return {};
}

GALLIUM_BINDINGS_RENDERER_NS_END
