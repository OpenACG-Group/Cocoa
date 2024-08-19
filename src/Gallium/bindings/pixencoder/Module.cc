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

#include "Gallium/ffi/Enum.h"
#include "Gallium/ffi/DefineClass.h"
#include "Gallium/bindings/pixencoder/Module.h"
#include "Gallium/bindings/pixencoder/PNGEncoder.h"
#include "Gallium/bindings/pixencoder/JPEGEncoder.h"
#include "Gallium/bindings/pixencoder/WebpEncoder.h"
GALLIUM_BINDINGS_NS_BEGIN

PixencoderModule::PixencoderModule()
    : ffi::NativeModule("pixencoder", "Encode images into PNG, JPEG, and Webp formats",
                        /*deps=*/ { "renderer" })
{
}

ffi::LocalExports PixencoderModule::OnBuildExports(v8::Isolate *isolate)
{
    using namespace pixencoder;
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();

    ffi::LocalExports exports;

    //! TSDecl: @enum PNGFilterFlag
    exports["PNGFilterFlag"] = ffi::DefineEnum<SkPngEncoder::FilterFlag>(
            isolate, "PNGFilterFlag", true)
        .Item("Zero", SkPngEncoder::FilterFlag::kZero)   //! TSDecl: @enumitem Zero
        .Item("None", SkPngEncoder::FilterFlag::kNone)   //! TSDecl: @enumitem None
        .Item("Sub", SkPngEncoder::FilterFlag::kSub)     //! TSDecl: @enumitem Sub
        .Item("Up", SkPngEncoder::FilterFlag::kUp)       //! TSDecl: @enumitem Up
        .Item("Avg", SkPngEncoder::FilterFlag::kAvg)     //! TSDecl: @enumitem Avg
        .Item("Paeth", SkPngEncoder::FilterFlag::kPaeth) //! TSDecl: @enumitem Paeth
        .Item("All", SkPngEncoder::FilterFlag::kAll)     //! TSDecl: @enumitem All
        .Finalize();
    //! TSDecl: @end

    //! TSDecl: @enum JPEGAlphaOption
    exports["JPEGAlphaOption"] = ffi::DefineEnum<SkJpegEncoder::AlphaOption>(
            isolate, "JPEGAlphaOption", false)
        //! TSDecl: @enumitem Ignore
        .Item("Ignore", SkJpegEncoder::AlphaOption::kIgnore)
        //! TSDecl: @enumitem BlendOnBlack
        .Item("BlendOnBlack", SkJpegEncoder::AlphaOption::kBlendOnBlack)
        .Finalize();
    //! TSDecl: @end

    //! TSDecl: @enum JPEGDownsample
    exports["JPEGDownsample"] = ffi::DefineEnum<SkJpegEncoder::Downsample>(
            isolate, "JPEGDownsample", false)
        .Item("YUV420", SkJpegEncoder::Downsample::k420) //! TSDecl: @enumitem YUV420
        .Item("YUV422", SkJpegEncoder::Downsample::k422) //! TSDecl: @enumitem YUV422
        .Item("YUV444", SkJpegEncoder::Downsample::k444) //! TSDecl: @enumitem YUV444
        .Finalize();
    //! TSDecl: @end

    //! TSDecl: @enum WebpCompression
    exports["WebpCompression"] = ffi::DefineEnum<SkWebpEncoder::Compression>(
            isolate, "WebpCompression", false)
        //! TSDecl: @enumitem Lossy
        .Item("Lossy", SkWebpEncoder::Compression::kLossy)
        //! TSDecl: @enumitem Lossless
        .Item("Lossless", SkWebpEncoder::Compression::kLossless)
        .Finalize();
    //! TSDecl: @end

    ffi::DefineInterface<PNGEncoderOptions>(isolate, "PNGEncoderOptions")
        .Field("filterFlags", &PNGEncoderOptions::filter_flags)
        .Field("zlibLevel", &PNGEncoderOptions::zlib_level)
        .Field("comments", &PNGEncoderOptions::comments)
        .Finalize();

    ffi::DefineInterface<JPEGEncoderOptions>(isolate, "JPEGEncoderOptions")
        .Field("quality", &JPEGEncoderOptions::quality)
        .Field("downsample", &JPEGEncoderOptions::downsample)
        .Field("alphaOption", &JPEGEncoderOptions::alpha_option)
        .Field("xmpMetadata", &JPEGEncoderOptions::xmp_metadata)
        .Finalize();

    ffi::DefineInterface<WebpEncoderOptions>(isolate, "WebpEncoderOptions")
        .Field("quality", &WebpEncoderOptions::quality)
        .Field("compression", &WebpEncoderOptions::compression)
        .Finalize();

    ffi::DefineInterface<WebpFrame>(isolate, "WebpFrame")
        .Field("pixmap", &WebpFrame::pixmap)
        .Field("duration", &WebpFrame::duration)
        .Finalize();

    exports["PNGEncoder"] = ffi::DefineClass<PNGEncoder>(isolate)
        .MethodStaticPure("EncodeImage", PNGEncoder::EncodeImage)
        .MethodStaticPure("EncodePixmap", PNGEncoder::EncodePixmap)
        .Finalize()
        ->GetFunction(ctx).ToLocalChecked();

    exports["JPEGEncoder"] = ffi::DefineClass<JPEGEncoder>(isolate)
        .MethodStaticPure("EncodeImage", JPEGEncoder::EncodeImage)
        .MethodStaticPure("EncodePixmap", JPEGEncoder::EncodePixmap)
        .Finalize()
        ->GetFunction(ctx).ToLocalChecked();

    exports["WebpEncoder"] = ffi::DefineClass<WebpEncoder>(isolate)
        .MethodStaticPure("EncodeImage", WebpEncoder::EncodeImage)
        .MethodStaticPure("EncodePixmap", WebpEncoder::EncodePixmap)
        .MethodStaticPure("EncodeAnimated", WebpEncoder::EncodeAnimated)
        .Finalize()
        ->GetFunction(ctx).ToLocalChecked();

    return exports;
}

GALLIUM_BINDINGS_NS_END
