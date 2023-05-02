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

#include "include/core/SkShader.h"
#include "include/effects/SkPerlinNoiseShader.h"
#include "include/effects/SkGradientShader.h"

#include "Gallium/bindings/glamor/Exports.h"
#include "Gallium/bindings/glamor/EffectDSLParser.h"
#include "Gallium/bindings/glamor/EffectDSLBuilderHelperMacros.h"
#include "Gallium/bindings/glamor/CkMatrixWrap.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

namespace {

template<typename T>
using Nullable = EffectStackOperand::Nullable<T>;

//! ShaderDecl: Empty()
DEF_BUILDER(empty)
{
    CHECK_ARGC(0, empty)
    return SkShaders::Empty();
}

//! ShaderDecl: color(Color color)
DEF_BUILDER(color)
{
    CHECK_ARGC(1, color)
    POP_ARGUMENT_CHECKED(color, Color, color)
    return SkShaders::Color(*color);
}

//! ShaderDecl: fractal_noise(Float baseFreqX, Float baseFreqY, Int numOctaves,
//!                           Float seed, ISize? tileSize)
DEF_BUILDER(fractal_noise)
{
    CHECK_ARGC(5, fractal_noise)

    POP_ARGUMENT(tileSize, ISize)
    POP_ARGUMENT_CHECKED(seed, Float, fractal_noise)
    POP_ARGUMENT_CHECKED(numOctaves, Integer, fractal_noise)
    POP_ARGUMENT_CHECKED(baseFreqY, Float, fractal_noise)
    POP_ARGUMENT_CHECKED(baseFreqX, Float, fractal_noise)

    return SkPerlinNoiseShader::MakeFractalNoise(*baseFreqX, *baseFreqY, *numOctaves,
                                                 *seed, AUTO_SELECT_PTR(tileSize));
}

//! ShaderDecl: turbulence(Float baseFreqX, Float baseFreqY, Int numOctaves,
//!                        Float seed, ISize? tileSize)
DEF_BUILDER(turbulence)
{
    CHECK_ARGC(5, turbulence)

    POP_ARGUMENT(tileSize, ISize)
    POP_ARGUMENT_CHECKED(seed, Float, turbulence)
    POP_ARGUMENT_CHECKED(numOctaves, Integer, turbulence)
    POP_ARGUMENT_CHECKED(baseFreqY, Float, turbulence)
    POP_ARGUMENT_CHECKED(baseFreqX, Float, turbulence)

    return SkPerlinNoiseShader::MakeTurbulence(*baseFreqX, *baseFreqY, *numOctaves,
                                               *seed, AUTO_SELECT_PTR(tileSize));
}

//! ShaderDecl: gradient_linear(Vector2 p1, Vector2 p2, Color[] colors, Float[]? pos, Int tile_mode)
DEF_BUILDER(gradient_linear)
{
    CHECK_ARGC(5, gradient_linear)

    POP_ARGUMENT_CHECKED(tile_mode, Integer, gradient_linear)
    if (*tile_mode < 0 || *tile_mode > static_cast<int32_t>(SkTileMode::kLastTileMode))
        g_throw(RangeError, "gradient_linear: Invalid enumeration value for `tile_mode`");

    auto pos = st.top()->ToMonoTypeArraySafe<SkScalar>([](const EffectStackOperand::Ptr& op) {
        return op->ToFloatSafe();
    });
    st.pop();

    auto colors = st.top()->ToMonoTypeArraySafe<SkColor>([](const EffectStackOperand::Ptr& op) {
        return op->ToColorSafe();
    });
    st.pop();

    if (!colors)
        g_throw(Error, "gradient_linear: Argument `colors` cannot be null");

    if (pos && pos->size() != colors->size())
        g_throw(Error, "gradient_linear: Lengths of `pos` and `colors` do not match");

    POP_ARGUMENT_CHECKED(p2, Vector2, gradient_linear)
    POP_ARGUMENT_CHECKED(p1, Vector2, gradient_linear)

    SkPoint pts[2] = {*p1, *p2};
    return SkGradientShader::MakeLinear(pts, colors->data(), pos ? pos->data() : nullptr,
                                        static_cast<int32_t>(colors->size()),
                                        static_cast<SkTileMode>(*tile_mode));
}

//! ShaderDecl: gradient_radial(Vector2 center, Float radius, Color[] colors, Float[]? pos, Int tile_mode)
DEF_BUILDER(gradient_radial)
{
    CHECK_ARGC(5, gradient_radial)

    POP_ARGUMENT_CHECKED(tile_mode, Integer, gradient_radial)
    if (*tile_mode < 0 || *tile_mode > static_cast<int32_t>(SkTileMode::kLastTileMode))
        g_throw(RangeError, "gradient_linear: Invalid enumeration value for `tile_mode`");

    auto pos = st.top()->ToMonoTypeArraySafe<SkScalar>([](const EffectStackOperand::Ptr& op) {
        return op->ToFloatSafe();
    });
    st.pop();

    auto colors = st.top()->ToMonoTypeArraySafe<SkColor>([](const EffectStackOperand::Ptr& op) {
        return op->ToColorSafe();
    });
    st.pop();

    if (!colors)
        g_throw(Error, "gradient_radial: Argument `colors` cannot be null");

    if (colors->size() < 2)
        g_throw(Error, "gradient_radial: At least 2 colors should be provided by `colors`");

    if (pos && pos->size() != colors->size())
        g_throw(Error, "gradient_linear: Lengths of `pos` and `colors` do not match");

    POP_ARGUMENT_CHECKED(radius, Float, gradient_radial)
    POP_ARGUMENT_CHECKED(center, Vector2, gradient_radial)

    return SkGradientShader::MakeRadial(*center, *radius, colors->data(), pos ? pos->data() : nullptr,
                                        static_cast<int32_t>(colors->size()),
                                        static_cast<SkTileMode>(*tile_mode));
}

//! ShaderDecl: gradient_two_point_conical(Vector2 start, Float start_radius,
//!                                        Vector2 end, Float end_radius,
//!                                        Color[] colors, Float[]? pos, Int tile_mode)
DEF_BUILDER(gradient_two_point_conical)
{
    CHECK_ARGC(7, gradient_two_point_conical)

    POP_ARGUMENT_CHECKED(tile_mode, Integer, gradient_two_point_conical)
    if (*tile_mode < 0 || *tile_mode > static_cast<int32_t>(SkTileMode::kLastTileMode))
        g_throw(RangeError, "gradient_two_point_conical: Invalid enumeration value for `tile_mode`");

    auto pos = st.top()->ToMonoTypeArraySafe<SkScalar>([](const EffectStackOperand::Ptr& op) {
        return op->ToFloatSafe();
    });
    st.pop();

    auto colors = st.top()->ToMonoTypeArraySafe<SkColor>([](const EffectStackOperand::Ptr& op) {
        return op->ToColorSafe();
    });
    st.pop();

    if (!colors)
        g_throw(Error, "gradient_two_point_conical: Argument `colors` cannot be null");
    if (pos && pos->size() != colors->size())
        g_throw(Error, "gradient_two_point_conical: Lengths of `pos` and `colors` do not match");

    POP_ARGUMENT_CHECKED(end_radius, Float, gradient_two_point_conical)
    POP_ARGUMENT_CHECKED(end, Vector2, gradient_two_point_conical)
    POP_ARGUMENT_CHECKED(start_radius, Float, gradient_two_point_conical)
    POP_ARGUMENT_CHECKED(start, Vector2, gradient_two_point_conical)

    return SkGradientShader::MakeTwoPointConical(*start, *start_radius, *end, *end_radius, colors->data(),
                                                 pos ? pos->data() : nullptr,
                                                 static_cast<int32_t>(colors->size()),
                                                 static_cast<SkTileMode>(*tile_mode));
}

//! TSDecl: gradient_sweep(Float cx, Float cy, Color[] colors, Float[]? pos, Int tile_mode,
//!                        Float start_angle, Float end_angle)
DEF_BUILDER(gradient_sweep)
{
    CHECK_ARGC(7, gradient_sweep)

    POP_ARGUMENT_CHECKED(end_angle, Float, gradient_sweep)
    POP_ARGUMENT_CHECKED(start_angle, Float, gradient_sweep)
    POP_ARGUMENT_CHECKED(tile_mode, Integer, gradient_sweep)
    if (*tile_mode < 0 || *tile_mode > static_cast<int32_t>(SkTileMode::kLastTileMode))
        g_throw(RangeError, "gradient_two_point_conical: Invalid enumeration value for `tile_mode`");

    auto pos = st.top()->ToMonoTypeArraySafe<SkScalar>([](const EffectStackOperand::Ptr& op) {
        return op->ToFloatSafe();
    });
    st.pop();

    auto colors = st.top()->ToMonoTypeArraySafe<SkColor>([](const EffectStackOperand::Ptr& op) {
        return op->ToColorSafe();
    });
    st.pop();

    if (!colors)
        g_throw(Error, "gradient_two_point_conical: Argument `colors` cannot be null");
    if (pos && pos->size() != colors->size())
        g_throw(Error, "gradient_two_point_conical: Lengths of `pos` and `colors` do not match");

    POP_ARGUMENT_CHECKED(cy, Float, gradient_sweep)
    POP_ARGUMENT_CHECKED(cx, Float, gradient_sweep)

    return SkGradientShader::MakeSweep(*cx, *cy, colors->data(), pos ? pos->data() : nullptr,
                                       static_cast<int32_t>(colors->size()),
                                       static_cast<SkTileMode>(*tile_mode),
                                       *start_angle, *end_angle, 0, nullptr);
}

#define ENTRY(N)    { #N, builder_##N }
EffectDSLParser::EffectorBuildersMap g_shader_builders_map = {
    ENTRY(empty),
    ENTRY(color),
    ENTRY(fractal_noise),
    ENTRY(turbulence),
    ENTRY(gradient_linear),
    ENTRY(gradient_radial),
    ENTRY(gradient_two_point_conical),
    ENTRY(gradient_sweep)
};
#undef ENTRY

} // namespace anonymous

