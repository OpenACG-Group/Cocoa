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

#include "Gallium/ffi/Interface.h"
#include "Gallium/bindings/renderer/SamplingOptions.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

ffi::Ret<SamplingOptionsAdapter>
SamplingOptionsAdapter::Cast(v8::Isolate *isolate, v8::Local<v8::Value> value)
{
    auto maybe_iface = ffi::Cast<ffi::IFace<SamplingOptions>>::From(isolate, value);
    if (maybe_iface.HasError())
        return maybe_iface.GetError();

    ffi::IFace<SamplingOptions>& iface = maybe_iface.Extract();
    if (iface->max_aniso)
    {
        return SamplingOptionsAdapter{
            .options = SkSamplingOptions::Aniso(*iface->max_aniso)
        };
    }

    if (!iface->use_cubic)
        return ffi::Fail(ffi::kErr, "missing `useCubic` property");

    if (*iface->use_cubic)
    {
        if (!iface->cubic_C || !iface->cubic_B)
            return ffi::Fail(ffi::kErr, "missing `cubicC` and `cubicB` properties");

        SkCubicResampler resampler{ .B = *iface->cubic_B, .C = *iface->cubic_C };
        return SamplingOptionsAdapter{
            .options = SkSamplingOptions(resampler)
        };
    }

    SkFilterMode filter = SkFilterMode::kNearest;
    SkMipmapMode mipmap = SkMipmapMode::kNone;
    if (iface->filter)
        filter = **iface->filter;
    if (iface->mipmap)
        mipmap = **iface->mipmap;

    return SamplingOptionsAdapter{
        .options = SkSamplingOptions(filter, mipmap)
    };
}

ffi::Ret<ffi::IFace<SamplingOptions>> CubicSamplers::Mictchell()
{
    auto iface = ffi::IFace<SamplingOptions>::Allocate();
    new(iface.Address()) SamplingOptions();
    iface->use_cubic = true;
    iface->cubic_B = 1 / 3.0f;
    iface->cubic_C = 1 / 3.0f;
    return iface;
}

ffi::Ret<ffi::IFace<SamplingOptions>> CubicSamplers::CatmullRom()
{
    auto iface = ffi::IFace<SamplingOptions>::Allocate();
    new(iface.Address()) SamplingOptions();
    iface->use_cubic = true;
    iface->cubic_B = 0.0f;
    iface->cubic_C = 1 / 2.0f;
    return iface;
}

GALLIUM_BINDINGS_RENDERER_NS_END
