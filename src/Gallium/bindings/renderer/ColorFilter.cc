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
#include "include/effects/SkHighContrastFilter.h"
#include "include/effects/SkLumaColorFilter.h"
#include "include/effects/SkOverdrawColorFilter.h"

#include "Gallium/bindings/renderer/ColorFilter.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

ffi::RetLocal<v8::Value> ColorFilter::Compose(const ffi::Class<ColorFilter>& outer,
                                              const ffi::Class<ColorFilter>& inner)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<ColorFilter>(
            isolate, SkColorFilters::Compose(outer->filter_, inner->filter_));
}

ffi::RetLocal<v8::Value> ColorFilter::Blend(Color4fQuadruple color,
                                            const ffi::Opt<ffi::Class<ColorSpace>>& colorspace,
                                            const ffi::Enum<SkBlendMode>& mode)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto [r, g, b, a] = color;
    return ffi::JSObject::New<ColorFilter>(isolate, SkColorFilters::Blend(
            {r, g, b, a}, colorspace ? (*colorspace)->GetColorSpace() : nullptr, *mode));
}

ffi::RetLocal<v8::Value> ColorFilter::Matrix(const ffi::Class<ColorMatrix>& matrix)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<ColorFilter>(
            isolate, SkColorFilters::Matrix(matrix->GetSkColorMatrix()));
}

ffi::RetLocal<v8::Value> ColorFilter::HSLAMatrix(const ffi::Class<ColorMatrix>& matrix)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<ColorFilter>(
            isolate, SkColorFilters::HSLAMatrix(matrix->GetSkColorMatrix()));
}

ffi::RetLocal<v8::Value> ColorFilter::LinearToSRGBGamma()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<ColorFilter>(isolate, SkColorFilters::LinearToSRGBGamma());
}

ffi::RetLocal<v8::Value> ColorFilter::SRGBToLinearGamma()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<ColorFilter>(isolate, SkColorFilters::SRGBToLinearGamma());
}

ffi::RetLocal<v8::Value> ColorFilter::Lerp(float t,
                                           const ffi::Class<ColorFilter>& dst,
                                           const ffi::Class<ColorFilter>& src)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<ColorFilter>(
            isolate, SkColorFilters::Lerp(t, dst->filter_, src->filter_));
}

ffi::RetLocal<v8::Value> ColorFilter::Table(const ffi::Mem<uint8_t>& table)
{
    if (table.Size() < 256)
        return ffi::Fail(ffi::kErr, "size of table is insufficient (less than 256)");
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<ColorFilter>(isolate, SkColorFilters::Table(table.Address()));
}

ffi::RetLocal<v8::Value> ColorFilter::TableARGB(const ffi::Mem<uint8_t>& table_a,
                                                const ffi::Mem<uint8_t>& table_r,
                                                const ffi::Mem<uint8_t>& table_g,
                                                const ffi::Mem<uint8_t>& table_b)
{
    if (table_a.Size() < 256 || table_r.Size() < 256 ||
        table_g.Size() < 256 || table_b.Size() < 256)
    {
        return ffi::Fail(ffi::kErr, "size of table is insufficient (less than 256)");
    }
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<ColorFilter>(isolate, SkColorFilters::TableARGB(
            table_a.Address(), table_r.Address(), table_g.Address(), table_b.Address()));
}

ffi::RetLocal<v8::Value> ColorFilter::Lighting(uint32_t mul, uint32_t add)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<ColorFilter>(isolate, SkColorFilters::Lighting(mul, add));
}

ffi::RetLocal<v8::Value> ColorFilter::HighContrast(const ffi::IFace<HighContrastConfig>& config)
{
    SkHighContrastConfig sk_config{config->grayscale, *config->invert_style, config->contrast};
    if (!sk_config.isValid())
        return ffi::Fail(ffi::kErr, "invalid high contrast config, values are out of range");
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<ColorFilter>(isolate, SkHighContrastFilter::Make(sk_config));
}

ffi::RetLocal<v8::Value> ColorFilter::Luma()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<ColorFilter>(isolate, SkLumaColorFilter::Make());
}

ffi::RetLocal<v8::Value> ColorFilter::Overdraw(const std::vector<uint32_t>& colors)
{
    if (colors.size() != 6)
        return ffi::Fail(ffi::kErr, "invalid number of colors (6 colors must be provided exactly)");
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<ColorFilter>(isolate, SkOverdrawColorFilter::MakeWithSkColors(colors.data()));
}

ffi::RetLocal<v8::Value> ColorFilter::asAColorMode()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    SkColor color;
    SkBlendMode mode;
    if (!filter_->asAColorMode(&color, &mode))
        return v8::Null(isolate);

    using Tuple = std::tuple<uint32_t, uint32_t>;
    return ffi::Cast<Tuple>::ToChecked(isolate, std::make_tuple(
        color, static_cast<uint32_t>(mode)
    ));
}

ffi::Ret<bool> ColorFilter::asAColorMatrix(const ffi::Mem<float>& matrix)
{
    if (matrix.Size() < 20)
        return ffi::Fail(ffi::kErr, "size of matrix is insufficient (less than 20)");
    return filter_->asAColorMatrix(matrix.Address());
}

ffi::Ret<Color4fQuadruple> ColorFilter::filterColor4f(Color4fQuadruple src,
                                                      const ffi::Class<ColorSpace>& src_cs,
                                                      const ffi::Class<ColorSpace>& dst_cs)
{
    auto [r, g, b, a] = src;
    SkColor4f result = filter_->filterColor4f(
            {r, g, b, a}, src_cs->GetColorSpace().get(), dst_cs->GetColorSpace().get());
    return std::make_tuple(result.fR, result.fG, result.fB, result.fA);
}

ffi::RetLocal<v8::Value> ColorFilter::makeComposed(const ffi::Class<ColorFilter>& inner)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<ColorFilter>(isolate, filter_->makeComposed(inner->filter_));
}

ffi::RetLocal<v8::Value> ColorFilter::makeWithWorkingColorSpace(const ffi::Class<ColorSpace>& cs)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<ColorFilter>(
            isolate, filter_->makeWithWorkingColorSpace(cs->GetColorSpace()));
}

sk_sp<SkData> ColorFilter::OnSerializeImpl(SkSerialProcs *procs)
{
    return filter_->serialize(procs);
}

GALLIUM_BINDINGS_RENDERER_NS_END
