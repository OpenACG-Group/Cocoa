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

#include "include/encode/SkPngEncoder.h"
#include "include/encode/SkJpegEncoder.h"
#include "include/encode/SkWebpEncoder.h"

#include "Gallium/bindings/pixencoder/Exports.h"
#include "Gallium/binder/Convert.h"
GALLIUM_BINDINGS_PIXENCODER_NS_BEGIN

void SetInstanceProperties(v8::Local<v8::Object> instance)
{
    #define V(x) static_cast<uint32_t>(x)
    std::unordered_map<std::string_view, uint32_t> constants = {
        { "PNG_ENCODER_FILTER_FLAG_ZERO", V(SkPngEncoder::FilterFlag::kZero) },
        { "PNG_ENCODER_FILTER_FLAG_NONE", V(SkPngEncoder::FilterFlag::kNone) },
        { "PNG_ENCODER_FILTER_FLAG_SUB", V(SkPngEncoder::FilterFlag::kSub) },
        { "PNG_ENCODER_FILTER_FLAG_UP", V(SkPngEncoder::FilterFlag::kUp) },
        { "PNG_ENCODER_FILTER_FLAG_AVG", V(SkPngEncoder::FilterFlag::kAvg) },
        { "PNG_ENCODER_FILTER_FLAG_PAETH", V(SkPngEncoder::FilterFlag::kPaeth) },
        { "PNG_ENCODER_FILTER_FLAG_ALL", V(SkPngEncoder::FilterFlag::kAll) },

        { "JPEG_ENCODER_ALPHA_OPTION_IGNORE", V(SkJpegEncoder::AlphaOption::kIgnore) },
        { "JPEG_ENCODER_ALPHA_OPTION_BLEND_ON_BLACK", V(SkJpegEncoder::AlphaOption::kBlendOnBlack) },

        { "JPEG_ENCODER_DOWNSAMPLE_K420", V(SkJpegEncoder::Downsample::k420) },
        { "JPEG_ENCODER_DOWNSAMPLE_K422", V(SkJpegEncoder::Downsample::k422) },
        { "JPEG_ENCODER_DOWNSAMPLE_K444", V(SkJpegEncoder::Downsample::k444) },

        { "WEBP_ENCODER_COMPRESSION_LOSSY", V(SkWebpEncoder::Compression::kLossy) },
        { "WEBP_ENCODER_COMPRESSION_LOSSLESS", V(SkWebpEncoder::Compression::kLossless) }
    };

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    v8::Local<v8::Object> constants_object = binder::to_v8(isolate, constants);

    instance->Set(ctx, v8::String::NewFromUtf8Literal(isolate, "Constants"),
                  constants_object).Check();
}

GALLIUM_BINDINGS_PIXENCODER_NS_END