v8::Local<v8::Value> CkShaderWrap::MakeFromDSL(v8::Local<v8::Value> dsl,
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
                                               g_shader_builders_map);

    return binder::Class<CkShaderWrap>::create_object(
            isolate, effector.CheckShader());
}

v8::Local<v8::Value> CkShaderWrap::makeWithLocalMatrix(v8::Local<v8::Value> matrix)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto *m = binder::Class<CkMatrix>::unwrap_object(isolate, matrix);
    if (!m)
        g_throw(TypeError, "Argument `matrix` must be an instance of `CkMatrix`");

    sk_sp<SkShader> result = GetSkObject()->makeWithLocalMatrix(m->GetMatrix());
    if (!result)
        g_throw(Error, "Failed to make shader with local matrix");

    return binder::Class<CkShaderWrap>::create_object(isolate, result);
}

v8::Local<v8::Value> CkShaderWrap::makeWithColorFilter(v8::Local<v8::Value> filter)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto *f = binder::Class<CkColorFilterWrap>::unwrap_object(isolate, filter);
    if (!f)
        g_throw(TypeError, "Argument `filter` must be an instance of `CkFilter`");

    sk_sp<SkShader> result = GetSkObject()->makeWithColorFilter(f->GetSkObject());
    if (!result)
        g_throw(Error, "Failed to make shader with color filter");

    return binder::Class<CkShaderWrap>::create_object(isolate, result);
}

GALLIUM_BINDINGS_GLAMOR_NS_END
