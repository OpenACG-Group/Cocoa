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

#ifndef COCOA_GALLIUM_BINDINGS_MULTIMEDIA_FORMATDEMUXER_H
#define COCOA_GALLIUM_BINDINGS_MULTIMEDIA_FORMATDEMUXER_H

#include "Gallium/bindings/multimedia/ffwrappers/libavformat.h"
#include "Gallium/bindings/multimedia/ffwrappers/libavutil.h"

#include "Gallium/ffi/JSObject.h"
#include "Gallium/ffi/Class.h"
#include "Gallium/ffi/Interface.h"
#include "Gallium/bindings/multimedia/Types.h"
#include "Gallium/bindings/multimedia/MediaInputOutput.h"
GALLIUM_BINDINGS_MULTIMEDIA_NS_BEGIN

class Packet;

//! @tsdocbegin
//! An interface for specifying custom options to the demuxer, affecting its behaviour.
//! @tsdocend
//! TSDecl: @interface DemuxerOptions
struct DemuxerOptions
{
    //! @tsdocbegin
    //! A comma ',' separated string of allowed container formats.
    //! If absent all are allowed.
    //! @tsdocend
    //! TSDecl: @property @optional formatWhitelist: string
    ffi::Opt<std::string> format_whitelist;

    //! @tsdocbegin
    //! A comma ',' separated string of allowed decoders.
    //! If absent all are allowed.
    //! @tsdocend
    //! TSDecl: @property @optional codecWhitelist: string
    ffi::Opt<std::string> codec_whitelist;
};
//! TSDecl: @end

//! @tsdocbegin
//! Info contained in the format container (mp4, mov, etc.).
//! This provides similar information to what FFmpeg's `ffprobe` command prints.
//! Note that details of each stream are not included.
//! @tsdocend
//! TSDecl: @interface FormatContainerInfo
//! TSDecl: @property formatName: string
//! TSDecl: @property formatLongName: string
//! TSDecl: @property streamCount: u32
//! TSDecl: @property duration: i64
//! TSDecl: @property totalStreamBitRate: i64
//! TSDecl: @property metadata: @generic(Map, string, string)
//! TSDecl: @end

//! @tsdocbegin
//! Media stream info read from the media file, filled by `FormatDemuxer`.
//! @tsdocend
//! TSDecl: @interface DemuxStreamInfo
    //! TSDecl: @property index: i32
    //! TSDecl: @property type: MediaType

    //! @tsdocbegin
    //! This is the fundamental unit of time (in seconds) in terms of which
    //! frame timestamps are represented.
    //! @tsdocend
    //! TSDecl: @property timeBase: Rational

    //! @tsdocbegin
    //! PTS of the first frame of the stream presentation order,
    //! in stream time base. May be absent if unknown.
    //! @tsdocend
    //! TSDecl: @property @optional startTime: i64

    //! @tsdocbegin
    //! Duration of the stream, in stream time base.
    //! If a source file does not specify a duration, but does specify a bitrate,
    //! this value will be estimated from bitrate and file size.
    //! @tsdocend
    //! TSDecl: @property duration: i64

    //! @tsdocbegin
    //! Stream disposition - a combination of `StreamDisposition.*` flags.
    //! @tsdocend
    //! TSDecl: @property disposition: i32

    //! TSDecl: @property metadata: @generic(Map, string, string)

    // TODO(sora): other info: `attached_pic`, `side_data`

    //! @tsdocbegin
    //! Video stream info. Only available when `type` is `MediaType.Video`.
    //! @tsdocend
    //! TSDecl: @property @optional videoInfo: DemuxStreamVideoInfo

    //! @tsdocbegin
    //! Audio stream info. Only available when `type` is `MediaType.Audio`.
    //! @tsdocend
    //! TSDecl: @property @optional audioInfo: DemuxStreamAudioInfo
//! TSDecl: @end

