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

#ifndef COCOA_GALLIUM_BINDINGS_MULTIMEDIA_CODECPARAMETERS_H
#define COCOA_GALLIUM_BINDINGS_MULTIMEDIA_CODECPARAMETERS_H

#include "Gallium/bindings/multimedia/ffwrappers/libavcodec-codecpar.h"

#include "Gallium/bindings/multimedia/Types.h"
#include "Gallium/ffi/JSObject.h"
#include "Gallium/ffi/Class.h"
#include "Gallium/ffi/Enum.h"
#include "Gallium/ffi/ArrayBuffer.h"
GALLIUM_BINDINGS_MULTIMEDIA_NS_BEGIN

class AChannelLayout;

//! @tsdocbegin
//! `CodecParameters` describes a set of parameters (properties) of an encoded stream.
//! These parameters are used to instantiate a codec context.
//! @tsdocend
//! TSDecl: @class CodecParameters
class CodecParameters : public ffi::JSObject
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    //! @tsdocbegin
    //! Create an instance with all the parameters set to default values,
    //! e.g. unknown or invalid or 0. User is supposed to fill the parameters
    //! before using this instance to create codec context.
    //! @tsdocend
    //! TSDecl: @constructor()
    CodecParameters();

    // internal constructor
    explicit CodecParameters(AVCodecParameters *params) : params_(params) {}
    ~CodecParameters() override;

    g_nodiscard AVCodecParameters *GetAVCodecParameters() const {
        return params_;
    }

    //! @tsdocbegin
    //! Search a codec ID by its name, always lowercase, with simple fuzzy search.
    //! Returns a map of matched codec name and ID.
    //! @tsdocend
    //! TSDecl: @method @static SearchCodecID(name: string): @generic(Map, string, u32)
    static ffi::RetLocal<v8::Value> SearchCodecID(const std::string& name);

    //! @tsdocbegin
    //! Get the media type of the given codec ID.
    //! Codec ID can be queried by name through `SearchCodecID()` method.
    //! @tsdocend
    //! TSDecl: @method @static GetCodecType(id: u32): MediaType
    static ffi::Ret<int32_t> GetCodecType(uint32_t id);

    //! @tsdocbegin
    //! Returns codec bits per sample, or zero if unknown.
    //! Codec ID can be queried by name through `SearchCodecID()` method.
    //! @tsdocend
    //! TSDecl: @method @static GetCodecBitsPerSample(id: u32): i32
    static ffi::Ret<int32_t> GetCodecBitsPerSample(uint32_t id);

    //! TSDecl: @method clone(): CodecParameters
    ffi::RetLocal<v8::Value> clone();

    //! TSDecl: @method setCodecType(type: MediaType): CodecParameters
    ffi::RetLocal<v8::Value> setCodecType(const ffi::Enum<MediaType>& type) {
        params_->codec_type = static_cast<AVMediaType>(*type);
        return GetThisHandle(v8::Isolate::GetCurrent());
    }

    //! @tsdocbegin
    //! Codec ID can be queried from `SearchCodecID()` method.
    //! @tsdocend
    //! TSDecl: @method setCodecID(id: u32): CodecParameters
    ffi::RetLocal<v8::Value> setCodecID(uint32_t id) {
        params_->codec_id = static_cast<AVCodecID>(id);
        return GetThisHandle(v8::Isolate::GetCurrent());
    }

    //! @tsdocbegin
    //! Extra binary data needed for initializing the decoder, codec-dependent.
    //! It copies the original data, not storing the given array buffer.
    //! @tsdocend
    //! TSDecl: @method copyExtraData(data: @mem(u8)): CodecParameters
    ffi::RetLocal<v8::Value> copyExtraData(const ffi::Mem<uint8_t>& data);
    
    //! TSDecl: @method setPixelFormat(format: PixelFormat): CodecParameters
    ffi::RetLocal<v8::Value> setPixelFormat(const ffi::Enum<PixelFormat>& format) {
        params_->format = static_cast<AVPixelFormat>(format.GetInteger());
        return GetThisHandle(v8::Isolate::GetCurrent());
    }

    //! TSDecl: @method setSampleFormat(format: SampleFormat): CodecParameters
    ffi::RetLocal<v8::Value> setSampleFormat(const ffi::Enum<SampleFormat>& format) {
        params_->format = static_cast<AVSampleFormat>(format.GetInteger());
        return GetThisHandle(v8::Isolate::GetCurrent());
    }
    
    //! @tsdocbegin
    //! The average bitrate of the encoded data, in bits per second.
    //! @tsdocend
    //! TSDecl: @method setBitRate(br: i64): CodecParameters
    ffi::RetLocal<v8::Value> setBitRate(int64_t br) {
        params_->bit_rate = br;
        return GetThisHandle(v8::Isolate::GetCurrent());
    }
    
    //! @tsdocbegin
    //! The number of bits per sample in the codedwords.
    //!
    //! This is basically the bitrate per sample. It is mandatory for a bunch of
    //! formats to actually decode them. It's the number of bits for one sample in
    //! the actual coded bitstream.
    //!
    //! This could be for example 4 for ADPCM, for PCM formats this matches `setBitsPerRawSample()`.
    //! Can be 0.
    //! @tsdocend
    //! TSDecl: @method setBitsPerCodedSample(bpcs: i32): CodecParameters
    ffi::RetLocal<v8::Value> setBitsPerCodedSample(int32_t bpcs) {
        params_->bits_per_coded_sample = bpcs;
        return GetThisHandle(v8::Isolate::GetCurrent());
    }
    
    //! @tsdocbegin
    //! This is the number of valid bits in each output sample. If the
    //! sample format has more bits, the least significant bits are additional
    //! padding bits, which are always 0. Use right shifts to reduce the sample
    //! to its actual size. For example, audio formats with 24 bit samples will
    //! have bits_per_raw_sample set to 24, and format set to `SampleFormat.S32`.
    //! To get the original sample use "(i32)sample >> 8".
    //!
    //! For ADPCM this might be 12 or 16 or similar.
    //! Can be 0.
    //! @tsdocend
    //! TSDecl: @method setBitsPerRawSample(bprs: i32): CodecParameters
    ffi::RetLocal<v8::Value> setBitsPerRawSample(int32_t bprs) {
        params_->bits_per_raw_sample = bprs;
        return GetThisHandle(v8::Isolate::GetCurrent());
    }

    //! @tsdocbegin
    //! Codec-specific bitstream restrictions that the stream conforms to.
    //! @tsdocend
    //! TSDecl: @method setProfileLevel(profile: i32, level: i32): CodecParameters
    ffi::RetLocal<v8::Value> setProfileLevel(int32_t profile, int32_t level) {
        params_->profile = profile;
        params_->level = level;
        return GetThisHandle(v8::Isolate::GetCurrent());
    }

    //! @tsdocbegin
    //! Video only. The dimensions of the video frame in pixels.
    //! @tsdocend
    //! TSDecl: @method setDimensions(width: i32, height: i32): CodecParameters
    ffi::RetLocal<v8::Value> setDimensions(int32_t width, int32_t height) {
        params_->width = width;
        params_->height = height;
        return GetThisHandle(v8::Isolate::GetCurrent());
    }

    //! @tsdocbegin
    //! Video only. The aspect ratio which a single pixel should have
    //! when displayed. When the aspect ratio is unknown or undefined, the numerator
    //! should be set to 0 (the denominator may have any value).
    //! @tsdocend
    //! TSDecl: @method setSampleAspectRatio(sar: Rational): CodecParameters
    ffi::RetLocal<v8::Value> setSampleAspectRatio(const AVRationalAdapter& sar) {
        params_->sample_aspect_ratio = *sar;
        return GetThisHandle(v8::Isolate::GetCurrent());
    }

    //! @tsdocbegin
    //! Video only. The order of the fields in interlaced video.
    //! @tsdocend
    //! TSDecl: @method setFieldOrder(fo: FieldOrder): CodecParameters
    ffi::RetLocal<v8::Value> setFieldOrder(const ffi::Enum<FieldOrder>& fo) {
        params_->field_order = static_cast<AVFieldOrder>(fo.GetInteger());
        return GetThisHandle(v8::Isolate::GetCurrent());
    }

    //! @tsdocbegin
    //! Video only. Additional colorspace characteristics.
    //! @tsdocend
    //! TSDecl: @method setColorSpace(range: ColorRange, primaries: ColorPrimaries,
    //! TSDecl:                       trc: ColorTransferCharacteristic, space: ColorSpace,
    //! TSDecl:                       chromaLoc: ChromaLocation): CodecParameters
    ffi::RetLocal<v8::Value> setColorSpace(const ffi::Enum<ColorRange>& range,
                                 const ffi::Enum<ColorPrimaries>& primaries,
                                 const ffi::Enum<ColorTransferCharacteristic>& trc,
                                 const ffi::Enum<ColorSpace>& space,
                                 const ffi::Enum<ChromaLocation>& chroma_loc)
    {
        params_->color_range = static_cast<AVColorRange>(range.GetInteger());
        params_->color_primaries = static_cast<AVColorPrimaries>(primaries.GetInteger());
        params_->color_trc = static_cast<AVColorTransferCharacteristic>(trc.GetInteger());
        params_->color_space = static_cast<AVColorSpace>(space.GetInteger());
        params_->chroma_location = static_cast<AVChromaLocation>(chroma_loc.GetInteger());
        return GetThisHandle(v8::Isolate::GetCurrent());
    }

    //! @tsdocbegin
    //! Video only. Number of delayed frame.
    //! @tsdocend
    //! TSDecl: @method setVideoDelay(delay: i32): CodecParameters
    ffi::RetLocal<v8::Value> setVideoDelay(int32_t delay) {
        params_->video_delay = delay;
        return GetThisHandle(v8::Isolate::GetCurrent());
    }

    //! @tsdocbegin
    //! Audio only. The number of audio samples per second.
    //! @tsdocend
    //! TSDecl: @method setSampleRate(sr: i32): CodecParameters
    ffi::RetLocal<v8::Value> setSampleRate(int32_t sr) {
        params_->sample_rate = sr;
        return GetThisHandle(v8::Isolate::GetCurrent());
    }

    //! @tsdocbegin
    //! Audio only. The number of bytes per coded audio frame, required by some
    //! formats. Corresponds to nBlockAlign in WACEFORMATEX.
    //! @tsdocend
    //! TSDecl: @method setBlockAlign(ba: i32): CodecParameters
    ffi::RetLocal<v8::Value> setBlockAlign(int32_t ba) {
        params_->block_align = ba;
        return GetThisHandle(v8::Isolate::GetCurrent());
    }

    //! @tsdocbegin
    //! Audio only. Audio frame size, if known. Required by some formats to be static.
    //! @tsdocend
    //! TSDecl: @method setFrameSize(fs: i32): CodecParameters
    ffi::RetLocal<v8::Value> setFrameSize(int32_t fs) {
        params_->frame_size = fs;
        return GetThisHandle(v8::Isolate::GetCurrent());
    }

    //! @tsdocbegin
    //! Audio only. The amount of padding (in samples) inserted by the encoder at
    //! the beginning of the audio. I.e. this number of leading decoded samples
    //! must be discarded by the caller to get the original audio without leading
    //! padding.
    //! @tsdocend
    //! TSDecl: @method setInitialPadding(pad: i32): CodecParameters
    ffi::RetLocal<v8::Value> setInitialPadding(int32_t pad) {
        params_->initial_padding = pad;
        return GetThisHandle(v8::Isolate::GetCurrent());
    }

    //! @tsdocbegin
    //! Audio only. The amount of padding (in samples) appended by the encoder to
    //! the end of the audio. I.e. this number of decoded samples must be
    //! discarded by the caller from the end of the stream to get the original
    //! audio without any trailing padding.
    //! @tsdocend
    //! TSDecl: @method setTrailingPadding(pad: i32): CodecParameters
    ffi::RetLocal<v8::Value> setTrailingPadding(int32_t pad) {
        params_->trailing_padding = pad;
        return GetThisHandle(v8::Isolate::GetCurrent());
    }

    //! @tsdocbegin
    //! Audio only. Number of samples to skip after a discontinuity.
    //! @tsdocend
    //! TSDecl: @method setSeekPreroll(preroll: i32): CodecParameters
    ffi::RetLocal<v8::Value> setSeekPreroll(int32_t preroll) {
        params_->seek_preroll = preroll;
        return GetThisHandle(v8::Isolate::GetCurrent());
    }

    //! @tsdocbegin
    //! Audio only. The channel layout and number of channels.
    //! @tsdocend
    //! TSDecl: @method setChannelLayout(layout: AChannelLayout): CodecParameters
    ffi::RetLocal<v8::Value> setChannelLayout(const ffi::Class<AChannelLayout>& layout);

private:
    AVCodecParameters *params_;
};
//! TSDecl: @end

GALLIUM_BINDINGS_MULTIMEDIA_NS_END
#endif //COCOA_GALLIUM_BINDINGS_MULTIMEDIA_CODECPARAMETERS_H
