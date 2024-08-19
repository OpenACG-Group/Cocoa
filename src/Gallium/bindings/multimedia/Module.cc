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

#include "Gallium/ffi/Enum.h"
#include "Gallium/ffi/DefineClass.h"
#include "Gallium/bindings/multimedia/Module.h"
#include "Gallium/bindings/multimedia/MediaInputOutput.h"
#include "Gallium/bindings/multimedia/AChannelLayout.h"
#include "Gallium/bindings/multimedia/FormatDemuxer.h"
#include "Gallium/bindings/multimedia/CodecParameters.h"
#include "Gallium/bindings/multimedia/Packet.h"
#include "Gallium/bindings/multimedia/CodecContext.h"
#include "Gallium/bindings/multimedia/Frame.h"
#include "Gallium/bindings/multimedia/PixelFrameFactory.h"
#include "Gallium/bindings/multimedia/PixelFrameView.h"
#include "Gallium/bindings/multimedia/AudioSinkStream.h"
#include "Gallium/bindings/multimedia/AudioStreamService.h"
#include "Gallium/bindings/multimedia/FrameScheduler.h"
#include "Gallium/bindings/multimedia/HWFramesContext.h"
#include "Gallium/bindings/multimedia/HWDeviceContext.h"
#include "Gallium/bindings/multimedia/FilterGraph.h"
GALLIUM_BINDINGS_NS_BEGIN

FFI_GVSTORE_DEFINE_ID(ctor_Rational)

MultimediaModule::MultimediaModule()
    : ffi::NativeModule("multimedia", "Video and audio processing", { "renderer" })
{
}