//! TSDecl: @interface DemuxStreamVideoInfo
    //! @tsdocbegin
    //! Sample aspect ratio, may be absent if unknown or undefined.
    //! @tsdocend
    //! TSDecl: @property @optional SAR: Rational

    //! TSDecl: @property avgFrameRate: Rational

    //! @tsdocbegin
    //! Real base framerate of the stream.
    //! This is the lowest framerate with which all timestamps can be
    //! represented accurately (it is the least common multiple of all
    //! framerates in the stream). Note, this value is just a guess!
    //! For example, if the time base is 1/90000 and all frames have either
    //! approximately 3600 or 1800 timer ticks, then `framerate` will be 50/1.
    //! @tsdocend
    //! TSDecl: @property framerate: Rational

    //! TSDecl: @property format: PixelFormat

    //! @tsdocbegin
    //! The average bitrate of the encoded data (in bits per second).
    //! @tsdocend
    //! TSDecl: @property bitrate: i64

    //! TSDecl: @property width: i32
    //! TSDecl: @property height: i32

    //! @tsdocbegin
    //! Additional, detailed, colorspace characteristics.
    //! These characteristics are useful for color conversion or running other
    //! image-processing algorithms.
    //! @tsdocend
    //! TSDecl: @property colorRange: ColorRange
    //! TSDecl: @property colorPrimaries: ColorPrimaries
    //! TSDecl: @property colorTrc: ColorTransferCharacteristic
    //! TSDecl: @property colorSpace: ColorSpace
    //! TSDecl: @property chromaLocation: ChromaLocation

    //! @tsdocbegin
    //! Number of delayed frames.
    //! @tsdocend
    //! TSDecl: @property delayFrames: i32

//! TSDecl: @end

//! TSDecl: @interface DemuxStreamAudioInfo
    //! TSDecl: @property format: SampleFormat
    //! TSDecl: @property channelLayout: AChannelLayout
    //! TSDecl: @property sampleRate: i32

    //! @tsdocbegin
    //! The amount of padding (in samples) inserted by the encoder at the beginning of
    //! the audio. I.e. this number of leading decoded samples must be discarded by the
    //! caller to get the original audio without leading padding.
    //! @tsdocend
    //! TSDecl: @property initialPadding: i32

    //! @tsdocbegin
    //! The amount of padding (in samples) appended by the encoder to the end of the audio.
    //! I.e. this number of decoded samples must be discarded by the caller from the end of
    //! the stream to get the original audio without any trailing padding.
    //! @tsdocend
    //! TSDecl: @property trailingPadding: i32

    //! @tsdocbegin
    //! Number of samples to skip after a discontinuity.
    //! @tsdocend
    //! TSDecl: @property seekPreroll: i32
//! TSDecl: @end

