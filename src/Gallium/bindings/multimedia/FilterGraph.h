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

#ifndef COCOA_GALLIUM_BINDINGS_MULTIMEDIA_FILTERGRAPH_H
#define COCOA_GALLIUM_BINDINGS_MULTIMEDIA_FILTERGRAPH_H

#include "Gallium/bindings/multimedia/Types.h"
#include "Gallium/ffi/JSObject.h"
#include "Gallium/ffi/Enum.h"
#include "Gallium/ffi/Class.h"
GALLIUM_BINDINGS_MULTIMEDIA_NS_BEGIN

class Frame;
class AChannelLayout;
class HWFramesContext;

struct PipelineContext;

//! @tsdocbegin
//! Helper class for creating a `FilterGraph` instance.
//! @tsdocend
//! TSDecl: @class FilterGraphBuilder
class FilterGraphBuilder : public ffi::JSObject
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    //! TSDecl: @constructor()
    FilterGraphBuilder();
    ~FilterGraphBuilder() override = default;

    //! TSDecl: @method setThreads(num: i32): FilterGraphBuilder
    ffi::RetLocal<v8::Value> setThreads(int32_t num);

    //! @tsdocbegin
    //! Set the FFmpeg graph description of the target filter graph. It describes the
    //! nodes (filters) and links in the graph. For detailed information, see:
    //!     https://ffmpeg.org/ffmpeg-filters.html
    //!
    //! This function just declares the topology of the graph, and the user should
    //! call `add{Audio,Video}{Src,Sink}()` methods to create the corresponding input
    //! and output pads that are specified in the description. For example:
    //! \code
    //!     builder.setGraph('[inp] afade [outp]');
    //!     builder.addAudioSrc('inp', ...);
    //!     builder.addAudioSink('outp', ...);
    //! \endcode
    //!
    //! This function can be called multiple times, and the newer value replaces the
    //! older value. But the input/output pads that have added remain unchanged.
    //! @tsdocend
    //! TSDecl: @method setGraph(dsl: string): FilterGraphBuilder
    ffi::RetLocal<v8::Value> setGraph(const std::string& dsl);

    //! @tsdocbegin
    //! Adds an audio input pad in the graph. ID of the pad is what you specify in the
    //! graph description. Other arguments constrains the format and properties of
    //! acceptable frames on this pad. Frames not satisfying all the constraints cannot
    //! be pushed into the filter graph through this pad.
    //! @tsdocend
    //! TSDecl: @method addAudioSrc(padId: string,
    //! TSDecl:                     format: SampleFormat,
    //! TSDecl:                     timebase: @union(Rational, null),
    //! TSDecl:                     sampleRate: i32,
    //! TSDecl:                     channelLayout: AChannelLayout): FilterGraphBuilder
    ffi::RetLocal<v8::Value> addAudioSrc(const std::string& pad_id,
                                         ffi::Enum<SampleFormat> format,
                                         const ffi::Opt<AVRationalAdapter>& timebase,
                                         int32_t sample_rate,
                                         const ffi::Class<AChannelLayout>& ch_layout);

    //! @tsdocbegin
    //! Adds an video input pad in the graph. ID of the pad is what you specify in the
    //! graph description. Other arguments constrains the format and properties of
    //! acceptable frames on this pad. Frames not satisfying all the constraints cannot
    //! be pushed into the filter graph through this pad.
    //!
    //! If `hwctx` is passed, `format` must be `PixelFormat.kNone`, and an appropriate
    //! format will be selected automatically.
    //! @tsdocend
    //! TSDecl: @method addVideoSrc(padId: string,
    //! TSDecl:                     format: PixelFormat,
    //! TSDecl:                     timebase: @union(Rational, null),
    //! TSDecl:                     width: i32,
    //! TSDecl:                     height: i32,
    //! TSDecl:                     sar: Rational,
    //! TSDecl:                     colorSpace: ColorSpace,
    //! TSDecl:                     colorRange: ColorRange,
    //! TSDecl:                     hwctx: @union(HWFramesContext, null)): FilterGraphBuilder
    ffi::RetLocal<v8::Value> addVideoSrc(const std::string& pad_id,
                                         ffi::Enum<PixelFormat> format,
                                         const ffi::Opt<AVRationalAdapter>& timebase,
                                         int32_t width,
                                         int32_t height,
                                         const AVRationalAdapter& sar,
                                         ffi::Enum<ColorSpace> color_space,
                                         ffi::Enum<ColorRange> color_range,
                                         const ffi::Opt<ffi::Class<HWFramesContext>>& hwctx);

    //! @tsdocbegin
    //! Adds an audio output pad in the graph. ID of the pad is what you specify in the
    //! graph description.
    //! @tsdocend
    //! TSDecl: @method addAudioSink(padId: string): FilterGraphBuilder
    ffi::RetLocal<v8::Value> addAudioSink(const std::string& pad_id);

    //! @tsdocbegin
    //! Adds an video output pad in the graph. ID of the pad is what you specify in the
    //! graph description.
    //! @tsdocend
    //! TSDecl: @method addVideoSink(padId: string): FilterGraphBuilder
    ffi::RetLocal<v8::Value> addVideoSink(const std::string& pad_id);

    //! @tsdocbegin
    //! Checks the configuration and creates a filter graph instance.
    //! The builder itself is disposed after return.
    //! Throws an exception on failure.
    //! @tsdocend
    //! TSDecl: @method build(): FilterGraph
    ffi::RetLocal<v8::Value> build();

