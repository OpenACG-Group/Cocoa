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

#include "include/core/SkColorType.h"
#include "include/core/SkAlphaType.h"
#include "include/effects/SkHighContrastFilter.h"

#include "Gallium/ffi/DefineClass.h"
#include "Gallium/ffi/Interface.h"
#include "Gallium/ffi/Enum.h"
#include "Gallium/ffi/Context.h"
#include "Gallium/bindings/renderer/Module.h"
#include "Gallium/bindings/renderer/Color.h"
#include "Gallium/bindings/renderer/Rect.h"
#include "Gallium/bindings/renderer/Matrix.h"
#include "Gallium/bindings/renderer/SamplingOptions.h"
#include "Gallium/bindings/renderer/ImageInfo.h"
#include "Gallium/bindings/renderer/Pixmap.h"
#include "Gallium/bindings/renderer/Surface.h"
#include "Gallium/bindings/renderer/ImageAsyncReadResult.h"
#include "Gallium/bindings/renderer/Image.h"
#include "Gallium/bindings/renderer/GpuDirectContext.h"
#include "Gallium/bindings/renderer/Canvas.h"
#include "Gallium/bindings/renderer/PathBuilder.h"
#include "Gallium/bindings/renderer/PathEffect.h"
#include "Gallium/bindings/renderer/Picture.h"
#include "Gallium/bindings/renderer/PictureRecorder.h"
#include "Gallium/bindings/renderer/ImageFilter.h"
#include "Gallium/bindings/renderer/ColorFilter.h"
#include "Gallium/bindings/renderer/Blender.h"
#include "Gallium/bindings/renderer/Shader.h"
#include "Gallium/bindings/renderer/RuntimeEffect.h"
#include "Gallium/bindings/renderer/FontStyle.h"
#include "Gallium/bindings/renderer/FontMgr.h"
#include "Gallium/bindings/renderer/Typeface.h"
#include "Gallium/bindings/renderer/Font.h"
#include "Gallium/bindings/renderer/Vertices.h"
GALLIUM_BINDINGS_NS_BEGIN

FFI_GVSTORE_DEFINE_ID(ctor_Rect)
FFI_GVSTORE_DEFINE_ID(ctor_Mat3x3)
FFI_GVSTORE_DEFINE_ID(ctor_Mat4x4)
FFI_GVSTORE_DEFINE_ID(instance_fontmgr_global)
FFI_GVSTORE_DEFINE_ID(instance_fontmgr_empty)

RendererModule::RendererModule()
    : ffi::NativeModule("renderer", "Render 2D graphics through canvas API")
{
}