//! @tsdocbegin
//! Reads data from a given `MediaIOContext` context, treating the data
//! as multimedia file formats (e.g. mp4, mp3, mov, etc.), then parse and extract
//! media contents from the data.
//! @tsdocend
//! TSDecl: @class @nonconstructible FormatDemuxer
class FormatDemuxer : public ffi::JSObject
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    explicit FormatDemuxer(std::shared_ptr<MediaIOContext::ContextOwner> ioctx,
                           AVFormatContext *format_ctx)
        : ioctx_(std::move(ioctx)), format_ctx_(format_ctx) {}
    ~FormatDemuxer() override = default;

    //! @tsdocbegin
    //! Creates a demuxer that reads data from the given `MediaIOContext`, and returns
    //! the created `FormatDemuxer`. Context must be readable.
    //! The given `MediaIOContext` will be locked until the demuxer is disposed.
    //! Throws an exception when context is not readable, or an error occurs.
    //! @tsdocend
    //! TSDecl: @method @static Make(context: MediaIOContext, options: DemuxerOptions): FormatDemuxer
    static ffi::RetLocal<v8::Value> Make(const ffi::Class<MediaIOContext>& context,
                                         const ffi::IFace<DemuxerOptions>& options);

    //! TSDecl: @method dispose(): void
    ffi::Ret<void> dispose();

    //! TSDecl: @property @readonly formatInfo: FormatContainerInfo
    ffi::RetLocal<v8::Value> getFormatInfo();

    //! @tsdocbegin
    //! Find the "best" stream in the file.
    //! The best stream is determined according to various heuristics as the most
    //! likely to be what the user expects. For example, among multiple video streams,
    //! it selects the one that has the highest resolution.
    //! Returns the index if a stream is found; otherwise, returns -1 if not found or
    //! found but there is no corresponding decoder.
    //! @tsdocend
    //! TSDecl: @method findBestStream(type: MediaType): i32
    ffi::Ret<int32_t> findBestStream(ffi::Enum<MediaType> type);

    //! @tsdocbegin
    //! Stream specifier describes some constraints to filter streams.
    //! This method tests every stream contained in the media file, and returns stream indices
    //! that satisfy all the constraints.
    //! For the syntax of stream specifier, see FFmpeg's documentation:
    //!   https://ffmpeg.org/ffmpeg.html#Stream-specifiers-1
    //!
    //! Throws an exception if specifier is invalid.
    //! @tsdocend
    //! TSDecl: @method matchStreamSpecifier(specifier: string): @array(i32)
    ffi::RetLocal<v8::Value> matchStreamSpecifier(const std::string& specifier);

    //! @tsdocbegin
    //! Get stream information by its index.
    //! Each call returns a newly created object, and user can use the returned object freely.
    //! The object is not cached (each call returns a newly created object).
    //! Throws an exception if `index` is invalid.
    //! @tsdocend
    //! TSDecl: @method getStreamInfo(index: i32): DemuxStreamInfo
    ffi::RetLocal<v8::Value> getStreamInfo(int32_t index);

    //! @tsdocbegin
    //! Controls which frame in a specified stream will not be demuxed.
    //! Throws an exception if `index` is invalid.
    //! @tsdocend
    //! TSDecl: @method setStreamDiscard(index: i32, discard: Discard): void
    ffi::Ret<void> setStreamDiscard(int32_t index, const ffi::Enum<Discard>& discard);

    //! @tsdocbegin
    //! Returns codec parameters of a specified stream.
    //! Never modify the returned parameters, and use it to instantiate a corresponding decoder.
    //! The object is not cached (each call returns a newly created object).
    //! Throws an exception if `index` is invalid.
    //! @tsdocend
    //! TSDecl: @method getCodecParameters(index: i32): CodecParameters
    ffi::RetLocal<v8::Value> getCodecParameters(int32_t index);

    //! @tsdocbegin
    //! Seek to the keyframe at timestamp.
    //! Throws an exception on failure.
    //!
    //! @param index     Stream index. If -1 is provided, a default stream is selected,
    //!                  and timestamp is automatically converted from `TIME_BASE` units
    //!                  to the stream specific time base.
    //! @param timestamp Timestamp in stream's time base units or, if index is -1,
    //!                  in `TIME_BASE` units.
    //! @param flags     Flags which select direction and seeking mode.
    //! @tsdocend
    //! TSDecl: @method seekFrame(index: i32, timestamp: i64, flags: SeekFrameFlags): void
    ffi::Ret<void> seekFrame(int32_t index, int64_t timestamp, int flags);

    // TODO(sora): support `seekFile()`, `flush()`

    //! @tsdocbegin
    //! Start playing a network-based stream (e.g. RTSP stream) at the current position.
    //! Throws an exception on failure.
    //! @tsdocend
    //! TSDecl: @method readPlay(): void
    ffi::Ret<void> readPlay();

    //! @tsdocbegin
    //! Pause a network-based stream (e.g. RTSP stream). Use `readPlay()` to resume it.
    //! Throws an exception on failure.
    //! @tsdocend
    //! TSDecl: @method readPause(): void
    ffi::Ret<void> readPause();
    
    //! @tsdocbegin
    //! Return the next frame of a stream.
    //! This function returns what is stored in the file, and does not validate
    //! that what is there are valid frames for the decoder. It will split what is
    //! stored in the file into frames and return one for each call. It will not
    //! omit invalid data between valid frames so as to give the decoder the maximum
    //! information possible for decoding.
    //!
    //! If `reuse` argument is null, a new packet instance will be created and returned;
    //! otherwise, we try to reuse the provided packet to store data, and returns the same
    //! instance. If the provided packet is not reusable, throws an exception.
    //!
    //! If EOF, returns `null`; throws an exception if an error occurs.
    //! In both case `reuse` keeps untouched.
    //!
    //! For video, the packet contains exactly one frame.
    //! For audio, it contains an integer number of frames if each frame has
    //! a known fixed size (e.g. PCM or ADPCM data). If the audio frames have
    //! a variable size (e.g. MPEG audio), then it contains one frame.
    //!
    //! `packet.pts`, `packet.dts` and `packet.duration` are always set to correct
    //! values in stream's timebase units (and guessed if the format cannot provide them).
    //! `packet.pts` can be `null` if the video format has B-frames, so it is better to
    //! rely on dts if you do not decompress the payload.
    //! @tsdocend
    //! TSDecl: @method readFrame(reuse: @union(Packet, null)): Packet
    ffi::RetLocal<v8::Value> readFrame(ffi::Opt<ffi::Class<Packet>> reuse);

private:
    std::shared_ptr<MediaIOContext::ContextOwner> ioctx_;
    AVFormatContext *format_ctx_;
};
//! TSDecl: @end

GALLIUM_BINDINGS_MULTIMEDIA_NS_END
#endif //COCOA_GALLIUM_BINDINGS_MULTIMEDIA_FORMATDEMUXER_H