private:
    std::shared_ptr<PipelineContext> ctx_;
};
//! TSDecl: @end

//! TSDecl: @enum FilterGraphReceiveStatus
enum class FilterGraphReceiveStatus
{
    //! TSDecl: @enumitem Success
    kSuccess,

    //! @tsdocbegin
    //! No frames are available at this point; more input frames must
    //! be sent to the filter graph to get more output.
    //! @tsdocend
    //! TSDecl: @enumitem NeedInput
    kNeedInput,

    //! @tsdocbegin
    //! There will be no more output frames on this sink.
    //! @tsdocend
    //! TSDecl: @enumitem EOF
    kEOF
};
//! TSDecl: @end

//! TSDecl: @interface FilterGraphSinkProperties
    //! TSDecl: @property type: MediaType
    //! TSDecl: @property timeBase: @union(Rational, null)

    //! @tsdocbegin
    //! The following properties are video only.
    //! Note that for video sinks, the following properties are never absent,
    //! but can be `null` when unavailable.
    //! @tsdocend
    //! TSDecl: @property @optional pixelFormat: PixelFormat
    //! TSDecl: @property @optional frameRate: @union(Rational, null)
    //! TSDecl: @property @optional width: i32
    //! TSDecl: @property @optional height: i32
    //! TSDecl: @property @optional SAR: @union(Rational, null)
    //! TSDecl: @property @optional colorSpace: ColorSpace
    //! TSDecl: @property @optional colorRange: ColorRange
    //! TSDecl: @property @optional hwFramesCtx: @union(null, HWFramesContext)

    //! @tsdocbegin
    //! The following properties are audio only.
    //! @tsdocend
    //! TSDecl: @property @optional sampleFormat: SampleFormat
    //! TSDecl: @property @optional channelLayout: AChannelLayout
    //! TSDecl: @property @optional sampleRate: i32
//! TSDecl: @end

//! @tsdocbegin
//! `FilterGraph` is a high-performance pipeline for audio/video processing.
//! Detailed information about filter graph: https://ffmpeg.org/ffmpeg-filters.html
//!
//! The user should create a filter graph by using `FilterGraphBuilder`.
//! @tsdocend
//! TSDecl: @class @nonconstructible FilterGraph
class FilterGraph : public ffi::JSObject
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    explicit FilterGraph(std::shared_ptr<PipelineContext> ctx) : ctx_(std::move(ctx)) {}
    ~FilterGraph() override = default;

    //! TSDecl: @method dispose(): void
    ffi::Ret<void> dispose();

    //! @tsdocbegin
    //! Get the properties of frames received at output pad `pad`.
    //! @tsdocend
    //! TSDecl: @method getSinkProperties(pad: string): FilterGraphSinkProperties
    ffi::RetLocal<v8::Value> getSinkProperties(const std::string& pad);

    //! @tsdocbegin
    //! Pushes a frame into the graph. If `frame` is `null`, it indicates the EOF signal.
    //! Throws an exception on failure.
    //! @tsdocend
    //! TSDecl: @method sendFrame(pad: string, frame: @union(Frame, null)): void
    ffi::Ret<void> sendFrame(const std::string& pad, const ffi::Opt<ffi::Class<Frame>>& frame);

    //! @tsdocbegin
    //! Receives a frame from the graph. If `reuse` points to a `Frame` instance in reusable
    //! state, reuses that instance to receive the result, and returns the identical instance.
    //! Otherwise, returns a newly created `Frame` instance.
    //!
    //! If the returned status is not `Success`, `FilterGraphReceiveStatus.frame` is `null`,
    //! and `reuse` is not touched.
    //!
    //! Throws an exception on failure.
    //! @tsdocend
    //! TSDecl: @method receiveFrame(pad: string, reuse: @union(Frame, null))
    //! TSDecl:     : @tuple(FilterGraphReceiveStatus, Frame)
    ffi::RetLocal<v8::Value> receiveFrame(const std::string& pad,
                                          const ffi::Opt<ffi::Class<Frame>>& reuse);

    //! @tsdocbegin
    //! Send a command `cmd` with arguments `args` to the `target` filter in the graph.
    //! `target` can be the filter's name or ID (see FFmpeg documentation).
    //!
    //! If `propagate` is true, command is sent to all the filters matching `target`;
    //! otherwise, it is only sent to the first matched filter.
    //!
    //! Throws an exception on failure.
    //! @tsdocend
    //! TSDecl: @method sendCommand(target: string, cmd: string, args: string,
    //! TSDecl:                     propagate: boolean): void
    ffi::Ret<void> sendCommand(const std::string& target, const std::string& cmd,
                               const std::string& args, bool propagate);

private:
    std::shared_ptr<PipelineContext> ctx_;
};
//! TSDecl: @end

GALLIUM_BINDINGS_MULTIMEDIA_NS_END
#endif //COCOA_GALLIUM_BINDINGS_MULTIMEDIA_FILTERGRAPH_H
