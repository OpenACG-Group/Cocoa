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
#include "include/effects/Sk1DPathEffect.h"
#include "include/effects/Sk2DPathEffect.h"
#include "include/effects/SkTrimPathEffect.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/effects/SkDiscretePathEffect.h"
#include "include/effects/SkCornerPathEffect.h"

#include "Gallium/bindings/renderer/PathEffect.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

#define CREATE_OR_THROW(value)                                                          \
    if (!value) {                                                                       \
        return ffi::Fail(ffi::kErr, "failed to create path effect: invalid arguments"); \
    }                                                                                   \
    return ffi::JSObject::New<PathEffect>(v8::Isolate::GetCurrent(), value);

ffi::RetLocal<v8::Value> PathEffect::MakeSum(const ffi::Class<PathEffect>& first,
                                             const ffi::Class<PathEffect>& second)
{
    auto pe = SkPathEffect::MakeSum(first->GetSkPathEffect(), second->GetSkPathEffect());
    CREATE_OR_THROW(pe)
}

ffi::RetLocal<v8::Value> PathEffect::MakeCompose(const ffi::Class<PathEffect>& outer,
                                                 const ffi::Class<PathEffect>& inner)
{
    auto pe = SkPathEffect::MakeCompose(outer->GetSkPathEffect(), inner->GetSkPathEffect());
    CREATE_OR_THROW(pe)
}

ffi::RetLocal<v8::Value> PathEffect::Make1D(const ffi::Class<Path>& path, float advance, float phase,
                                            const ffi::Enum<SkPath1DPathEffect::Style>& style)
{
    auto pe = SkPath1DPathEffect::Make(path->GetSkPath(), advance, phase, *style);
    CREATE_OR_THROW(pe)
}

ffi::RetLocal<v8::Value> PathEffect::MakeLine2D(float width, const Mat3x3Adapter& matrix)
{
    auto pe = SkLine2DPathEffect::Make(width, *matrix);
    CREATE_OR_THROW(pe)
}

ffi::RetLocal<v8::Value> PathEffect::MakePath2D(const Mat3x3Adapter& matrix, const ffi::Class<Path>& path)
{
    auto pe = SkPath2DPathEffect::Make(*matrix, path->GetSkPath());
    CREATE_OR_THROW(pe)
}

ffi::RetLocal<v8::Value> PathEffect::MakeCorner(float radius)
{
    auto pe = SkCornerPathEffect::Make(radius);
    CREATE_OR_THROW(pe)
}

ffi::RetLocal<v8::Value> PathEffect::MakeTrim(float start_t, float stop_t, bool inverted)
{
    auto pe = SkTrimPathEffect::Make(start_t, stop_t, inverted ? SkTrimPathEffect::Mode::kInverted
                                                               : SkTrimPathEffect::Mode::kNormal);
    CREATE_OR_THROW(pe)
}

ffi::RetLocal<v8::Value> PathEffect::MakeDiscrete(float seg_length, float dev, uint32_t seed_assist)
{
    auto pe = SkDiscretePathEffect::Make(seg_length, dev, seed_assist);
    CREATE_OR_THROW(pe)
}

ffi::RetLocal<v8::Value> PathEffect::MakeDash(const ffi::Mem<float>& intervals, float phase)
{
    auto pe = SkDashPathEffect::Make(intervals.Address(), static_cast<int>(intervals.Size()), phase);
    CREATE_OR_THROW(pe)
}

sk_sp<SkData> PathEffect::OnSerializeImpl(SkSerialProcs *procs)
{
    return effect_->serialize(procs);
}

GALLIUM_BINDINGS_RENDERER_NS_END