ffi::LocalExports RendererModule::OnBuildExports(v8::Isolate *isolate)
{
    using namespace renderer;

    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();

    ffi::LocalExports exports;

    // Enum and constants
#define E(v) .Item(#v, SkColorType::k##v##_SkColorType)
    //! TSDecl: @enum ColorType
    exports["ColorType"] = ffi::DefineEnum<SkColorType>(isolate, "ColorType", false)
        E(Unknown)                  //! TSDecl: @enumitem Unknown
        E(Alpha_8)                  //! TSDecl: @enumitem Alpha_8
        E(RGB_565)                  //! TSDecl: @enumitem RGB_565
        E(ARGB_4444)                //! TSDecl: @enumitem ARGB_4444
        E(RGBA_8888)                //! TSDecl: @enumitem RGBA_8888
        E(RGB_888x)                 //! TSDecl: @enumitem RGB_888x
        E(BGRA_8888)                //! TSDecl: @enumitem BGRA_8888
        E(RGBA_1010102)             //! TSDecl: @enumitem RGBA_1010102
        E(BGRA_1010102)             //! TSDecl: @enumitem BGRA_1010102
        E(RGB_101010x)              //! TSDecl: @enumitem RGB_101010x
        E(BGR_101010x)              //! TSDecl: @enumitem BGR_101010x
        E(BGR_101010x_XR)           //! TSDecl: @enumitem BGR_101010x_XR
        E(RGBA_10x6)                //! TSDecl: @enumitem RGBA_10x6
        E(Gray_8)                   //! TSDecl: @enumitem Gray_8
        E(RGBA_F16Norm)             //! TSDecl: @enumitem RGBA_F16Norm
        E(RGBA_F32)                 //! TSDecl: @enumitem RGBA_F32
        E(R8G8_unorm)               //! TSDecl: @enumitem R8G8_unorm
        E(A16_float)                //! TSDecl: @enumitem A16_float
        E(R16G16_float)             //! TSDecl: @enumitem R16G16_float
        E(A16_unorm)                //! TSDecl: @enumitem A16_unorm
        E(R16G16_unorm)             //! TSDecl: @enumitem R16G16_unorm
        E(R16G16B16A16_unorm)       //! TSDecl: @enumitem R16G16B16A16_unorm
        E(SRGBA_8888)               //! TSDecl: @enumitem SRGBA_8888
        E(R8_unorm)                 //! TSDecl: @enumitem R8_unorm
        E(N32)                      //! TSDecl: @enumitem N32
        .Finalize();
#undef E
    //! TSDecl: @end

#define E(v) .Item(#v, SkAlphaType::k##v##_SkAlphaType)
    //! TSDecl: @enum AlphaType
    exports["AlphaType"] = ffi::DefineEnum<SkAlphaType>(isolate, "AlphaType", false)
        E(Unknown)  //! TSDecl: @enumitem Unknown
        E(Opaque)   //! TSDecl: @enumitem Opaque
        E(Premul)   //! TSDecl: @enumitem Premul
        E(Unpremul) //! TSDecl: @enumitem Unpremul
        .Finalize();
#undef E
    //! TSDecl: @end

#define E(v) .Item(#v, SkFilterMode::k##v)
    //! TSDecl: @enum FilterMode
    exports["FilterMode"] = ffi::DefineEnum<SkFilterMode>(isolate, "FilterMode", false)
        E(Nearest)      //! TSDecl: @enumitem Nearest
        E(Linear)       //! TSDecl: @enumitem Linear
        .Finalize();
    //! TSDecl: @end
#undef E

#define E(v) .Item(#v, SkMipmapMode::k##v)
    //! TSDecl: @enum MipmapMode
    exports["MipmapMode"] = ffi::DefineEnum<SkMipmapMode>(isolate, "MipmapMode", false)
        E(None)         //! TSDecl: @enumitem None
        E(Nearest)      //! TSDecl: @enumitem Nearest
        E(Linear)       //! TSDecl: @enumitem Linear
        .Finalize();
    //! TSDecl: @end
#undef E

#define E(v) .Item(#v, SkPixelGeometry::k##v##_SkPixelGeometry)
    //! TSDecl: @enum PixelGeometry
    exports["PixelGeometry"] = ffi::DefineEnum<SkPixelGeometry>(isolate, "PixelGeometry", false)
        E(Unknown)      //! TSDecl: @enumitem Unknown
        E(RGB_H)        //! TSDecl: @enumitem RGB_H
        E(BGR_H)        //! TSDecl: @enumitem BGR_H
        E(RGB_V)        //! TSDecl: @enumitem RGB_V
        E(BGR_V)        //! TSDecl: @enumitem BGR_V
        .Finalize();
    //! TSDecl: @end
#undef E

#define E(v) .Item(#v, SkImage::RescaleGamma::k##v)
    //! TSDecl: @enum ImageRescaleGamma
    exports["ImageRescaleGamma"] = ffi::DefineEnum<SkImage::RescaleGamma>(isolate, "ImageRescaleGamma", false)
        E(Src)          //! TSDecl: @enumitem Src
        E(Linear)       //! TSDecl: @enumitem Linear
        .Finalize();
    //! TSDecl: @end
#undef E

#define E(v) .Item(#v, SkImage::RescaleMode::k##v)
    //! TSDecl: @enum ImageRescaleMode
    exports["ImageRescaleMode"] = ffi::DefineEnum<SkImage::RescaleMode>(isolate, "ImageRescaleMode", false)
        E(Nearest)          //! TSDecl: @enumitem Nearest
        E(Linear)           //! TSDecl: @enumitem Linear
        E(RepeatedLinear)   //! TSDecl: @enumitem RepeatedLinear
        E(RepeatedCubic)    //! TSDecl: @enumitem RepeatedCubic
        .Finalize();
    //! TSDecl: @end
#undef E

#define E(v) .Item(#v, SkYUVColorSpace::k##v##_SkYUVColorSpace)
    //! TSDecl: @enum YUVColorSpace
    exports["YUVColorSpace"] = ffi::DefineEnum<SkYUVColorSpace>(isolate, "YUVColorSpace", false)
        E(JPEG_Full)                //! TSDecl: @enumitem JPEG_Full
        E(Rec601_Limited)           //! TSDecl: @enumitem Rec601_Limited
        E(Rec709_Full)              //! TSDecl: @enumitem Rec709_Full
        E(Rec709_Limited)           //! TSDecl: @enumitem Rec709_Limited
        E(BT2020_8bit_Full)         //! TSDecl: @enumitem BT2020_8bit_Full
        E(BT2020_8bit_Limited)      //! TSDecl: @enumitem BT2020_8bit_Limited
        E(BT2020_10bit_Full)        //! TSDecl: @enumitem BT2020_10bit_Full
        E(BT2020_10bit_Limited)     //! TSDecl: @enumitem BT2020_10bit_Limited
        E(BT2020_12bit_Full)        //! TSDecl: @enumitem BT2020_12bit_Full
        E(BT2020_12bit_Limited)     //! TSDecl: @enumitem BT2020_12bit_Limited
        E(Identity)                 //! TSDecl: @enumitem Identity
        .Finalize();
    //! TSDecl: @end
#undef E

#define E(v) .Item(#v, SkBlendMode::k##v)
    //! TSDecl: @enum BlendMode
    exports["BlendMode"] = ffi::DefineEnum<SkBlendMode>(isolate, "BlendMode", false)
        E(Clear)        //! TSDecl: @enumitem Clear
        E(Src)		    //! TSDecl: @enumitem Src
        E(Dst)		    //! TSDecl: @enumitem Dst
        E(SrcOver)      //! TSDecl: @enumitem SrcOver
        E(DstOver)		//! TSDecl: @enumitem DstOver
        E(SrcIn)		//! TSDecl: @enumitem SrcIn
        E(DstIn)		//! TSDecl: @enumitem DstIn
        E(SrcOut)		//! TSDecl: @enumitem SrcOut
        E(DstOut)		//! TSDecl: @enumitem DstOut
        E(SrcATop)		//! TSDecl: @enumitem SrcATop
        E(DstATop)		//! TSDecl: @enumitem DstATop
        E(Xor)		    //! TSDecl: @enumitem Xor
        E(Plus)		    //! TSDecl: @enumitem Plus
        E(Modulate)		//! TSDecl: @enumitem Modulate
        E(Screen)		//! TSDecl: @enumitem Screen
        E(Overlay)		//! TSDecl: @enumitem Overlay
        E(Darken)		//! TSDecl: @enumitem Darken
        E(Lighten)		//! TSDecl: @enumitem Lighten
        E(ColorDodge)	//! TSDecl: @enumitem ColorDodge
        E(ColorBurn)	//! TSDecl: @enumitem ColorBurn
        E(HardLight)	//! TSDecl: @enumitem HardLight
        E(SoftLight)	//! TSDecl: @enumitem SoftLight
        E(Difference)	//! TSDecl: @enumitem Difference
        E(Exclusion)	//! TSDecl: @enumitem Exclusion
        E(Multiply)		//! TSDecl: @enumitem Multiply
        E(Hue)		    //! TSDecl: @enumitem Hue
        E(Saturation)	//! TSDecl: @enumitem Saturation
        E(Color)		//! TSDecl: @enumitem Color
        E(Luminosity)	//! TSDecl: @enumitem Luminosity
        .Finalize();
    //! TSDecl: @end
#undef E

    exports["ImageMemoryMutability"] = ffi::DefineEnum<ImageMemoryMutability>(
            isolate, "ImageMemoryMutability", false)
        .Item("MutableAndCopy", ImageMemoryMutability::kMutableAndCopy)
        .Item("ImmutableAndShare", ImageMemoryMutability::kImmutableAndShare)
        .Finalize();

    //! TSDecl: @enum TextureCompressionType
    exports["TextureCompressionType"] = ffi::DefineEnum<SkTextureCompressionType>(
            isolate, "TextureCompressionType", false)
        //! TSDecl: @enumitem ETC2_RGB8_UNORM
        .Item("ETC2_RGB8_UNORM", SkTextureCompressionType::kETC2_RGB8_UNORM)
        //! TSDecl: @enumitem BC1_RGB8_UNORM
        .Item("BC1_RGB8_UNORM", SkTextureCompressionType::kBC1_RGB8_UNORM)
        //! TSDecl: @enumitem BC1_RGBA8_UNORM
        .Item("BC1_RGBA8_UNORM", SkTextureCompressionType::kBC1_RGBA8_UNORM)
        //! TSDecl: @enumitem None
        .Item("None", SkTextureCompressionType::kNone)
        .Finalize();
    //! TSDecl: @end

    //! TSDecl: @enum Style
    exports["Style"] = ffi::DefineEnum<SkPaint::Style>(isolate, "Style", false)
        //! TSDecl: @enumitem Fill
        .Item("Fill", SkPaint::Style::kFill_Style)
        //! TSDecl: @enumitem Stroke
        .Item("Stroke", SkPaint::Style::kStroke_Style)
        //! TSDecl: @enumitem StrokeAndFill
        .Item("StrokeAndFill", SkPaint::Style::kStrokeAndFill_Style)
        .Finalize();
    //! TSDecl: @end

    //! TSDecl: @enum LineCap
    exports["LineCap"] = ffi::DefineEnum<SkPaint::Cap>(isolate, "LineCap", false)
        //! TSDecl: @enumitem Butt
        .Item("Butt", SkPaint::Cap::kButt_Cap)
        //! TSDecl: @enumitem Round
        .Item("Round", SkPaint::Cap::kRound_Cap)
        //! TSDecl: @enumitem Square
        .Item("Square", SkPaint::Cap::kSquare_Cap)
        //! TSDecl: @enumitem Default
        .Item("Default", SkPaint::Cap::kDefault_Cap)
        .Finalize();
    //! TSDecl: @end

    //! TSDecl: @enum LineJoin
    exports["LineJoin"] = ffi::DefineEnum<SkPaint::Join>(isolate, "LineJoin", false)
        //! TSDecl: @enumitem Miter
        .Item("Miter", SkPaint::Join::kMiter_Join)
        //! TSDecl: @enumitem Round
        .Item("Round", SkPaint::Join::kRound_Join)
        //! TSDecl: @enumitem Bevel
        .Item("Bevel", SkPaint::Join::kBevel_Join)
        //! TSDecl: @enumitem Default
        .Item("Default", SkPaint::Join::kDefault_Join)
        .Finalize();
    //! TSDecl: @end

    //! TSDecl: @enum SaveLayerFlags
    exports["SaveLayerFlags"] = ffi::DefineEnum<SkCanvas::SaveLayerFlagsSet>(isolate, "SaveLayerFlags", true)
        //! TSDecl: @enumitem PreserveLCDText
        .Item("PreserveLCDText", SkCanvas::kPreserveLCDText_SaveLayerFlag)
        //! TSDecl: @enumitem InitWithPrevious
        .Item("InitWithPrevious", SkCanvas::kInitWithPrevious_SaveLayerFlag)
        //! TSDecl: @enumitem F16ColorType
        .Item("F16ColorType", SkCanvas::kF16ColorType)
        .Finalize();
    //! TSDecl: @end

    //! TSDecl: @enum ClipOp
    exports["ClipOp"] = ffi::DefineEnum<SkClipOp>(isolate, "ClipOp", false)
        //! TSDecl: @enumitem Difference
        .Item("Difference", SkClipOp::kDifference)
        //! TSDecl: @enumitem Intersect
        .Item("Intersect", SkClipOp::kIntersect)
        .Finalize();
    //! TSDecl: @end

    //! TSDecl: @enum PointMode
    exports["PointMode"] = ffi::DefineEnum<SkCanvas::PointMode>(isolate, "PointMode", false)
        //! TSDecl: @enumitem Points
        .Item("Points", SkCanvas::kPoints_PointMode)
        //! TSDecl: @enumitem Lines
        .Item("Lines", SkCanvas::kLines_PointMode)
        //! TSDecl: @enumitem Polygon
        .Item("Polygon", SkCanvas::kPolygon_PointMode)
        .Finalize();
    //! TSDecl: @end

    //! TSDecl: @enum PathFillType
    exports["PathFillType"] = ffi::DefineEnum<SkPathFillType>(isolate, "PathFillType", false)
        //! TSDecl: @enumitem Winding
        .Item("Winding", SkPathFillType::kWinding)
        //! TSDecl: @enumitem EvenOdd
        .Item("EvenOdd", SkPathFillType::kEvenOdd)
        //! TSDecl: @enumitem InverseWinding
        .Item("InverseWinding", SkPathFillType::kInverseWinding)
        //! TSDecl: @enumitem InverseEvenOdd
        .Item("InverseEvenOdd", SkPathFillType::kInverseEvenOdd)
        .Finalize();
    //! TSDecl: @end

    //! TSDecl: @enum PathDirection
    exports["PathDirection"] = ffi::DefineEnum<SkPathDirection>(isolate, "PathDirection", false)
        //! TSDecl: @enumitem CW
        .Item("CW", SkPathDirection::kCW)
        //! TSDecl: @enumitem CCW
        .Item("CCW", SkPathDirection::kCCW)
        .Finalize();
    //! TSDecl: @end

    //! TSDecl: @enum PathSegmentMask
    exports["PathSegmentMask"] = ffi::DefineEnum<SkPathSegmentMask>(isolate, "PathSegmentMask", true)
        //! TSDecl: @enumitem Line
        .Item("Line", SkPathSegmentMask::kLine_SkPathSegmentMask)
        //! TSDecl: @enumitem Quad
        .Item("Quad", SkPathSegmentMask::kQuad_SkPathSegmentMask)
        //! TSDecl: @enumitem Conic
        .Item("Conic", SkPathSegmentMask::kConic_SkPathSegmentMask)
        //! TSDecl: @enumitem Cubic
        .Item("Cubic", SkPathSegmentMask::kCubic_SkPathSegmentMask)
        .Finalize();
    //! TSDecl: @end

    //! TSDecl: @enum PathVerb
    exports["PathVerb"] = ffi::DefineEnum<SkPathVerb>(isolate, "PathVerb", false)
        //! TSDecl: @enumitem Move
        .Item("Move", SkPathVerb::kMove)
        //! TSDecl: @enumitem Line
        .Item("Line", SkPathVerb::kLine)
        //! TSDecl: @enumitem Quad
        .Item("Quad", SkPathVerb::kQuad)
        //! TSDecl: @enumitem Conic
        .Item("Conic", SkPathVerb::kConic)
        //! TSDecl: @enumitem Cubic
        .Item("Cubic", SkPathVerb::kCubic)
        //! TSDecl: @enumitem Close
        .Item("Close", SkPathVerb::kClose)
        .Finalize();
    //! TSDecl: @end

    //! TSDecl: @enum AddPathMode
    exports["AddPathMode"] = ffi::DefineEnum<SkPath::AddPathMode>(isolate, "AddPathMode", false)
        //! TSDecl: @enumitem Append
        .Item("Append", SkPath::AddPathMode::kAppend_AddPathMode)
        //! TSDecl: @enumitem Extend
        .Item("Extend", SkPath::AddPathMode::kExtend_AddPathMode)
        .Finalize();
    //! TSDecl: @end

    //! TSDecl: @enum Path1DEffectStyle
    exports["Path1DEffectStyle"] = ffi::DefineEnum<SkPath1DPathEffect::Style>(isolate, "Path1DEffectStyle", false)
        //! TSDecl: @enumitem Translate
        .Item("Translate", SkPath1DPathEffect::Style::kTranslate_Style)
        //! TSDecl: @enumitem Rotate
        .Item("Rotate", SkPath1DPathEffect::Style::kRotate_Style)
        //! TSDecl: @enumitem Morph
        .Item("Morph", SkPath1DPathEffect::Style::kMorph_Style)
        .Finalize();
    //! TSDecl: @end

    //! TSDecl: @enum MapDirection
    exports["MapDirection"] = ffi::DefineEnum<SkImageFilter::MapDirection>(isolate, "MapDirection", false)
        //! TSDecl: @enumitem Forward
        .Item("Forward", SkImageFilter::MapDirection::kForward_MapDirection)
        //! TSDecl: @enumitem Reverse
        .Item("Reverse", SkImageFilter::MapDirection::kReverse_MapDirection)
        .Finalize();
    //! TSDecl: @end

    //! TSDecl: @enum TileMode
    exports["TileMode"] = ffi::DefineEnum<SkTileMode>(isolate, "TileMode", false)
        //! TSDecl: @enumitem Clamp
        .Item("Clamp", SkTileMode::kClamp)
        //! TSDecl: @enumitem Repeat
        .Item("Repeat", SkTileMode::kRepeat)
        //! TSDecl: @enumitem Mirror
        .Item("Mirror", SkTileMode::kMirror)
        //! TSDecl: @enumitem Decal
        .Item("Decal", SkTileMode::kDecal)
        .Finalize();
    //! TSDecl: @end

    //! TSDecl: @enum ColorChannel
    exports["ColorChannel"] = ffi::DefineEnum<SkColorChannel>(isolate, "ColorChannel", false)
        //! TSDecl: @enumitem R
        .Item("R", SkColorChannel::kR)
        //! TSDecl: @enumitem G
        .Item("G", SkColorChannel::kG)
        //! TSDecl: @enumitem B
        .Item("B", SkColorChannel::kB)
        //! TSDecl: @enumitem A
        .Item("A", SkColorChannel::kA)
        .Finalize();
    //! TSDecl: @end

    //! TSDecl: @enum HighContrastInvertStyle
    exports["HighContrastInvertStyle"] = ffi::DefineEnum<SkHighContrastConfig::InvertStyle>(
            isolate, "HighContrastInvertStyle", false)
        //! TSDecl: @enumitem No
        .Item("No", SkHighContrastConfig::InvertStyle::kNoInvert)
        //! TSDecl: @enumitem Brightness
        .Item("Brightness", SkHighContrastConfig::InvertStyle::kInvertBrightness)
        //! TSDecl: @enumitem Lightness
        .Item("Lightness", SkHighContrastConfig::InvertStyle::kInvertLightness)
        .Finalize();
    //! TSDecl: @end

#define E(x)  .Item(#x, SkGradientShader::Interpolation::ColorSpace::k##x)
    //! TSDecl: @enum GradientInterpColorSpace
    exports["GradientInterpColorSpace"] = ffi::DefineEnum<SkGradientShader::Interpolation::ColorSpace>(
            isolate, "GradientInterpColorSpace", false)
        E(Destination)        //! TSDecl: @enumitem Destination
        E(SRGBLinear)         //! TSDecl: @enumitem SRGBLinear
        E(Lab)                //! TSDecl: @enumitem Lab
        E(OKLab)              //! TSDecl: @enumitem OKLab
        E(OKLabGamutMap)      //! TSDecl: @enumitem OKLabGamutMap
        E(LCH)                //! TSDecl: @enumitem LCH
        E(OKLCH)              //! TSDecl: @enumitem OKLCH
        E(OKLCHGamutMap)      //! TSDecl: @enumitem OKLCHGamutMap
        E(SRGB)               //! TSDecl: @enumitem SRGB
        E(HSL)                //! TSDecl: @enumitem HSL
        E(HWB)                //! TSDecl: @enumitem HWB
        .Finalize();
    //! TSDecl: @end
#undef E

#define E(x)  .Item(#x, SkGradientShader::Interpolation::HueMethod::k##x)
    //! TSDecl: @enum GradientInterpHueMethod
    exports["GradientInterpHueMethod"] = ffi::DefineEnum<SkGradientShader::Interpolation::HueMethod>(
            isolate, "GradientInterpHueMethod", false)
        E(Shorter)      //! TSDecl: @enumitem Shorter
        E(Longer)       //! TSDecl: @enumitem Longer
        E(Increasing)   //! TSDecl: @enumitem Increasing
        E(Decreasing)   //! TSDecl: @enumitem Decreasing
        .Finalize();
    //! TSDecl: @end
#undef E

#define E(x)    .Item(#x, SkRuntimeEffect::Uniform::Type::k##x)
    //! TSDecl: @enum SkSLUniformType
    exports["SkSLUniformType"] = ffi::DefineEnum<SkRuntimeEffect::Uniform::Type>(
            isolate, "SkSLUniformType", false)
        E(Float)        //! TSDecl: @enumitem Float
        E(Float2)       //! TSDecl: @enumitem Float2
        E(Float3)       //! TSDecl: @enumitem Float3
        E(Float4)       //! TSDecl: @enumitem Float4
        E(Float2x2)     //! TSDecl: @enumitem Float2x2
        E(Float3x3)     //! TSDecl: @enumitem Float3x3
        E(Float4x4)     //! TSDecl: @enumitem Float4x4
        E(Int)          //! TSDecl: @enumitem Int
        E(Int2)         //! TSDecl: @enumitem Int2
        E(Int3)         //! TSDecl: @enumitem Int3
        E(Int4)         //! TSDecl: @enumitem Int4
        .Finalize();
    //! TSDecl: @end
#undef E

#define E(x)    .Item(#x, SkRuntimeEffect::Uniform::Flags::k##x##_Flag)
    //! TSDecl: @enum SkSLUniformFlags
    exports["SkSLUniformFlags"] = ffi::DefineEnum<SkRuntimeEffect::Uniform::Flags>(
            isolate, "SkSLUniformFlags", true)
        E(Array)            //! TSDecl: @enumitem Array
        E(Color)            //! TSDecl: @enumitem Color
        E(Vertex)           //! TSDecl: @enumitem Vertex
        E(Fragment)         //! TSDecl: @enumitem Fragment
        E(HalfPrecision)    //! TSDecl: @enumitem HalfPrecision
        .Finalize();
    //! TSDecl: @end
#undef E

#define E(x)    .Item(#x, SkRuntimeEffect::ChildType::k##x)
    //! TSDecl: @enum SkSLChildType
    exports["SkSLChildType"] = ffi::DefineEnum<SkRuntimeEffect::ChildType>(
            isolate, "SkSLChildType", false)
        E(Shader)       //! TSDecl: @enumitem Shader
        E(ColorFilter)  //! TSDecl: @enumitem ColorFilter
        E(Blender)      //! TSDecl: @enumitem Blender
        .Finalize();
    //! TSDecl: @end
#undef E

#define E(x)    .Item(#x, SkFontStyle::Weight::k##x##_Weight)
    //! TSDecl: @enum FontWeight
    exports["FontWeight"] = ffi::DefineEnum<SkFontStyle::Weight>(isolate, "FontWeight", false)
        E(Invisible)        //! TSDecl: @enumitem Invisible
        E(Thin)             //! TSDecl: @enumitem Thin
        E(ExtraLight)       //! TSDecl: @enumitem ExtraLight
        E(Light)            //! TSDecl: @enumitem Light
        E(Normal)           //! TSDecl: @enumitem Normal
        E(Medium)           //! TSDecl: @enumitem Medium
        E(SemiBold)         //! TSDecl: @enumitem SemiBold
        E(Bold)             //! TSDecl: @enumitem Bold
        E(ExtraBold)        //! TSDecl: @enumitem ExtraBold
        E(Black)            //! TSDecl: @enumitem Black
        E(ExtraBlack)       //! TSDecl: @enumitem ExtraBlack
        .Finalize();
    //! TSDecl: @end
#undef E

#define E(x)    .Item(#x, SkFontStyle::Width::k##x##_Width)
    //! TSDecl: @enum FontWidth
    exports["FontWidth"] = ffi::DefineEnum<SkFontStyle::Width>(isolate, "Width", false)
        E(UltraCondensed)   //! TSDecl: @enumitem UltraCondensed
        E(ExtraCondensed)   //! TSDecl: @enumitem ExtraCondensed
        E(Condensed)        //! TSDecl: @enumitem Condensed
        E(SemiCondensed)    //! TSDecl: @enumitem SemiCondensed
        E(Normal)           //! TSDecl: @enumitem Normal
        E(SemiExpanded)     //! TSDecl: @enumitem SemiExpanded
        E(Expanded)         //! TSDecl: @enumitem Expanded
        E(ExtraExpanded)    //! TSDecl: @enumitem ExtraExpanded
        E(UltraExpanded)    //! TSDecl: @enumitem UltraExpanded
        .Finalize();
    //! TSDecl: @end
#undef E

    //! TSDecl: @enum FontSlant
    exports["FontSlant"] = ffi::DefineEnum<SkFontStyle::Slant>(isolate, "Slant", false)
        //! TSDecl: @enumitem Upright
        .Item("Upright", SkFontStyle::Slant::kUpright_Slant)
        //! TSDecl: @enumitem Italic
        .Item("Italic", SkFontStyle::Slant::kItalic_Slant)
        //! TSDecl: @enumitem Oblique
        .Item("Oblique", SkFontStyle::Slant::kOblique_Slant)
        .Finalize();
    //! TSDecl: @end

    //! TSDecl: @enum TypefaceSerializeBehavior
    exports["TypefaceSerializeBehavior"] = ffi::DefineEnum<SkTypeface::SerializeBehavior>(
            isolate, "TypefaceSerializeBehavior", false)
        //! TSDecl: @enumitem IncludeData
        .Item("IncludeData", SkTypeface::SerializeBehavior::kDoIncludeData)
        //! TSDecl: @enumitem NotIncludeData
        .Item("NotIncludeData", SkTypeface::SerializeBehavior::kDontIncludeData)
        //! TSDecl: @enumitem IncludeDataIfLocal
        .Item("IncludeDataIfLocal", SkTypeface::SerializeBehavior::kIncludeDataIfLocal)
        .Finalize();
    //! TSDecl: @end

    //! TSDecl: @enum TextEncoding
    exports["TextEncoding"] = ffi::DefineEnum<SkTextEncoding>(isolate, "TextEncoding", false)
        //! TSDecl: @enumitem UTF8
        .Item("UTF8", SkTextEncoding::kUTF8)
        //! TSDecl: @enumitem UTF16
        .Item("UTF16", SkTextEncoding::kUTF16)
        //! TSDecl: @enumitem UTF32
        .Item("UTF32", SkTextEncoding::kUTF32)
        //! TSDecl: @enumitem GlyphID
        .Item("GlyphID", SkTextEncoding::kGlyphID)
        .Finalize();
    //! TSDecl: @end

    //! TSDecl: @enum FontEdging
    exports["FontEdging"] = ffi::DefineEnum<SkFont::Edging>(isolate, "Edging", false)
        //! TSDecl: @enumitem Alias
        .Item("Alias", SkFont::Edging::kAlias)
        //! TSDecl: @enumitem AntiAlias
        .Item("AntiAlias", SkFont::Edging::kAntiAlias)
        //! TSDecl: @enumitem SubpixelAntiAlias
        .Item("SubpixelAntiAlias", SkFont::Edging::kSubpixelAntiAlias)
        .Finalize();
    //! TSDecl: @end

    //! TSDecl: @enum FontHinting
    exports["FontHinting"] = ffi::DefineEnum<SkFontHinting>(isolate, "FontHinting", false)
        //! TSDecl: @enumitem None
        .Item("None", SkFontHinting::kNone)
        //! TSDecl: @enumitem Slight
        .Item("Slight", SkFontHinting::kSlight)
        //! TSDecl: @enumitem Normal
        .Item("Normal", SkFontHinting::kNormal)
        //! TSDecl: @enumitem Full
        .Item("Full", SkFontHinting::kFull)
        .Finalize();
    //! TSDecl: @end

    //! TSDecl: @enum VertexMode
    exports["VertexMode"] = ffi::DefineEnum<SkVertices::VertexMode>(isolate, "VertexMode", false)
        //! TSDecl: @enumitem Triangles
        .Item("Triangles", SkVertices::VertexMode::kTriangles_VertexMode)
        //! TSDecl: @enumitem TriangleStrip
        .Item("TriangleStrip", SkVertices::VertexMode::kTriangleStrip_VertexMode)
        //! TSDecl: @enumitem TriangleFan
        .Item("TriangleFan", SkVertices::VertexMode::kTriangleFan_VertexMode)
        .Finalize();
    //! TSDecl: @end




    // Interfaces

    ffi::DefineInterface<RRect>(isolate, "RRect")
        .Field("rect", &RRect::rect)
        .Field("uniformRadii", &RRect::uniform_radii)
        .Field("borderRadii", &RRect::border_radii)
        .Finalize();

    ffi::DefineInterface<SamplingOptions>(isolate, "SamplingOptions")
        .Field("maxAniso", &SamplingOptions::max_aniso)
        .Field("useCubic", &SamplingOptions::use_cubic)
        .Field("cubicB", &SamplingOptions::cubic_B)
        .Field("cubicC", &SamplingOptions::cubic_C)
        .Field("filter", &SamplingOptions::filter)
        .Field("mipmap", &SamplingOptions::mipmap)
        .Finalize();

    ffi::DefineInterface<Pixmap>(isolate, "Pixmap")
        .Field("pixels", &Pixmap::pixels)
        .Field("rowBytes", &Pixmap::row_bytes)
        .Field("imageInfo", &Pixmap::image_info)
        .Finalize();

    ffi::DefineInterface<SurfaceProps>(isolate, "SurfaceProps")
        .Field("useDeviceIndependentFonts", &SurfaceProps::use_device_independent_fonts)
        .Field("dynamicMSAA", &SurfaceProps::dynamic_msaa)
        .Field("alwaysDither", &SurfaceProps::always_dither)
        .Field("pixelGeometry", &SurfaceProps::pixel_geometry)
        .Finalize();

    ffi::DefineInterface<Serializers>(isolate, "Serializers")
        .Field("onImage", &Serializers::on_image)
        .Field("onPicture", &Serializers::on_picture)
        .Field("onTypeface", &Serializers::on_typeface)
        .Finalize();

    ffi::DefineInterface<Deserializers>(isolate, "Deserializers")
        .Field("onImage", &Deserializers::on_image)
        .Field("onPicture", &Deserializers::on_picture)
        .Field("onTypeface", &Deserializers::on_typeface)
        .Finalize();

    ffi::DefineInterface<HighContrastConfig>(isolate, "HighContrastConfig")
        .Field("grayscale", &HighContrastConfig::grayscale)
        .Field("invertStyle", &HighContrastConfig::invert_style)
        .Field("contrast", &HighContrastConfig::contrast)
        .Finalize();

    ffi::DefineInterface<GradientInterpolation>(isolate, "GradientInterpolation")
        .Field("inPremul", &GradientInterpolation::in_premul)
        .Field("colorSpace", &GradientInterpolation::color_space)
        .Field("hueMethod", &GradientInterpolation::hue_method)
        .Finalize();

    ffi::DefineInterface<SaveLayerRec>(isolate, "SaveLayerRec")
        .Field("bounds", &SaveLayerRec::bounds)
        .Field("paint", &SaveLayerRec::paint)
        .Field("flags", &SaveLayerRec::flags)
        .Field("backdrop", &SaveLayerRec::backdrop)
        .Field("filters", &SaveLayerRec::filters)
        .Finalize();

    ffi::DefineInterface<SkSLReflectUniform>(isolate, "SkSLReflectUniform")
        .Field("name", &SkSLReflectUniform::name, false, true)
        .Field("offset", &SkSLReflectUniform::offset, false, true)
        .Field("sizeInBytes", &SkSLReflectUniform::size_in_bytes, false, true)
        .Field("type", &SkSLReflectUniform::type, false, true)
        .Field("count", &SkSLReflectUniform::count, false, true)
        .Field("flags", &SkSLReflectUniform::flags, false, true)
        .Finalize();

    ffi::DefineInterface<SkSLReflectChild>(isolate, "SkSLReflectChild")
        .Field("name", &SkSLReflectChild::name, false, true)
        .Field("type", &SkSLReflectChild::type, false, true)
        .Field("index", &SkSLReflectChild::index, false, true)
        .Finalize();

    ffi::DefineInterface<FontStyle>(isolate, "FontStyle")
        .Field("weight", &FontStyle::weight)
        .Field("width", &FontStyle::width)
        .Field("slant", &FontStyle::slant)
        .Finalize();

    ffi::DefineInterface<FontArguments>(isolate, "FontArguments")
        .Field("collectionIndex", &FontArguments::collection_index)
        .Field("variationDesignPosition", &FontArguments::variation_design_position)
        .Field("paletteIndex", &FontArguments::palette_index)
        .Field("paletteOverrides", &FontArguments::palette_overrides)
        .Finalize();



    // Classes

#define MS(v) .MethodStatic(#v, ColorSpace::v)
#define M(v)  .Method(#v, &ColorSpace::v)
    exports["ColorSpace"] = ffi::DefineClass<ColorSpace>(isolate)
        MS(MakeSRGB)
        MS(MakeSRGBLinear)
        MS(Equals)
        M(dispose)
        M(clone)
        M(gammaCloseToSRGB)
        M(gammaIsLinear)
        M(makeLinearGamma)
        M(makeSRGBGamma)
        M(makeColorSpin)
        M(isSRGB)
        .Finalize(ctx);
#undef MS
#undef M

    exports["ColorMatrix"] = ffi::DefineClass<ColorMatrix>(isolate)
        .Constructor<float, float, float, float, float,
                     float, float, float, float, float,
                     float, float, float, float, float,
                     float, float, float, float, float>()
        .MethodStatic("RGBtoYUV", ColorMatrix::RGBtoYUV)
        .MethodStatic("YUVtoRGB", ColorMatrix::YUVtoRGB)
        .Method("setIdentity", &ColorMatrix::setIdentity)
        .Method("setScale", &ColorMatrix::setScale)
        .Method("postTranslate", &ColorMatrix::postTranslate)
        .Method("setConcat", &ColorMatrix::setConcat)
        .Method("preConcat", &ColorMatrix::preConcat)
        .Method("postConcat", &ColorMatrix::postConcat)
        .Method("setSaturation", &ColorMatrix::setSaturation)
        .Method("setRowMajor", &ColorMatrix::setRowMajor)
        .Method("getRowMajor", &ColorMatrix::getRowMajor)
        .Finalize(ctx);

#define MS(v) .MethodStatic(#v, ImageInfo::v)
#define M(v)  .Method(#v, &ImageInfo::v)
    exports["ImageInfo"] = ffi::DefineClass<ImageInfo>(isolate)
        MS(Make)
        MS(MakeN32)
        MS(MakeS32)
        MS(MakeN32Premul)
        MS(MakeA8)
        MS(MakeUnknown)
        .Property<int32_t>("width", &ImageInfo::getWidth, nullptr)
        .Property<int32_t>("height", &ImageInfo::getHeight, nullptr)
        .Property<ffi::Enum<SkColorType>>("colorType", &ImageInfo::getColorType, nullptr)
        .Property<ffi::Enum<SkAlphaType>>("alphaType", &ImageInfo::getAlphaType, nullptr)
        M(isEmpty)
        M(isOpaque)
        M(refColorSpace)
        M(gammaCloseToSRGB)
        M(makeWH)
        M(makeAlphaType)
        M(makeColorType)
        M(makeColorSpace)
        .Property<int32_t>("bytesPerPixel", &ImageInfo::getBytesPerPixel, nullptr)
        .Property<int32_t>("shiftPerPixel", &ImageInfo::getShiftPerPixel, nullptr)
        .Property<size_t>("minRowBytes", &ImageInfo::getMinRowBytes, nullptr)
        M(computeOffset)
        M(equalsTo)
        M(computeByteSize)
        M(computeMinByteSize)
        M(validRowBytes)
        M(reset)
        .Finalize(ctx);
#undef MS
#undef M

    exports["CubicSamplers"] = ffi::DefineClass<CubicSamplers>(isolate)
        .MethodStaticPure("Mitchell", CubicSamplers::Mictchell)
        .MethodStaticPure("CatmullRom", CubicSamplers::CatmullRom)
        .Finalize(ctx);

    exports["PixmapUtils"] = ffi::DefineClass<PixmapUtils>(isolate)
        .MethodStaticPure("ComputeIsOpaque", PixmapUtils::ComputeIsOpaque)
        .MethodStaticPure("Copy", PixmapUtils::Copy)
        .MethodStaticPure("EraseSubset", PixmapUtils::EraseSubset)
        .MethodStaticPure("Erase", PixmapUtils::Erase)
        .MethodStaticPure("Scale", PixmapUtils::Scale)
        .MethodStaticPure("PickColor", &PixmapUtils::PickColor)
        .Finalize(ctx);

#define MS(v) .MethodStatic(#v, Surface::v)
#define M(v)  .Method(#v, &Surface::v)
#define P(t, name, g) .Property<t>(name, &Surface::g, nullptr)
    exports["Surface"] = ffi::DefineClass<Surface>(isolate)
        MS(MakeNull)
        MS(MakeRaster)
        MS(MakeFromPixmap)
        M(dispose)
        P(int32_t, "width", getWidth)
        P(int32_t, "height", getHeight)
        P(v8::Local<v8::Object>, "imageInfo", getImageInfo)
        P(uint32_t, "generationID", getGenerationID)
        P(v8::Local<v8::Value>, "canvas", getCanvas)
        M(makeImageSnapshot)
        M(peekPixels)
        M(notifyContentWillChange)
        M(writePixels)
        M(asyncRescaleAndReadPixels)
        M(asyncRescaleAndReadPixelsYUV420)
        M(asyncRescaleAndReadPixelsYUVA420)
        .Finalize(ctx);
#undef P
#undef M
#undef MS

#define M(v)            .Method(#v, &Canvas::v)
#define P(t, name, g)   .Property<t>(name, &Canvas::g, nullptr)
    exports["Canvas"] = ffi::DefineClass<Canvas>(isolate)
        .Inherit<EventEmitterBase>()
        P(v8::Local<v8::Value>, "parent", getParent)
        M(save)
        M(saveLayer)
        M(saveLayerAlphaf)
        M(saveLayerRec)
        M(restore)
        M(getSaveCount)
        M(restoreToCount)
        M(translate)
        M(scale)
        M(rotate)
        M(rotatePivot)
        M(skew)
        M(concat33)
        M(concat44)
        M(setMatrix)
        M(resetMatrix)
        M(clipRect)
        M(clipRRect)
        M(clipPath)
        M(clipShader)
        M(quickRejectRect)
        M(quickRejectPath)
        P(v8::Local<v8::Value>, "localClipBounds", getLocalClipBounds)
        P(v8::Local<v8::Value>, "deviceClipBounds", getDeviceClipBounds)
        M(drawColor)
        M(drawColor4f)
        M(clear)
        M(discard)
        M(drawPaint)
        M(drawPoints)
        M(drawPoint)
        M(drawLine)
        M(drawRect)
        M(drawOval)
        M(drawRRect)
        M(drawDRRect)
        M(drawCircle)
        M(drawArc)
        M(drawRoundRect)
        M(drawPath)
        M(drawImage)
        M(drawImageRect)
        M(drawImageRectToRect)
        M(drawPicture)
        M(drawString)
        M(drawGlyphs)
        M(drawVertices)
        .Finalize(ctx);
#undef P
#undef M

#define M(v) .Method(#v, &ImageAsyncReadResult::v)
    exports["ImageAsyncReadResult"] = ffi::DefineClass<ImageAsyncReadResult>(isolate)
        .Property<int32_t>("count", &ImageAsyncReadResult::getCount, nullptr)
        M(dispose)
        M(dimensionsOf)
        M(bytesPerPixelOf)
        M(readPlane)
        .Finalize(ctx);
#undef M

#define MS(v) .MethodStatic(#v, Image::v)
#define M(v)  .Method(#v, &Image::v)
#define P(t, name, g) .Property<t>(name, &Image::g, nullptr)
    exports["Image"] = ffi::DefineClass<Image>(isolate)
        MS(FromPixmap)
        MS(FromCompressedTextureData)
        MS(DeferredFromEncodedData)
        MS(DeferredFromEncodedFile)
        MS(BatchFromEncodedFile)
        MS(DeferredFromPicture)
        MS(MakeWithFilter)
        M(dispose)
        P(v8::Local<v8::Value>, "imageInfo", getImageInfo)
        P(int32_t, "width", getWidth)
        P(int32_t, "height", getHeight)
        P(v8::Local<v8::Value>, "bounds", getBounds)
        P(uint32_t, "uniqueID", getUniqueID)
        M(isTextureBacked)
        M(textureSize)
        M(isValid)
        M(readPixels)
        M(asyncRescaleAndReadPixels)
        M(asyncRescaleAndReadPixelsYUV420)
        M(asyncRescaleAndReadPixelsYUVA420)
        M(scalePixels)
        M(makeSubset)
        P(bool, "hasMipmaps", getHasMipmaps)
        P(bool, "isProtected", getIsProtected)
        M(withDefaultMipmaps)
        M(makeNonTextureImage)
        M(makeRasterImage)
        P(bool, "isLazyGenerated", getIsLazyGenerated)
        M(makeColorSpace)
        M(makeColorTypeAndColorSpace)
        M(reinterpretColorSpace)
        M(makeShader)
        M(makeRawShader)
        .Finalize(ctx);
#undef M
#undef MS
#undef P

#define MS(v) .MethodStatic(#v, GpuDirectContext::v)
#define M(v)  .Method(#v, &GpuDirectContext::v)
    exports["GpuDirectContext"] = ffi::DefineClass<GpuDirectContext>(isolate)
        M(dispose)
        .Finalize(ctx);
#undef M
#undef MS

    exports["Paint"] = ffi::DefineClass<Paint>(isolate)
        .Constructor()
        .Method("clone", &Paint::clone)
        .Method("equalTo", &Paint::equalTo)
        .Method("reset", &Paint::reset)
#define ACCESSORS(type, prop) .Property<type>(#prop, &Paint::get_##prop, &Paint::set_##prop)
        PAINT_PROPERTIES_MAP(ACCESSORS)
#undef ACCESSORS
        .Method("getColor4f", &Paint::getColor4f)
        .Method("setColor4f", &Paint::setColor4f)
        .Method("nothingToDraw", &Paint::nothingToDraw)
        .Method("canComputeFastBounds", &Paint::canComputeFastBounds)
        .Method("computeFastBounds", &Paint::computeFastBounds)
        .Finalize(ctx);

#define M(v)  .Method(#v, &Path::v)
    exports["Path"] = ffi::DefineClass<Path>(isolate)
        .Constructor()
        .MethodStatic("Make", Path::Make)
        .MethodStatic("Rect", Path::Rect)
        .MethodStatic("Oval", Path::Oval)
        .MethodStatic("Circle", Path::Circle)
        .MethodStatic("RRect", Path::RRect)
        .MethodStatic("Polygon", Path::Polygon)
        .MethodStatic("Line", Path::Line)
        M(clone)
        M(equalTo)
        M(isInterpolatable)
        M(interpolate)
        .Property<ffi::Enum<SkPathFillType>>("fillType", &Path::getFillType, &Path::setFillType)
        M(toggleInverseFillType)
        .Property<bool>("isConvex", &Path::getIsConvex, nullptr)
        M(asOval)
        M(asRRect)
        M(reset)
        M(rewind)
        .Property<bool>("isEmpty", &Path::getIsEmpty, nullptr)
        .Property<bool>("isLastContourClosed", &Path::getIsLastContourClosed, nullptr)
        .Property<bool>("isFinite", &Path::getIsFinite, nullptr)
        .Property<bool>("isVolatile", &Path::getIsVolatile, &Path::setIsVolatile)
        M(countPoints)
        M(getPoint)
        M(getPoints)
        M(countVerbs)
        M(getVerbs)
        .Property<v8::Local<v8::Value>>("roughBounds", &Path::getRoughBounds, nullptr)
        M(computeTightBounds)
        M(conservativelyContainsRect)
        M(asRect)
        M(addPathOffset)
        M(addPath)
        M(reverseAddPath)
        M(transform)
        M(makeTransform)
        .Property<uint32_t>("segmentMasks", &Path::getSegmentMasks, nullptr)
        M(contains)
        M(fillWithPaint)
        M(serialize)
        M(serializeToMemory)
        .MethodStatic("Deserialize", Path::Deserialize)
        .Property<uint32_t>("generationID", &Path::getGenerationID, nullptr)
        M(isValid)
        .Finalize(ctx);
#undef M

#define M(v)  .Method(#v, &PathBuilder::v)
    exports["PathBuilder"] = ffi::DefineClass<PathBuilder>(isolate)
        .Constructor()
        M(clone)
        M(computeBounds)
        M(snapshot)
        M(detach)
        M(setFillType)
        M(setIsVolatile)
        M(reset)
        M(moveTo)
        M(lineTo)
        M(quadTo)
        M(conicTo)
        M(cubicTo)
        M(close)
        M(rLineTo)
        M(rQuadTo)
        M(rConicTo)
        M(rCubicTo)
        M(ovalArcTo)
        M(tangentArcTo)
        M(rotateOvalArcTo)
        M(addArc)
        M(addRect)
        M(addOval)
        M(addRRect)
        M(addCircle)
        M(addPolygon)
        M(addPath)
        M(incReserve)
        M(offset)
        M(toggleInverseFillType)
        .Finalize(ctx);
#undef M

    exports["Flattenable"] = ffi::DefineClass<Flattenable>(isolate)
        .Method("serialize", &Flattenable::serialize)
        .Finalize(ctx);

#define MS(v)  .MethodStatic(#v, PathEffect::v)
    exports["PathEffect"] = ffi::DefineClass<PathEffect>(isolate)
        .Inherit<Flattenable>()
        MS(MakeSum)
        MS(MakeCompose)
        MS(Make1D)
        MS(MakeLine2D)
        MS(MakePath2D)
        MS(MakeCorner)
        MS(MakeTrim)
        MS(MakeDiscrete)
        MS(MakeDash)
        MS(Deserialize)
        .Finalize(ctx);
#undef MS

#define MS(v)  .MethodStatic(#v, ImageFilter::v)
    exports["ImageFilter"] = ffi::DefineClass<ImageFilter>(isolate)
        .Inherit<Flattenable>()
        MS(Arithmetic)
        MS(Blend)
        MS(Blender)
        MS(Blur)
        MS(Compose)
        MS(ColorFilter)
        MS(Crop)
        MS(DisplacementMap)
        MS(DropShadow)
        MS(DropShadowOnly)
        MS(Empty)
        MS(Image)
        MS(Magnifier)
        MS(MatrixConvolution)
        MS(MatrixTransform)
        MS(Merge)
        MS(Offset)
        MS(Picture)
        MS(Shader)
        MS(Tile)
        MS(Dilate)
        MS(Erode)
        MS(DistantLitDiffuse)
        MS(PointLitDiffuse)
        MS(SpotLitDiffuse)
        MS(DistantLitSpecular)
        MS(PointLitSpecular)
        MS(SpotLitSpecular)
        MS(Deserialize)
        .Method("filterBounds", &ImageFilter::filterBounds)
        .Method("computeFastBounds", &ImageFilter::computeFastBounds)
        .Method("makeWithLocalMatrix", &ImageFilter::makeWithLocalMatrix)
        .Property<bool>("canComputeFastBounds", &ImageFilter::getCanComputeFastBounds, nullptr)
        .Finalize(ctx);
#undef MS

#define MS(v)  .MethodStatic(#v, ColorFilter::v)
    exports["ColorFilter"] = ffi::DefineClass<ColorFilter>(isolate)
        .Inherit<Flattenable>()
        MS(Compose)
        MS(Blend)
        MS(Matrix)
        MS(HSLAMatrix)
        MS(LinearToSRGBGamma)
        MS(SRGBToLinearGamma)
        MS(Lerp)
        MS(Table)
        MS(TableARGB)
        MS(Lighting)
        MS(HighContrast)
        MS(Luma)
        MS(Overdraw)
        MS(Deserialize)
        .Method("asAColorMode", &ColorFilter::asAColorMode)
        .Method("asAColorMatrix", &ColorFilter::asAColorMatrix)
        .Method("filterColor4f", &ColorFilter::filterColor4f)
        .Method("makeComposed", &ColorFilter::makeComposed)
        .Method("makeWithWorkingColorSpace", &ColorFilter::makeWithWorkingColorSpace)
        .Finalize(ctx);
#undef MS

#define MS(v)   .MethodStatic(#v, Shader::v)
    exports["Shader"] = ffi::DefineClass<Shader>(isolate)
        .Inherit<Flattenable>()
        MS(Empty)
        MS(Color)
        MS(Blend)
        MS(Blender)
        MS(CoordClamp)
        MS(Image)
        MS(RawImage)
        MS(LinearGradient)
        MS(RadialGradient)
        MS(TwoPointConicalGradient)
        MS(SweepGradient)
        MS(FractalNoise)
        MS(Turbulence)
        MS(Deserialize)
        .Property<bool>("isOpaque", &Shader::getIsOpaque, nullptr)
        .Method("makeWithLocalMatrix", &Shader::makeWithLocalMatrix)
        .Method("makeWithColorFilter", &Shader::makeWithColorFilter)
        .Method("makeWithWorkingColorSpace", &Shader::makeWithWorkingColorSpace)
        .Method("dispose", &Shader::dispose)
        .Finalize(ctx);
#undef MS

    exports["Blender"] = ffi::DefineClass<Blender>(isolate)
        .Inherit<Flattenable>()
        .MethodStatic("Mode", Blender::Mode)
        .MethodStatic("Arithmetic", Blender::Arithmetic)
        .MethodStatic("Deserialize", Blender::Deserialize)
        .Finalize(ctx);

#define MS(v) .MethodStatic(#v, RuntimeEffect::v)
#define M(v)  .Method(#v, &RuntimeEffect::v)
    exports["RuntimeEffect"] = ffi::DefineClass<RuntimeEffect>(isolate)
        MS(CompileColorFilter)
        MS(CompileShader)
        MS(CompileBlender)
        M(makeShader)
        M(makeColorFilter)
        M(makeBlender)
        .Property<v8::Local<v8::Value>>("source", &RuntimeEffect::getSource, nullptr)
        .Property<uint32_t>("uniformsByteSize", &RuntimeEffect::getUniformsByteSize, nullptr)
        .Property<v8::Local<v8::Value>>("uniforms", &RuntimeEffect::getUniforms, nullptr)
        .Property<v8::Local<v8::Value>>("children", &RuntimeEffect::getChildren, nullptr)
        .Property<bool>("allowShader", &RuntimeEffect::getAllowShader, nullptr)
        .Property<bool>("allowColorFilter", &RuntimeEffect::getAllowColorFilter, nullptr)
        .Property<bool>("allowBlender", &RuntimeEffect::getAllowBlender, nullptr)
        M(findUniform)
        M(findChild)
        .Finalize(ctx);
#undef M
#undef MS

    exports["Picture"] = ffi::DefineClass<Picture>(isolate)
        .Inherit<Flattenable>()
        .MethodStatic("Deserialize", Picture::Deserialize)
        .MethodStatic("MakePlaceholder", Picture::MakePlaceholder)
        .Method("playback", &Picture::playback)
        .Method("makeShader", &Picture::makeShader)
        .Method("approximateOpCount", &Picture::approximateOpCount)
        .Method("approximateBytesUsed", &Picture::approximateBytesUsed)
        .Property<v8::Local<v8::Value>>("cullRect", &Picture::getCullRect, nullptr)
        .Property<uint32_t>("uniqueID", &Picture::getUniqueID, nullptr)
        .Finalize(ctx);

    exports["PictureRecorder"] = ffi::DefineClass<PictureRecorder>(isolate)
        .Constructor()
        .Method("beginRecording", &PictureRecorder::beginRecording)
        .Method("getRecordingCanvas", &PictureRecorder::getRecordingCanvas)
        .Method("finishRecordingAsPicture", &PictureRecorder::finishRecordingAsPicture)
        .Method("finishRecordingAsPictureWithCull", &PictureRecorder::finishRecordingAsPictureWithCull)
        .Finalize(ctx);

    exports["FontStyleSet"] = ffi::DefineClass<FontStyleSet>(isolate)
        .MethodStatic("CreateEmpty", FontStyleSet::CreateEmpty)
        .Method("getStyle", &FontStyleSet::getStyle)
        .Method("createTypeface", &FontStyleSet::createTypeface)
        .Method("matchStyle", &FontStyleSet::matchStyle)
        .Property<int32_t>("count", &FontStyleSet::getCount, nullptr)
        .Finalize(ctx);

    exports["FontMgr"] = ffi::DefineClass<FontMgr>(isolate)
        .MethodStatic("GetGlobal", FontMgr::GetGlobal)
        .MethodStatic("GetEmpty", FontMgr::GetEmpty)
        .Method("countFamilies", &FontMgr::countFamilies)
        .Method("getFamilyName", &FontMgr::getFamilyName)
        .Method("createStyleSet", &FontMgr::createStyleSet)
        .Method("matchFamily", &FontMgr::matchFamily)
        .Method("matchFamilyStyle", &FontMgr::matchFamilyStyle)
        .Method("matchFamilyStyleCharacter", &FontMgr::matchFamilyStyleCharacter)
        .Method("makeFromData", &FontMgr::makeFromData)
        .Method("makeFromFile", &FontMgr::makeFromFile)
        .Finalize(ctx);

    exports["Typeface"] = ffi::DefineClass<Typeface>(isolate)
        .MethodStatic("MakeEmpty", Typeface::MakeEmpty)
        .MethodStatic("Deserialize", Typeface::Deserialize)
        .Method("getLocalizedFamilyNames", &Typeface::getLocalizedFamilyNames)
        .Method("getVariationDesignPosition", &Typeface::getVariationDesignPosition)
        .Method("getVariationDesignParameters", &Typeface::getVariationDesignParameters)
        .Property<v8::Local<v8::Value>>("fontStyle", &Typeface::getFontStyle, nullptr)
        .Property<bool>("isFixedPitch", &Typeface::getIsFixedPitch, nullptr)
        .Property<uint32_t>("uniqueID", &Typeface::getUniqueID, nullptr)
        .Method("equalTo", &Typeface::equalTo)
        .Method("makeClone", &Typeface::makeClone)
        .Method("serialize", &Typeface::serialize)
        .Method("unicharsToGlyphs", &Typeface::unicharsToGlyphs)
        .Method("unicharToGlyph", &Typeface::unicharToGlyph)
        .Method("textToGlyphs", &Typeface::textToGlyphs)
        .Property<int32_t>("glyphsCount", &Typeface::getGlyphsCount, nullptr)
        .Property<int32_t>("tablesCount", &Typeface::getTablesCount, nullptr)
        .Property<v8::Local<v8::Value>>("tableTags", &Typeface::getTableTags, nullptr)
        .Method("getTableSize", &Typeface::getTableSize)
        .Method("copyTableDataTo", &Typeface::copyTableDataTo)
        .Property<int32_t>("unitsPerEm", &Typeface::getUnitsPerEm, nullptr)
        .Method("getKerningPairAdjustments", &Typeface::getKerningPairAdjustments)
        .Property<v8::Local<v8::Value>>("familyName", &Typeface::getFamilyName, nullptr)
        .Property<v8::Local<v8::Value>>("postScriptName", &Typeface::getPostScriptName, nullptr)
        .Property<v8::Local<v8::Value>>("bounds", &Typeface::getBounds, nullptr)
        .Finalize(ctx);

#define REGISTER_RW_PROPS(name, type) .Property<type>(#name, &Font::get_##name, &Font::set_##name)

    exports["Font"] = ffi::DefineClass<Font>(isolate)
        .Constructor<const ffi::Class<Typeface>&, float, float, float>()
        .Method("equalTo", &Font::equalTo)
        FONT_RW_PROPERTIES_MAP(REGISTER_RW_PROPS)
        .Method("makeWithSize", &Font::makeWithSize)
        .Method("measureText", &Font::measureText)
        .Method("measureGlyphs", &Font::measureGlyphs)
        .Method("getPos", &Font::getPos)
        .Method("getXPos", &Font::getXPos)
        .Method("getPath", &Font::getPath)
        .Method("getIntercepts", &Font::getIntercepts)
        .Property<v8::Local<v8::Value>>("metrics", &Font::getMetrics, nullptr)
        .Property<float>("spacing", &Font::getSpacing, nullptr)
        .Finalize(ctx);
#undef REGISTER_RW_PROPS

    exports["Vertices"] = ffi::DefineClass<Vertices>(isolate)
        .MethodStatic("MakeCopy", Vertices::MakeCopy)
        .Property<uint32_t>("uniqueID", &Vertices::get_uniqueID, nullptr)
        .Property<size_t>("approximateSize", &Vertices::get_approximateSize, nullptr)
        .Property<v8::Local<v8::Value>>("bounds", &Vertices::get_bounds, nullptr)
        .Finalize(ctx);


    // Functions

#define EXPORT_FUNC_PURE(v) \
    exports[#v] = ffi::WrapFunctionAddress(isolate, v8::SideEffectType::kHasNoSideEffect, v) \
                  ->GetFunction(ctx).ToLocalChecked()

    EXPORT_FUNC_PURE(ColorTypeBytesPerPixel);
    EXPORT_FUNC_PURE(ColorTypeIsAlwaysOpaque);


    // Considering the overhead of calling into C++ from JavaScript, some performance-sensitive
    // objects (like `Rect`, `Matrix`) are implemented in JavaScript. Export those objects:
    InsertScriptExports(isolate, "internal://natives/renderer.js", exports);

    // Store the JS constructors exported by `renderer.js` so that they can be fast accessed
    // when we need to construct a JS-implemented object from C++.
    auto *storage = ffi::TypeContext::FromIsolate(isolate)->GetValueStorage();
    storage->Store(FFI_GVSTORE_USE_ID(ctor_Rect), exports["Rect"]);
    storage->Store(FFI_GVSTORE_USE_ID(ctor_Mat3x3), exports["Mat3x3"]);
    storage->Store(FFI_GVSTORE_USE_ID(ctor_Mat4x4), exports["Mat4x4"]);

    return exports;
}

GALLIUM_BINDINGS_NS_END
