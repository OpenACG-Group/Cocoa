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

#include "include/core/SkEncodedImageFormat.h"
#include "include/core/SkSamplingOptions.h"

#include "Gallium/bindings/glamor/Exports.h"
#include "Gallium/bindings/glamor/Scene.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

#define EV(x) static_cast<uint32_t>(x)

void GlamorSetInstanceProperties(v8::Local<v8::Object> instance)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();

    using T = SkColorType;
    using A = SkAlphaType;
    std::map<std::string, uint32_t> constants{
        { "COLOR_TYPE_ALPHA8", EV(T::kAlpha_8_SkColorType) },
        { "COLOR_TYPE_RGB565", EV(T::kRGB_565_SkColorType) },
        { "COLOR_TYPE_ARGB4444", EV(T::kARGB_4444_SkColorType) },
        { "COLOR_TYPE_RGBA8888", EV(T::kRGBA_8888_SkColorType) },
        { "COLOR_TYPE_RGB888x", EV(T::kRGB_888x_SkColorType) },
        { "COLOR_TYPE_BGRA8888", EV(T::kBGRA_8888_SkColorType) },
        { "COLOR_TYPE_BGRA1010102", EV(T::kBGRA_1010102_SkColorType) },
        { "COLOR_TYPE_RGBA1010102", EV(T::kRGBA_1010102_SkColorType) },
        { "COLOR_TYPE_RGB101010x", EV(T::kRGB_101010x_SkColorType) },
        { "COLOR_TYPE_BGR101010x", EV(T::kBGR_101010x_SkColorType) },
        { "COLOR_TYPE_GRAY8", EV(T::kGray_8_SkColorType) },
        { "COLOR_TYPE_RGBA_F16_NORM", EV(T::kRGBA_F16Norm_SkColorType) },
        { "COLOR_TYPE_RGBA_F16", EV(T::kRGBA_F16_SkColorType) },
        { "COLOR_TYPE_RGBA_F32", EV(T::kRGBA_F32_SkColorType) },
        { "COLOR_TYPE_R8G8_UNORM", EV(T::kR8G8_unorm_SkColorType) },
        { "COLOR_TYPE_A16_FLOAT", EV(T::kA16_float_SkColorType) },
        { "COLOR_TYPE_R16G16_FLOAT", EV(T::kR16G16_float_SkColorType) },
        { "COLOR_TYPE_A16_UNORM", EV(T::kA16_unorm_SkColorType) },
        { "COLOR_TYPE_R16G16_UNORM", EV(T::kR16G16_unorm_SkColorType) },
        { "COLOR_TYPE_R16G16B16A16_UNORM", EV(T::kR16G16B16A16_unorm_SkColorType) },

        { "ALPHA_TYPE_PREMULTIPLIED", EV(A::kPremul_SkAlphaType) },
        { "ALPHA_TYPE_UNPREMULTIPLIED", EV(A::kUnpremul_SkAlphaType) },
        { "ALPHA_TYPE_OPAQUE", EV(A::kOpaque_SkAlphaType) },

        { "FORMAT_PNG", EV(SkEncodedImageFormat::kPNG) },
        { "FORMAT_JPEG", EV(SkEncodedImageFormat::kJPEG) },
        { "FORMAT_WEBP", EV(SkEncodedImageFormat::kWEBP) },
        { "FORMAT_GIF", EV(SkEncodedImageFormat::kGIF) },

        { "SAMPLING_FILTER_NEAREST", EV(Sampling::kNearest) },
        { "SAMPLING_FILTER_LINEAR", EV(Sampling::kLinear) },
        { "SAMPLING_CUBIC_MITCHELL", EV(Sampling::kCubicMitchell) },
        { "SAMPLING_CUBIC_CATMULL_ROM", EV(Sampling::kCubicCatmullRom) },

        { "TILE_MODE_CLAMP", EV(SkTileMode::kClamp) },
        { "TILE_MODE_REPEAT", EV(SkTileMode::kRepeat) },
        { "TILE_MODE_MIRROR", EV(SkTileMode::kMirror) },
        { "TILE_MODE_DECAL", EV(SkTileMode::kDecal) }
    };

    instance->Set(ctx, binder::to_v8(isolate, "Constants"),
                  binder::to_v8(isolate, constants)).Check();
}

GALLIUM_BINDINGS_GLAMOR_NS_END
