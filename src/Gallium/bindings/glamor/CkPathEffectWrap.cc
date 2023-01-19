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

#include "include/core/SkPathEffect.h"
#include "include/effects/Sk1DPathEffect.h"
#include "include/effects/Sk2DPathEffect.h"
#include "include/effects/SkOpPathEffect.h"
#include "include/effects/SkTrimPathEffect.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/effects/SkCornerPathEffect.h"
#include "include/effects/SkDiscretePathEffect.h"
#include "include/effects/SkStrokeAndFillPathEffect.h"

#include "Gallium/bindings/glamor/CkPathEffectWrap.h"
#include "Gallium/bindings/glamor/EffectDSLParser.h"
#include "Gallium/bindings/glamor/EffectDSLBuilderHelperMacros.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

namespace {

template<typename T>
using Nullable = EffectStackOperand::Nullable<T>;

//! EffectorDecl: sum(PathEffect first, PathEffect second)
DEF_BUILDER(sum)
{
    CHECK_ARGC(2, sum)
    POP_ARGUMENT_CHECKED(second, PathEffect, sum)
    POP_ARGUMENT_CHECKED(first, PathEffect, sum)
    return SkPathEffect::MakeSum(*first, *second);
}

//! EffectorDecl: compose(PathEffect outer, PathEffect inner)
DEF_BUILDER(compose)
{
    CHECK_ARGC(2, compose)
    POP_ARGUMENT_CHECKED(inner, PathEffect, compose)
    POP_ARGUMENT_CHECKED(outer, PathEffect, compose)
    return SkPathEffect::MakeCompose(*outer, *inner);
}

//! EffectorDecl: path1d(Path path, Float advance, Float phase, Integer style)
DEF_BUILDER(path1d)
{
    CHECK_ARGC(4, path1d)
    POP_ARGUMENT_CHECKED(style, Integer, path1d)
    if (*style < 0 || *style > static_cast<int32_t>(SkPath1DPathEffect::kLastEnum_Style))
        g_throw(RangeError, "path1d: Invalid enumeration value for `style`");

    POP_ARGUMENT_CHECKED(phase, Float, path1d)
    POP_ARGUMENT_CHECKED(advance, Float, path1d)
    POP_ARGUMENT_CHECKED(path, Path, path1)

    return SkPath1DPathEffect::Make(**path, *advance, *phase,
                                    static_cast<SkPath1DPathEffect::Style>(*style));
}

//! EffectorDecl: path2d(Matrix matrix, Path path)
DEF_BUILDER(path2d)
{
    CHECK_ARGC(2, path2d)
    POP_ARGUMENT_CHECKED(path, Path, path2d)
    POP_ARGUMENT_CHECKED(matrix, Matrix, path2d)
    return SkPath2DPathEffect::Make(**matrix, **path);
}

//! EffectorDecl: line2d(Float width, Matrix matrix)
DEF_BUILDER(line2d)
{
    CHECK_ARGC(2, line2d)
    POP_ARGUMENT_CHECKED(matrix, Matrix, line2d)
    POP_ARGUMENT_CHECKED(width, Float, line2d)
    return SkLine2DPathEffect::Make(*width, **matrix);
}

//! EffectorDecl: trim(Float start_t, Float end_t, Int mode)
DEF_BUILDER(trim)
{
    CHECK_ARGC(3, trim)
    POP_ARGUMENT_CHECKED(mode, Integer, trim)
    if (*mode < 0 || *mode > static_cast<int32_t>(SkTrimPathEffect::Mode::kInverted))
        g_throw(RangeError, "trim: Invalid enumeration value for `mode`");
    POP_ARGUMENT_CHECKED(end_t, Float, trim)
    POP_ARGUMENT_CHECKED(start_t, Float, trim)
    return SkTrimPathEffect::Make(*start_t, *end_t, static_cast<SkTrimPathEffect::Mode>(*mode));
}

//! EffectorDecl: dash(Float[] intervals, Float phase)
DEF_BUILDER(dash)
{
    CHECK_ARGC(2, dash)
    POP_ARGUMENT_CHECKED(phase, Float, dash)
    
    auto intervals = st.top()->ToMonoTypeArraySafe<SkScalar>([](const EffectStackOperand::Ptr& op) {
        return op->ToFloatSafe();
    });
    st.pop();
    if (!intervals)
        g_throw(Error, "dash: argument `intervals` cannot be null");
    return SkDashPathEffect::Make(intervals->data(),
                                  static_cast<int32_t>(intervals->size()),
                                  *phase);
}

//! EffectorDecl: corner(Float radius)
DEF_BUILDER(corner)
{
    CHECK_ARGC(1, corner)
    POP_ARGUMENT_CHECKED(radius, Float, corner)
    return SkCornerPathEffect::Make(*radius);
}

//! EffectorDecl: discrete(Float seg_length, Float dev, Int seed_assist)
DEF_BUILDER(discrete)
{
    CHECK_ARGC(3, discrete)
    POP_ARGUMENT_CHECKED(seed_assist, Integer, discrete)
    POP_ARGUMENT_CHECKED(dev, Float, discrete)
    POP_ARGUMENT_CHECKED(seg_length, Float, discrete)
    return SkDiscretePathEffect::Make(*seg_length, *dev, *seed_assist);
}

//! EffectorDecl: stroke_and_fill()
DEF_BUILDER(stroke_and_fill)
{
    CHECK_ARGC(0, stroke_and_fill)
    return SkStrokeAndFillPathEffect::Make();
}


#define ENTRY(N)    { #N, builder_##N }
EffectDSLParser::EffectorBuildersMap g_path_effect_builders_map = {
    ENTRY(sum),
    ENTRY(compose),
    ENTRY(path1d),
    ENTRY(path2d),
    ENTRY(line2d),
    ENTRY(trim),
    ENTRY(dash),
    ENTRY(corner),
    ENTRY(discrete),
    ENTRY(stroke_and_fill)
};
#undef ENTRY

} // namespace anonymous

v8::Local<v8::Value> CkPathEffect::MakeFromDSL(v8::Local<v8::Value> dsl,
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
                                               g_path_effect_builders_map);

    return binder::Class<CkPathEffect>::create_object(
            isolate, effector.CheckPathEffect());
}

GALLIUM_BINDINGS_GLAMOR_NS_END