ffi::LocalExports MultimediaModule::OnBuildExports(v8::Isolate *isolate)
{
    using namespace multimedia;
    ffi::LocalExports exports;
    v8::Local<v8::Context> jsctx = isolate->GetCurrentContext();

    // TODO(sora): export this by `tsdecl-scanner.py`
    //>! TSDecl: @constant TIME_BASE: number
    exports["TIME_BASE"] = v8::Number::New(isolate, AV_TIME_BASE);

    //! TSDecl: @enum SeekWhence
    exports["SeekWhence"] = ffi::DefineEnum<SeekWhence>(isolate, "SeekWhence", false)
        .Item("Set", SeekWhence::kSet)  //! TSDecl: @enumitem Set
        .Item("Cur", SeekWhence::kCur)  //! TSDecl: @enumitem Cur
        .Item("End", SeekWhence::kEnd)  //! TSDecl: @enumitem End
        .Finalize();
    //! TSDecl: @end

    //! TSDecl: @enum MediaIOFlags
    exports["MediaIOFlags"] = ffi::DefineEnum<MediaIOFlags>(isolate, "MediaIOFlags", true)
        .Item("Read", MediaIOFlags::kRead)              //! TSDecl: @enumitem Read
        .Item("Write", MediaIOFlags::kWrite)            //! TSDecl: @enumitem Write
        .Item("ReadWrite", MediaIOFlags::kReadWrite)    //! TSDecl: @enumitem ReadWrite
        .Item("Direct", MediaIOFlags::kDirect)          //! TSDecl: @enumitem Direct
        .Item("NonBlock", MediaIOFlags::kNonBlock)      //! TSDecl: @enumitem NonBlock
        .Finalize();
    //! TSDecl: @end

    exports["MediaType"] = ffi::DefineEnum<MediaType>(isolate, "MediaType", false)
        .Item("Unknown", MediaType::kUnknown)
        .Item("Audio", MediaType::kAudio)
        .Item("Video", MediaType::kVideo)
        .Item("Attachment", MediaType::kAttachment)
        .Item("Data", MediaType::kData)
        .Item("Subtitle", MediaType::kSubtitle)
        .Finalize();

    exports["StreamDisposition"] = ffi::DefineEnum<StreamDisposition>(isolate, "StreamDisposition", true)
        .Item("Default", StreamDisposition::kDefault)
        .Item("Dub", StreamDisposition::kDub)
        .Item("Original", StreamDisposition::kOriginal)
        .Item("Comment", StreamDisposition::kComment)
        .Item("Lyrics", StreamDisposition::kLyrics)
        .Item("Karaoke", StreamDisposition::kKaraoke)
        .Item("Forced", StreamDisposition::kForced)
        .Item("HearingImpaired", StreamDisposition::kHearingImpaired)
        .Item("VisualImpaired", StreamDisposition::kVisualImpaired)
        .Item("CleanEffects", StreamDisposition::kCleanEffects)
        .Item("AttachedPic", StreamDisposition::kAttachedPic)
        .Item("TimedThumbnails", StreamDisposition::kTimedThumbnails)
        .Item("NonDiegetic", StreamDisposition::kNonDiegetic)
        .Item("Captions", StreamDisposition::kCaptions)
        .Item("Descriptions", StreamDisposition::kDescriptions)
        .Item("Metadata", StreamDisposition::kMetadata)
        .Item("Dependent", StreamDisposition::kDependent)
        .Item("StillImage", StreamDisposition::kStillImage)
        .Finalize();
    
    exports["PixelFormat"] = ffi::DefineEnum<PixelFormat>(isolate, "PixelFormat", false)
        .Item("kNone", PixelFormat::kNone)
        .Item("kYUV420P", PixelFormat::kYUV420P)
        .Item("kYUYV422", PixelFormat::kYUYV422)
        .Item("kRGB24", PixelFormat::kRGB24)
        .Item("kBGR24", PixelFormat::kBGR24)
        .Item("kYUV422P", PixelFormat::kYUV422P)
        .Item("kYUV444P", PixelFormat::kYUV444P)
        .Item("kYUV410P", PixelFormat::kYUV410P)
        .Item("kYUV411P", PixelFormat::kYUV411P)
        .Item("kGRAY8", PixelFormat::kGRAY8)
        .Item("kMONOWHITE", PixelFormat::kMONOWHITE)
        .Item("kMONOBLACK", PixelFormat::kMONOBLACK)
        .Item("kPAL8", PixelFormat::kPAL8)
        .Item("kUYVY422", PixelFormat::kUYVY422)
        .Item("kUYYVYY411", PixelFormat::kUYYVYY411)
        .Item("kBGR8", PixelFormat::kBGR8)
        .Item("kBGR4", PixelFormat::kBGR4)
        .Item("kBGR4_BYTE", PixelFormat::kBGR4_BYTE)
        .Item("kRGB8", PixelFormat::kRGB8)
        .Item("kRGB4", PixelFormat::kRGB4)
        .Item("kRGB4_BYTE", PixelFormat::kRGB4_BYTE)
        .Item("kNV12", PixelFormat::kNV12)
        .Item("kNV21", PixelFormat::kNV21)
        .Item("kARGB", PixelFormat::kARGB)
        .Item("kRGBA", PixelFormat::kRGBA)
        .Item("kABGR", PixelFormat::kABGR)
        .Item("kBGRA", PixelFormat::kBGRA)
        .Item("kGRAY16BE", PixelFormat::kGRAY16BE)
        .Item("kGRAY16LE", PixelFormat::kGRAY16LE)
        .Item("kYUV440P", PixelFormat::kYUV440P)
        .Item("kYUVA420P", PixelFormat::kYUVA420P)
        .Item("kRGB48BE", PixelFormat::kRGB48BE)
        .Item("kRGB48LE", PixelFormat::kRGB48LE)
        .Item("kRGB565BE", PixelFormat::kRGB565BE)
        .Item("kRGB565LE", PixelFormat::kRGB565LE)
        .Item("kRGB555BE", PixelFormat::kRGB555BE)
        .Item("kRGB555LE", PixelFormat::kRGB555LE)
        .Item("kBGR565BE", PixelFormat::kBGR565BE)
        .Item("kBGR565LE", PixelFormat::kBGR565LE)
        .Item("kBGR555BE", PixelFormat::kBGR555BE)
        .Item("kBGR555LE", PixelFormat::kBGR555LE)
        .Item("kYUV420P16LE", PixelFormat::kYUV420P16LE)
        .Item("kYUV420P16BE", PixelFormat::kYUV420P16BE)
        .Item("kYUV422P16LE", PixelFormat::kYUV422P16LE)
        .Item("kYUV422P16BE", PixelFormat::kYUV422P16BE)
        .Item("kYUV444P16LE", PixelFormat::kYUV444P16LE)
        .Item("kYUV444P16BE", PixelFormat::kYUV444P16BE)
        .Item("kDXVA2_VLD", PixelFormat::kDXVA2_VLD)
        .Item("kRGB444LE", PixelFormat::kRGB444LE)
        .Item("kRGB444BE", PixelFormat::kRGB444BE)
        .Item("kBGR444LE", PixelFormat::kBGR444LE)
        .Item("kBGR444BE", PixelFormat::kBGR444BE)
        .Item("kYA8", PixelFormat::kYA8)
        .Item("kY400A", PixelFormat::kY400A)
        .Item("kGRAY8A", PixelFormat::kGRAY8A)
        .Item("kBGR48BE", PixelFormat::kBGR48BE)
        .Item("kBGR48LE", PixelFormat::kBGR48LE)
        .Item("kYUV420P9BE", PixelFormat::kYUV420P9BE)
        .Item("kYUV420P9LE", PixelFormat::kYUV420P9LE)
        .Item("kYUV420P10BE", PixelFormat::kYUV420P10BE)
        .Item("kYUV420P10LE", PixelFormat::kYUV420P10LE)
        .Item("kYUV422P10BE", PixelFormat::kYUV422P10BE)
        .Item("kYUV422P10LE", PixelFormat::kYUV422P10LE)
        .Item("kYUV444P9BE", PixelFormat::kYUV444P9BE)
        .Item("kYUV444P9LE", PixelFormat::kYUV444P9LE)
        .Item("kYUV444P10BE", PixelFormat::kYUV444P10BE)
        .Item("kYUV444P10LE", PixelFormat::kYUV444P10LE)
        .Item("kYUV422P9BE", PixelFormat::kYUV422P9BE)
        .Item("kYUV422P9LE", PixelFormat::kYUV422P9LE)
        .Item("kGBRP", PixelFormat::kGBRP)
        .Item("kGBR24P", PixelFormat::kGBR24P)
        .Item("kGBRP9BE", PixelFormat::kGBRP9BE)
        .Item("kGBRP9LE", PixelFormat::kGBRP9LE)
        .Item("kGBRP10BE", PixelFormat::kGBRP10BE)
        .Item("kGBRP10LE", PixelFormat::kGBRP10LE)
        .Item("kGBRP16BE", PixelFormat::kGBRP16BE)
        .Item("kGBRP16LE", PixelFormat::kGBRP16LE)
        .Item("kYUVA422P", PixelFormat::kYUVA422P)
        .Item("kYUVA444P", PixelFormat::kYUVA444P)
        .Item("kYUVA420P9BE", PixelFormat::kYUVA420P9BE)
        .Item("kYUVA420P9LE", PixelFormat::kYUVA420P9LE)
        .Item("kYUVA422P9BE", PixelFormat::kYUVA422P9BE)
        .Item("kYUVA422P9LE", PixelFormat::kYUVA422P9LE)
        .Item("kYUVA444P9BE", PixelFormat::kYUVA444P9BE)
        .Item("kYUVA444P9LE", PixelFormat::kYUVA444P9LE)
        .Item("kYUVA420P10BE", PixelFormat::kYUVA420P10BE)
        .Item("kYUVA420P10LE", PixelFormat::kYUVA420P10LE)
        .Item("kYUVA422P10BE", PixelFormat::kYUVA422P10BE)
        .Item("kYUVA422P10LE", PixelFormat::kYUVA422P10LE)
        .Item("kYUVA444P10BE", PixelFormat::kYUVA444P10BE)
        .Item("kYUVA444P10LE", PixelFormat::kYUVA444P10LE)
        .Item("kYUVA420P16BE", PixelFormat::kYUVA420P16BE)
        .Item("kYUVA420P16LE", PixelFormat::kYUVA420P16LE)
        .Item("kYUVA422P16BE", PixelFormat::kYUVA422P16BE)
        .Item("kYUVA422P16LE", PixelFormat::kYUVA422P16LE)
        .Item("kYUVA444P16BE", PixelFormat::kYUVA444P16BE)
        .Item("kYUVA444P16LE", PixelFormat::kYUVA444P16LE)
        .Item("kXYZ12LE", PixelFormat::kXYZ12LE)
        .Item("kXYZ12BE", PixelFormat::kXYZ12BE)
        .Item("kNV16", PixelFormat::kNV16)
        .Item("kNV20LE", PixelFormat::kNV20LE)
        .Item("kNV20BE", PixelFormat::kNV20BE)
        .Item("kRGBA64BE", PixelFormat::kRGBA64BE)
        .Item("kRGBA64LE", PixelFormat::kRGBA64LE)
        .Item("kBGRA64BE", PixelFormat::kBGRA64BE)
        .Item("kBGRA64LE", PixelFormat::kBGRA64LE)
        .Item("kYVYU422", PixelFormat::kYVYU422)
        .Item("kYA16BE", PixelFormat::kYA16BE)
        .Item("kYA16LE", PixelFormat::kYA16LE)
        .Item("kGBRAP", PixelFormat::kGBRAP)
        .Item("kGBRAP16BE", PixelFormat::kGBRAP16BE)
        .Item("kGBRAP16LE", PixelFormat::kGBRAP16LE)
        .Item("k0RGB", PixelFormat::k0RGB)
        .Item("kRGB0", PixelFormat::kRGB0)
        .Item("k0BGR", PixelFormat::k0BGR)
        .Item("kBGR0", PixelFormat::kBGR0)
        .Item("kYUV420P12BE", PixelFormat::kYUV420P12BE)
        .Item("kYUV420P12LE", PixelFormat::kYUV420P12LE)
        .Item("kYUV420P14BE", PixelFormat::kYUV420P14BE)
        .Item("kYUV420P14LE", PixelFormat::kYUV420P14LE)
        .Item("kYUV422P12BE", PixelFormat::kYUV422P12BE)
        .Item("kYUV422P12LE", PixelFormat::kYUV422P12LE)
        .Item("kYUV422P14BE", PixelFormat::kYUV422P14BE)
        .Item("kYUV422P14LE", PixelFormat::kYUV422P14LE)
        .Item("kYUV444P12BE", PixelFormat::kYUV444P12BE)
        .Item("kYUV444P12LE", PixelFormat::kYUV444P12LE)
        .Item("kYUV444P14BE", PixelFormat::kYUV444P14BE)
        .Item("kYUV444P14LE", PixelFormat::kYUV444P14LE)
        .Item("kGBRP12BE", PixelFormat::kGBRP12BE)
        .Item("kGBRP12LE", PixelFormat::kGBRP12LE)
        .Item("kGBRP14BE", PixelFormat::kGBRP14BE)
        .Item("kGBRP14LE", PixelFormat::kGBRP14LE)
        .Item("kBAYER_BGGR8", PixelFormat::kBAYER_BGGR8)
        .Item("kBAYER_RGGB8", PixelFormat::kBAYER_RGGB8)
        .Item("kBAYER_GBRG8", PixelFormat::kBAYER_GBRG8)
        .Item("kBAYER_GRBG8", PixelFormat::kBAYER_GRBG8)
        .Item("kBAYER_BGGR16LE", PixelFormat::kBAYER_BGGR16LE)
        .Item("kBAYER_BGGR16BE", PixelFormat::kBAYER_BGGR16BE)
        .Item("kBAYER_RGGB16LE", PixelFormat::kBAYER_RGGB16LE)
        .Item("kBAYER_RGGB16BE", PixelFormat::kBAYER_RGGB16BE)
        .Item("kBAYER_GBRG16LE", PixelFormat::kBAYER_GBRG16LE)
        .Item("kBAYER_GBRG16BE", PixelFormat::kBAYER_GBRG16BE)
        .Item("kBAYER_GRBG16LE", PixelFormat::kBAYER_GRBG16LE)
        .Item("kBAYER_GRBG16BE", PixelFormat::kBAYER_GRBG16BE)
        .Item("kYUV440P10LE", PixelFormat::kYUV440P10LE)
        .Item("kYUV440P10BE", PixelFormat::kYUV440P10BE)
        .Item("kYUV440P12LE", PixelFormat::kYUV440P12LE)
        .Item("kYUV440P12BE", PixelFormat::kYUV440P12BE)
        .Item("kAYUV64LE", PixelFormat::kAYUV64LE)
        .Item("kAYUV64BE", PixelFormat::kAYUV64BE)
        .Item("kP010LE", PixelFormat::kP010LE)
        .Item("kP010BE", PixelFormat::kP010BE)
        .Item("kGBRAP12BE", PixelFormat::kGBRAP12BE)
        .Item("kGBRAP12LE", PixelFormat::kGBRAP12LE)
        .Item("kGBRAP10BE", PixelFormat::kGBRAP10BE)
        .Item("kGBRAP10LE", PixelFormat::kGBRAP10LE)
        .Item("kGRAY12BE", PixelFormat::kGRAY12BE)
        .Item("kGRAY12LE", PixelFormat::kGRAY12LE)
        .Item("kGRAY10BE", PixelFormat::kGRAY10BE)
        .Item("kGRAY10LE", PixelFormat::kGRAY10LE)
        .Item("kP016LE", PixelFormat::kP016LE)
        .Item("kP016BE", PixelFormat::kP016BE)
        .Item("kGRAY9BE", PixelFormat::kGRAY9BE)
        .Item("kGRAY9LE", PixelFormat::kGRAY9LE)
        .Item("kGBRPF32BE", PixelFormat::kGBRPF32BE)
        .Item("kGBRPF32LE", PixelFormat::kGBRPF32LE)
        .Item("kGBRAPF32BE", PixelFormat::kGBRAPF32BE)
        .Item("kGBRAPF32LE", PixelFormat::kGBRAPF32LE)
        .Item("kGRAY14BE", PixelFormat::kGRAY14BE)
        .Item("kGRAY14LE", PixelFormat::kGRAY14LE)
        .Item("kGRAYF32BE", PixelFormat::kGRAYF32BE)
        .Item("kGRAYF32LE", PixelFormat::kGRAYF32LE)
        .Item("kYUVA422P12BE", PixelFormat::kYUVA422P12BE)
        .Item("kYUVA422P12LE", PixelFormat::kYUVA422P12LE)
        .Item("kYUVA444P12BE", PixelFormat::kYUVA444P12BE)
        .Item("kYUVA444P12LE", PixelFormat::kYUVA444P12LE)
        .Item("kNV24", PixelFormat::kNV24)
        .Item("kNV42", PixelFormat::kNV42)
        .Item("kY210BE", PixelFormat::kY210BE)
        .Item("kY210LE", PixelFormat::kY210LE)
        .Item("kX2RGB10LE", PixelFormat::kX2RGB10LE)
        .Item("kX2RGB10BE", PixelFormat::kX2RGB10BE)
        .Item("kX2BGR10LE", PixelFormat::kX2BGR10LE)
        .Item("kX2BGR10BE", PixelFormat::kX2BGR10BE)
        .Item("kP210BE", PixelFormat::kP210BE)
        .Item("kP210LE", PixelFormat::kP210LE)
        .Item("kP410BE", PixelFormat::kP410BE)
        .Item("kP410LE", PixelFormat::kP410LE)
        .Item("kP216BE", PixelFormat::kP216BE)
        .Item("kP216LE", PixelFormat::kP216LE)
        .Item("kP416BE", PixelFormat::kP416BE)
        .Item("kP416LE", PixelFormat::kP416LE)
        .Item("kVUYA", PixelFormat::kVUYA)
        .Item("kRGBAF16BE", PixelFormat::kRGBAF16BE)
        .Item("kRGBAF16LE", PixelFormat::kRGBAF16LE)
        .Item("kVUYX", PixelFormat::kVUYX)
        .Item("kP012LE", PixelFormat::kP012LE)
        .Item("kP012BE", PixelFormat::kP012BE)
        .Item("kY212BE", PixelFormat::kY212BE)
        .Item("kY212LE", PixelFormat::kY212LE)
        .Item("kXV30BE", PixelFormat::kXV30BE)
        .Item("kXV30LE", PixelFormat::kXV30LE)
        .Item("kXV36BE", PixelFormat::kXV36BE)
        .Item("kXV36LE", PixelFormat::kXV36LE)
        .Item("kRGBF32BE", PixelFormat::kRGBF32BE)
        .Item("kRGBF32LE", PixelFormat::kRGBF32LE)
        .Item("kRGBAF32BE", PixelFormat::kRGBAF32BE)
        .Item("kRGBAF32LE", PixelFormat::kRGBAF32LE)
        .Finalize();

    exports["FieldOrder"] = ffi::DefineEnum<FieldOrder>(isolate, "FieldOrder", false)
        .Item("Unknown", FieldOrder::kUnknown)
        .Item("Progressive", FieldOrder::kProgressive)
        .Item("TT", FieldOrder::kTT)
        .Item("BB", FieldOrder::kBB)
        .Item("BT", FieldOrder::kBT)
        .Item("TB", FieldOrder::kTB)
        .Finalize();

    exports["ColorPrimaries"] = ffi::DefineEnum<ColorPrimaries>(isolate, "ColorPrimaries", false)
        .Item("Reserved0", ColorPrimaries::kReserved0)
        .Item("Unspecified", ColorPrimaries::kUnspecified)
        .Item("Reserved", ColorPrimaries::kReserved)
        .Item("BT709", ColorPrimaries::kBT709)
        .Item("BT470M", ColorPrimaries::kBT470M)
        .Item("BT470BG", ColorPrimaries::kBT470BG)
        .Item("SMPTE170M", ColorPrimaries::kSMPTE170M)
        .Item("SMPTE240M", ColorPrimaries::kSMPTE240M)
        .Item("FILM", ColorPrimaries::kFILM)
        .Item("BT2020", ColorPrimaries::kBT2020)
        .Item("SMPTE428", ColorPrimaries::kSMPTE428)
        .Item("SMPTEST428_1", ColorPrimaries::kSMPTEST428_1)
        .Item("SMPTE431", ColorPrimaries::kSMPTE431)
        .Item("SMPTE432", ColorPrimaries::kSMPTE432)
        .Item("EBU3213", ColorPrimaries::kEBU3213)
        .Item("JEDEC_P22", ColorPrimaries::kJEDEC_P22)
        .Finalize();

    exports["ColorTransferCharacteristic"]
        = ffi::DefineEnum<ColorTransferCharacteristic>(isolate, "ColorTransferCharacteristic", false)
        .Item("Reserved0", ColorTransferCharacteristic::kReserved0)
        .Item("Unspecified", ColorTransferCharacteristic::kUnspecified)
        .Item("Reserved", ColorTransferCharacteristic::kReserved)
        .Item("BT709", ColorTransferCharacteristic::kBT709)
        .Item("GAMMA22", ColorTransferCharacteristic::kGAMMA22)
        .Item("GAMMA28", ColorTransferCharacteristic::kGAMMA28)
        .Item("SMPTE170M", ColorTransferCharacteristic::kSMPTE170M)
        .Item("SMPTE240M", ColorTransferCharacteristic::kSMPTE240M)
        .Item("LINEAR", ColorTransferCharacteristic::kLINEAR)
        .Item("LOG", ColorTransferCharacteristic::kLOG)
        .Item("LOG_SQRT", ColorTransferCharacteristic::kLOG_SQRT)
        .Item("IEC61966_2_4", ColorTransferCharacteristic::kIEC61966_2_4)
        .Item("BT1361_ECG", ColorTransferCharacteristic::kBT1361_ECG)
        .Item("IEC61966_2_1", ColorTransferCharacteristic::kIEC61966_2_1)
        .Item("BT2020_10", ColorTransferCharacteristic::kBT2020_10)
        .Item("BT2020_12", ColorTransferCharacteristic::kBT2020_12)
        .Item("SMPTE2084", ColorTransferCharacteristic::kSMPTE2084)
        .Item("SMPTEST2084", ColorTransferCharacteristic::kSMPTEST2084)
        .Item("SMPTE428", ColorTransferCharacteristic::kSMPTE428)
        .Item("SMPTEST428_1", ColorTransferCharacteristic::kSMPTEST428_1)
        .Item("ARIB_STD_B67", ColorTransferCharacteristic::kARIB_STD_B67)
        .Finalize();

    exports["ColorSpace"] = ffi::DefineEnum<ColorSpace>(isolate, "ColorSpace", false)
        .Item("RGB", ColorSpace::kRGB)
        .Item("BT709", ColorSpace::kBT709)
        .Item("Unspecified", ColorSpace::kUnspecified)
        .Item("Reserved", ColorSpace::kReserved)
        .Item("FCC", ColorSpace::kFCC)
        .Item("BT470BG", ColorSpace::kBT470BG)
        .Item("SMPTE170M", ColorSpace::kSMPTE170M)
        .Item("SMPTE240M", ColorSpace::kSMPTE240M)
        .Item("YCGCO", ColorSpace::kYCGCO)
        .Item("YCOCG", ColorSpace::kYCOCG)
        .Item("BT2020_NCL", ColorSpace::kBT2020_NCL)
        .Item("BT2020_CL", ColorSpace::kBT2020_CL)
        .Item("SMPTE2085", ColorSpace::kSMPTE2085)
        .Item("CHROMA_DERIVED_NCL", ColorSpace::kCHROMA_DERIVED_NCL)
        .Item("CHROMA_DERIVED_CL", ColorSpace::kCHROMA_DERIVED_CL)
        .Item("ICTCP", ColorSpace::kICTCP)
        .Finalize();

    exports["ColorRange"] = ffi::DefineEnum<ColorRange>(isolate, "ColorRange", false)
        .Item("Unspecified", ColorRange::kUnspecified)
        .Item("MPEG", ColorRange::kMPEG)
        .Item("JPEG", ColorRange::kJPEG)
        .Finalize();

    exports["ChromaLocation"] = ffi::DefineEnum<ChromaLocation>(isolate, "ChromaLocation", false)
        .Item("Unspecified", ChromaLocation::kUnspecified)
        .Item("Left", ChromaLocation::kLeft)
        .Item("Center", ChromaLocation::kCenter)
        .Item("TopLeft", ChromaLocation::kTopLeft)
        .Item("Top", ChromaLocation::kTop)
        .Item("BottomLeft", ChromaLocation::kBottomLeft)
        .Item("Bottom", ChromaLocation::kBottom)
        .Finalize();

    exports["SampleFormat"] = ffi::DefineEnum<SampleFormat>(isolate, "SampleFormat", false)
        .Item("None", SampleFormat::kNone)
        .Item("U8", SampleFormat::kU8)
        .Item("S16", SampleFormat::kS16)
        .Item("S32", SampleFormat::kS32)
        .Item("FLT", SampleFormat::kFLT)
        .Item("DBL", SampleFormat::kDBL)
        .Item("U8P", SampleFormat::kU8P)
        .Item("S16P", SampleFormat::kS16P)
        .Item("S32P", SampleFormat::kS32P)
        .Item("FLTP", SampleFormat::kFLTP)
        .Item("DBLP", SampleFormat::kDBLP)
        .Item("S64", SampleFormat::kS64)
        .Item("S64P", SampleFormat::kS64P)
        .Finalize();

    exports["SeekFrameFlags"] = ffi::DefineEnum<SeekFrameFlags>(isolate, "SeekFrameFlags", true)
        .Item("Backward", SeekFrameFlags::kBackward)
        .Item("Byte", SeekFrameFlags::kByte)
        .Item("Any", SeekFrameFlags::kAny)
        .Item("Frame", SeekFrameFlags::kFrame)
        .Finalize();

    exports["Discard"] = ffi::DefineEnum<Discard>(isolate, "Discard", false)
        .Item("None", Discard::kNone)
        .Item("Default", Discard::kDefault)
        .Item("NonRef", Discard::kNonRef)
        .Item("Bidir", Discard::kBidir)
        .Item("NonIntra", Discard::kNonIntra)
        .Item("NonKey", Discard::kNonKey)
        .Item("All", Discard::kAll)
        .Finalize();

    exports["PacketFlags"] = ffi::DefineEnum<PacketFlags>(isolate, "PacketFlags", true)
        .Item("Key", PacketFlags::kKey)
        .Item("Corrupt", PacketFlags::kCorrupt)
        .Item("Discard", PacketFlags::kDiscard)
        .Item("Disposable", PacketFlags::kDisposable)
        .Finalize();

    exports["CodecType"] = ffi::DefineEnum<CodecType>(isolate, "CodecType", false)
        .Item("Encoder", CodecType::kEncoder)
        .Item("Decoder", CodecType::kDecoder)
        .Finalize();

    exports["CodecStatus"] = ffi::DefineEnum<CodecStatus>(isolate, "CodecStatus", false)
        .Item("Success", CodecStatus::kSuccess)
        .Item("NeedConsumeOutput", CodecStatus::kNeedConsumeOutput)
        .Item("NeedFeedInput", CodecStatus::kNeedFeedInput)
        .Item("EOF", CodecStatus::kEOF)
        .Item("Invalid", CodecStatus::kInvalid)
        .Item("NoMemory", CodecStatus::kNoMemory)
        .Item("Error", CodecStatus::kError)
        .Finalize();

    exports["FrameSchedulerStatus"] = ffi::DefineEnum<FrameSchedulerStatus>(
            isolate, "FrameSchedulerStatus", false)
        .Item("Success", FrameSchedulerStatus::kSuccess)
        .Item("Full", FrameSchedulerStatus::kFull)
        .Finalize();

    exports["FrameSchedulerPausePolicy"] = ffi::DefineEnum<FrameSchedulerPausePolicy>(
            isolate, "FrameSchedulerPausePolicy", false)
        .Item("Conserve", FrameSchedulerPausePolicy::kConserve)
        .Item("Discard", FrameSchedulerPausePolicy::kDiscard)
        .Item("DiscardAndReject", FrameSchedulerPausePolicy::kDiscardAndReject)
        .Finalize();

    exports["PictureType"] = ffi::DefineEnum<PictureType>(isolate, "PictureType", false)
        .Item("None", PictureType::kNone)
        .Item("I", PictureType::kI)
        .Item("P", PictureType::kP)
        .Item("B", PictureType::kB)
        .Item("S", PictureType::kS)
        .Item("SI", PictureType::kSI)
        .Item("SP", PictureType::kSP)
        .Item("BI", PictureType::kBI)
        .Finalize();

    exports["FrameFlags"] = ffi::DefineEnum<FrameFlags>(isolate, "FrameFlags", false)
        .Item("Corrupt", FrameFlags::kCorrupt)
        .Item("Key", FrameFlags::kKey)
        .Item("Discard", FrameFlags::kDiscard)
        .Item("Interlaced", FrameFlags::kInterlaced)
        .Item("TopFieldFirst", FrameFlags::kTopFieldFirst)
        .Finalize();

    exports["FilterGraphReceiveStatus"] = ffi::DefineEnum<FilterGraphReceiveStatus>(
            isolate, "FilterGraphReceiveStatus", false)
        .Item("Success", FilterGraphReceiveStatus::kSuccess)
        .Item("NeedInput", FilterGraphReceiveStatus::kNeedInput)
        .Item("EOF", FilterGraphReceiveStatus::kEOF)
        .Finalize();

    //! TSDecl: @enum ScaleResampler
    exports["ScaleResampler"] = ffi::DefineEnum<utau::ScaleResampler>(isolate, "ScaleResampler", false)
        //! TSDecl: @enumitem Nearest
        .Item("Nearest", utau::ScaleResampler::kNearest)
        //! TSDecl: @enumitem Bilinear
        .Item("Bilinear", utau::ScaleResampler::kBilinear)
        //! TSDecl: @enumitem Bicubic
        .Item("Bicubic", utau::ScaleResampler::kBicubic)
        .Finalize();
    //! TSDecl: @end

    ffi::DefineInterface<MediaIOBackend>(isolate, "MediaIOBackend")
        .Field("writable", &MediaIOBackend::writable)
        .Field("bufferSizeHint", &MediaIOBackend::buffer_size_hint)
        .Field("onReadPacket", &MediaIOBackend::on_read_packet)
        .Field("onWritePacket", &MediaIOBackend::on_write_packet)
        .Field("onSeek", &MediaIOBackend::on_seek)
        .Finalize();

    ffi::DefineInterface<DemuxerOptions>(isolate, "DemuxerOptions")
        .Field("formatWhitelist", &DemuxerOptions::format_whitelist)
        .Field("codecWhitelist", &DemuxerOptions::codec_whitelist)
        .Finalize();

    ffi::DefineInterface<FrameMakeFromEncodedOptions>(isolate, "FrameMakeFromEncodedOptions")
        .Field("forceDecodeToYCbCr", &FrameMakeFromEncodedOptions::force_decode_to_ycbcr)
        .Field("reinterpretToSRGB", &FrameMakeFromEncodedOptions::reinterpret_to_srgb)
        .Field("toLinearGamma", &FrameMakeFromEncodedOptions::to_linear_gamma)
        .Field("toSRGBColorSpace", &FrameMakeFromEncodedOptions::to_srgb_color_space)
        .Field("format", &FrameMakeFromEncodedOptions::format)
        .Finalize();

    ffi::DefineInterface<FrameSchedulerQueueOptions>(isolate, "FrameSchedulerQueueOptions")
        .Field("emitsPresentEvent", &FrameSchedulerQueueOptions::emits_present_event)
        .Field("requiresFrameFeedback", &FrameSchedulerQueueOptions::requires_frame_feedback)
        .Finalize();

    ffi::DefineInterface<FrameSpecification>(isolate, "FrameSpecification")
        .Field("pixelFormat", &FrameSpecification::pixel_format)
        .Field("sampleFormat", &FrameSpecification::sample_format)
        .Field("width", &FrameSpecification::width)
        .Field("height", &FrameSpecification::height)
        .Field("nbSamples", &FrameSpecification::nb_samples)
        .Field("pictureType", &FrameSpecification::picture_type)
        .Field("SAR", &FrameSpecification::sar)
        .Field("pts", &FrameSpecification::pts)
        .Field("sampleRate", &FrameSpecification::sample_rate)
        .Field("flags", &FrameSpecification::flags)
        .Field("colorRange", &FrameSpecification::color_range)
        .Field("colorPrimaries", &FrameSpecification::color_primaries)
        .Field("colorTrc", &FrameSpecification::color_trc)
        .Field("colorSpace", &FrameSpecification::color_space)
        .Field("chromaLocation", &FrameSpecification::chroma_location)
        .Field("channelLayout", &FrameSpecification::channel_layout)
        .Field("duration", &FrameSpecification::duration)
        .Field("hwFramesCtx", &FrameSpecification::hw_frames_ctx)
        .Field("bestEffortTimestamp", &FrameSpecification::best_effort_timestamp)
        .Field("cropTop", &FrameSpecification::crop_top)
        .Field("cropBottom", &FrameSpecification::crop_bottom)
        .Field("cropLeft", &FrameSpecification::crop_left)
        .Field("cropRight", &FrameSpecification::crop_right)
        .Finalize();

    exports["MediaIOContext"] = ffi::DefineClass<MediaIOContext>(isolate)
        .MethodStatic("GetSupportedProtocols", MediaIOContext::GetSupportedProtocols)
        .MethodStatic("OpenURL", MediaIOContext::OpenURL)
        .MethodStatic("OpenDynamicMemory", MediaIOContext::OpenDynamicMemory)
        .MethodStatic("OpenFrom", MediaIOContext::OpenFrom)
        .Property<bool>("writable", &MediaIOContext::getWritable, nullptr)
        .Property<bool>("readable", &MediaIOContext::getReadable, nullptr)
        .Method("dispose", &MediaIOContext::dispose)
        .Method("disposeMemoryDynamic", &MediaIOContext::disposeMemoryDynamic)
        .Method("isLocked", &MediaIOContext::isLocked)
        .Method("read", &MediaIOContext::read)
        .Method("readPartial", &MediaIOContext::readPartial)
        .Method("write", &MediaIOContext::write)
        .Method("seek", &MediaIOContext::seek)
        .Method("flush", &MediaIOContext::flush)
        .Property<ffi::PreciseU64>("sizeInBytes", &MediaIOContext::getSizeInBytes, nullptr)
        .Finalize(jsctx);

    exports["AChannelLayout"] = ffi::DefineClass<AChannelLayout>(isolate)
        .MethodStatic("Predefined", AChannelLayout::Predefined)
        .Property<int32_t>("channels", &AChannelLayout::getChannels, nullptr)
        .Property<v8::Local<v8::Value>>("orderedChannels", &AChannelLayout::getOrderedChannels, nullptr)
        .Method("clone", &AChannelLayout::clone)
        .Method("equalTo", &AChannelLayout::equalTo)
        .Method("intersect", &AChannelLayout::intersect)
        .Finalize(jsctx);

    exports["FormatDemuxer"] = ffi::DefineClass<FormatDemuxer>(isolate)
        .MethodStatic("Make", &FormatDemuxer::Make)
        .Method("dispose", &FormatDemuxer::dispose)
        .Property<v8::Local<v8::Value>>("formatInfo", &FormatDemuxer::getFormatInfo, nullptr)
        .Method("findBestStream", &FormatDemuxer::findBestStream)
        .Method("matchStreamSpecifier", &FormatDemuxer::matchStreamSpecifier)
        .Method("getStreamInfo", &FormatDemuxer::getStreamInfo)
        .Method("setStreamDiscard", &FormatDemuxer::setStreamDiscard)
        .Method("getCodecParameters", &FormatDemuxer::getCodecParameters)
        .Method("seekFrame", &FormatDemuxer::seekFrame)
        .Method("readPlay", &FormatDemuxer::readPlay)
        .Method("readPause", &FormatDemuxer::readPause)
        .Method("readFrame", &FormatDemuxer::readFrame)
        .Finalize(jsctx);

    exports["CodecParameters"] = ffi::DefineClass<CodecParameters>(isolate)
        .Constructor()
        .MethodStatic("SearchCodecID", CodecParameters::SearchCodecID)
        .MethodStatic("GetCodecType", CodecParameters::GetCodecType)
        .MethodStatic("GetCodecBitsPerSample", CodecParameters::GetCodecBitsPerSample)
        .Method("clone", &CodecParameters::clone)
        .Method("setCodecType", &CodecParameters::setCodecType)
        .Method("setCodecID", &CodecParameters::setCodecID)
        .Method("copyExtraData", &CodecParameters::copyExtraData)
        .Method("setPixelFormat", &CodecParameters::setPixelFormat)
        .Method("setSampleFormat", &CodecParameters::setSampleFormat)
        .Method("setBitRate", &CodecParameters::setBitRate)
        .Method("setBitsPerCodedSample", &CodecParameters::setBitsPerCodedSample)
        .Method("setBitsPerRawSample", &CodecParameters::setBitsPerRawSample)
        .Method("setProfileLevel", &CodecParameters::setProfileLevel)
        .Method("setDimensions", &CodecParameters::setDimensions)
        .Method("setSampleAspectRatio", &CodecParameters::setSampleAspectRatio)
        .Method("setFieldOrder", &CodecParameters::setFieldOrder)
        .Method("setColorSpace", &CodecParameters::setColorSpace)
        .Method("setVideoDelay", &CodecParameters::setVideoDelay)
        .Method("setSampleRate", &CodecParameters::setSampleRate)
        .Method("setBlockAlign", &CodecParameters::setBlockAlign)
        .Method("setFrameSize", &CodecParameters::setFrameSize)
        .Method("setInitialPadding", &CodecParameters::setInitialPadding)
        .Method("setTrailingPadding", &CodecParameters::setTrailingPadding)
        .Method("setSeekPreroll", &CodecParameters::setSeekPreroll)
        .Method("setChannelLayout", &CodecParameters::setChannelLayout)
        .Finalize(jsctx);

    exports["Packets"] = ffi::DefineClass<Packet>(isolate)
        .Method("clone", &Packet::clone)
        .Method("dispose", &Packet::dispose)
        .Method("disposeReusable", &Packet::disposeReusable)
        .Property<v8::Local<v8::Value>>("pts", &Packet::getPts, nullptr)
        .Property<v8::Local<v8::Value>>("dts", &Packet::getDts, nullptr)
        .Property<v8::Local<v8::Value>>("duration", &Packet::getDuration, nullptr)
        .Property<int32_t>("streamIndex", &Packet::getStreamIndex, nullptr)
        .Property<int32_t>("flags", &Packet::getFlags, nullptr)
        .Finalize(jsctx);

    exports["CodecContextBuilder"] = ffi::DefineClass<CodecContextBuilder>(isolate)
        .Constructor<const ffi::Enum<CodecType>&>()
        .Method("setCodecParameters", &CodecContextBuilder::setCodecParameters)
        .Method("setCodecOption", &CodecContextBuilder::setCodecOption)
        .Method("setPacketTimebase", &CodecContextBuilder::setPacketTimebase)
        .Method("setEncoderHWFramesContext", &CodecContextBuilder::setEncoderHWFramesContext)
        .Method("setDecoderHWDeviceContext", &CodecContextBuilder::setDecoderHWDeviceContext)
        .Method("detach", &CodecContextBuilder::detach)
        .Finalize(jsctx);

    exports["CodecContext"] = ffi::DefineClass<CodecContext>(isolate)
        .Method("dispose", &CodecContext::dispose)
        .Method("sendPacket", &CodecContext::sendPacket)
        .Method("receiveFrame", &CodecContext::receiveFrame)
        .Method("sendFrame", &CodecContext::sendFrame)
        .Method("receivePacket", &CodecContext::receivePacket)
        .Finalize(jsctx);

    exports["PixelFrameFactory"] = ffi::DefineClass<PixelFrameFactory>(isolate)
        .MethodStatic("FromEncoded", PixelFrameFactory::FromEncoded)
        .MethodStatic("FromImage", PixelFrameFactory::FromImage)
        .Finalize(jsctx);

    exports["Frame"] = ffi::DefineClass<Frame>(isolate)
        .MethodStatic("MakeUnallocated", Frame::MakeUnallocated)
        // skip the dispose-state check for this method, since it needs calling
        // in that state
        .Method("allocate", &Frame::allocate, true, true)
        .Method("clone", &Frame::clone)
        .Method("dispose", &Frame::dispose)
        .Method("disposeReusable", &Frame::disposeReusable)
        .Method("specification", &Frame::specification)
        .Method("updateSpecification", &Frame::updateSpecification)
        .Finalize(jsctx);

    exports["PixelFrameView"] = ffi::DefineClass<PixelFrameView>(isolate)
        .Constructor<const ffi::Class<Frame>&>()
        .Method("isWritable", &PixelFrameView::isWritable)
        .Method("makeWritable", &PixelFrameView::makeWritable)
        .Method("writeImageFrom", &PixelFrameView::writeImageFrom)
        .Method("readPlaneRectTo", &PixelFrameView::readPlaneRectTo)
        .Method("resolveColorSpace", &PixelFrameView::resolveColorSpace)
        .Method("blitToPixmap", &PixelFrameView::blitToPixmap)
        .Method("wrapToImage", &PixelFrameView::wrapToImage)
        .Method("applyCropping", &PixelFrameView::applyCropping)
        .Finalize(jsctx);

    exports["AudioStreamService"] = ffi::DefineClass<AudioStreamService>(isolate)
        .MethodStatic("Connect", AudioStreamService::Connect)
        .Method("dispose", &AudioStreamService::dispose)
        .Method("createSinkStream", &AudioStreamService::createSinkStream)
        .Finalize(jsctx);

    exports["AudioSinkStream"] = ffi::DefineClass<AudioSinkStream>(isolate)
        .Inherit<EventEmitterBase>()
        .Method("dispose", &AudioSinkStream::dispose)
        .Method("enqueue", &AudioSinkStream::enqueue)
        .Method("getDelayInUs", &AudioSinkStream::getDelayInUs)
        .Method("setVolumes", &AudioSinkStream::setVolumes)
        .Finalize(jsctx);

    exports["FrameSchedulerBuilder"] = ffi::DefineClass<FrameSchedulerBuilder>(isolate)
        .Constructor<>()
        .Method("addQueue", &FrameSchedulerBuilder::addQueue)
        .Method("setAudioSinkCreationInfo", &FrameSchedulerBuilder::setAudioSinkCreationInfo)
        .Method("build", &FrameSchedulerBuilder::build)
        .Finalize(jsctx);

    exports["FrameScheduler"] = ffi::DefineClass<FrameScheduler>(isolate)
        .Inherit<EventEmitterBase>()
        .Method("enqueue", &FrameScheduler::enqueue)
        .Method("enqueuePromise", &FrameScheduler::enqueuePromise)
        .Method("pause", &FrameScheduler::pause)
        .Method("resume", &FrameScheduler::resume)
        .Method("dispose", &FrameScheduler::dispose)
        .Finalize(jsctx);

    exports["HWDeviceContext"] = ffi::DefineClass<HWDeviceContext>(isolate)
        .MethodStatic("MakeVulkan", HWDeviceContext::MakeVulkan)
        .Method("dispose", &HWDeviceContext::dispose)
        .Finalize(jsctx);

    exports["HWFramesContext"] = ffi::DefineClass<HWFramesContext>(isolate)
        .MethodStatic("GetPlatformConstraints", HWFramesContext::GetPlatformConstraints)
        .MethodStatic("Make", HWFramesContext::Make)
        .MethodStatic("Transfer", HWFramesContext::Transfer)
        .Method("equalTo", &HWFramesContext::equalTo)
        .Method("getBuffer", &HWFramesContext::getBuffer)
        .Method("queryTransferFormats", &HWFramesContext::queryTransferFormats)
        .Finalize(jsctx);

    exports["FilterGraphBuilder"] = ffi::DefineClass<FilterGraphBuilder>(isolate)
        .Constructor<>()
        .Method("setThreads", &FilterGraphBuilder::setThreads)
        .Method("setGraph", &FilterGraphBuilder::setGraph)
        .Method("addAudioSrc", &FilterGraphBuilder::addAudioSrc)
        .Method("addVideoSrc", &FilterGraphBuilder::addVideoSrc)
        .Method("addAudioSink", &FilterGraphBuilder::addAudioSink)
        .Method("addVideoSink", &FilterGraphBuilder::addVideoSink)
        .Method("build", &FilterGraphBuilder::build)
        .Finalize(jsctx);

    exports["FilterGraph"] = ffi::DefineClass<FilterGraph>(isolate)
        .Method("dispose", &FilterGraph::dispose)
        .Method("getSinkProperties", &FilterGraph::getSinkProperties)
        .Method("sendFrame", &FilterGraph::sendFrame)
        .Method("receiveFrame", &FilterGraph::receiveFrame)
        .Method("sendCommand", &FilterGraph::sendCommand)
        .Finalize(jsctx);

    InsertScriptExports(isolate, "internal://natives/multimedia.js", exports);
    auto *storage = ffi::TypeContext::FromIsolate(isolate)->GetValueStorage();
    storage->Store(FFI_GVSTORE_USE_ID(ctor_Rational), exports["Rational"]);

    return exports;
}

GALLIUM_BINDINGS_NS_END
