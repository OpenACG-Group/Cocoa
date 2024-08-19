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

#ifndef COCOA_GALLIUM_BINDINGS_MULTIMEDIA_FRAME_H
#define COCOA_GALLIUM_BINDINGS_MULTIMEDIA_FRAME_H

#include <list>

#include "Core/Errors.h"
#include "Utau/Utau.h"
#include "Gallium/bindings/multimedia/ffwrappers/libavutil-frame.h"
#include "Gallium/bindings/multimedia/Types.h"
#include "Gallium/bindings/multimedia/AChannelLayout.h"
#include "Gallium/bindings/renderer/Color.h"
#include "Gallium/bindings/renderer/ImageInfo.h"
#include "Gallium/bindings/renderer/Pixmap.h"
#include "Gallium/ffi/JSObject.h"
#include "Gallium/ffi/Interface.h"
#include "Gallium/ffi/Enum.h"
#include "Gallium/ffi/Class.h"
GALLIUM_BINDINGS_MULTIMEDIA_NS_BEGIN

class HWFramesContext;

//! TSDecl: @interface FrameSpecification
struct FrameSpecification
{
    //! @tsdocbegin
    //! Video only. The pixel format of the frame. Maybe `PixelFormat.kNone`
    //! if the format is not set, unknown, or the frame is a hardware frame.
    //!
    //! set spec: optional;
    //! get spec: guaranteed for video frames.
    //! @tsdocend
    //! TSDecl: @property @optional pixelFormat: PixelFormat
    ffi::Opt<ffi::Enum<PixelFormat>> pixel_format;

    //! @tsdocbegin
    //! Audio only. The sample format of the frame. Maybe `SampleFormat.None`
    //! if the format is not set or unknown.
    //!
    //! set spec: optional;
    //! get spec: guaranteed for audio frames.
    //! @tsdocend
    //! TSDecl: @property @optional sampleFormat: SampleFormat
    ffi::Opt<ffi::Enum<SampleFormat>> sample_format;

    //! @tsdocbegin
    //! Video only. Width and height in pixels.
    //!
    //! set spec: optional;
    //! get spec: guaranteed for video frames.
    //! @tsdocend
    //! TSDecl: @property @optional width: i32
    ffi::Opt<int32_t> width;

    //! TSDecl: @property @optional height: i32
    ffi::Opt<int32_t> height;

    //! @tsdocbegin
    //! Audio only. Audio samples (per channel) described by the frame.
    //!
    //! set spec: optional;
    //! get spec: guaranteed for audio frames.
    //! @tsdocend
    //! TSDecl: @property @optional nbSamples: i32
    ffi::Opt<int32_t> nb_samples;

    //! @tsdocbegin
    //! Video only. Picture type of the frame.
    //!
    //! set spec: optional;
    //! get spec: guaranteed for video frames.
    //! @tsdocend
    //! TSDecl: @property @optional pictureType: PictureType
    ffi::Opt<ffi::Enum<PictureType>> picture_type;

    //! @tsdocbegin
    //! Video only. Sample aspect ratio.
    //!
    //! set spec: optional;
    //! get spec: guaranteed for video frames, `0/1` of unknown.
    //! @tsdocend
    //! TSDecl: @property @optional SAR: Rational
    ffi::OptLocal<v8::Object> sar;

    //! @tsdocbegin
    //! Presentation timestamp, in the stream timebase.
    //!
    //! set spec: optional;
    //! get spec: may be absent if unknown.
    //! @tsdocend
    //! TSDecl: @property @optional pts: i64
    ffi::Opt<int64_t> pts;

    //! @tsdocbegin
    //! Audio only. Audio sample rate in Hz.
    //!
    //! set spec: optional.
    //! get spec: guaranteed for audio frames.
    //! @tsdocend
    //! TSDecl: @property @optional sampleRate: i32
    ffi::Opt<int32_t> sample_rate;

    //! @tsdocbegin
    //! Frame flags. See `FrameFlags` for more details.
    //!
    //! set spec: optional;
    //! get spec: guaranteed.
    //! @tsdocend
    //! TSDecl: @property @optional flags: FrameFlags
    ffi::Opt<ffi::Enum<FrameFlags>> flags;

    //! @tsdocbegin
    //! Video only. Color characteristics.
    //!
    //! set spec: optional;
    //! get spec: guaranteed for video frames.
    //! @tsdocend
    //! TSDecl: @property @optional colorRange: ColorRange
    ffi::Opt<ffi::Enum<ColorRange>> color_range;

    //! TSDecl: @property @optional colorPrimaries: ColorPrimaries
    ffi::Opt<ffi::Enum<ColorPrimaries>> color_primaries;

    //! TSDecl: @property @optional colorTrc: ColorTransferCharacteristic
    ffi::Opt<ffi::Enum<ColorTransferCharacteristic>> color_trc;

    //! TSDecl: @property @optional colorSpace: ColorSpace
    ffi::Opt<ffi::Enum<ColorSpace>> color_space;

    //! TSDecl: @property @optional chromaLocation: ChromaLocation
    ffi::Opt<ffi::Enum<ChromaLocation>> chroma_location;

    //! @tsdocbegin
    //! Audio only. Audio channel layout.
    //!
    //! set spec: optional;
    //! get spec: guaranteed for audio frames.
    //! @tsdocend
    //! TSDecl: @property @optional channelLayout: AChannelLayout
    ffi::OptLocal<v8::Object> channel_layout;

    //! @tsdocbegin
    //! Presentation duration the frame lasts.
    //!
    //! set spec: optional;
    //! get spec: may be absent if unknown.
    //! @tsdocend
    //! TSDecl: @property @optional duration: i64
    ffi::Opt<int64_t> duration;

    //! @tsdocbegin
    //! Video only. Hardware frames context.
    //!
    //! set spec: not acceptable;
    //! get spec: only absent if not a hardware frame.
    //! @tsdocend
    //! TSDecl: @property @optional hwFramesCtx: HWFramesContext
    ffi::OptLocal<v8::Object> hw_frames_ctx;

    //! @tsdocbegin
    //! Frame timestamp estimated using various heuristics, in stream time base.
    //!
    //! set spec: not acceptable;
    //! get spec: guaranteed.
    //! @tsdocend
    //! TSDecl: @property @optional bestEffortTimestamp: i64
    ffi::Opt<int64_t> best_effort_timestamp;

    //! @tsdocbegin
    //! Video only. The number of pixels to discard from the the top/bottom/left/right
    //! border of the frame to obtain the sub-rectangle of the frame intended for
    //! presentation.
    //!
    //! set spec: optional;
    //! get spec: guaranteed for video frames.
    //! @tsdocend
    //! TSDecl: @property @optional cropTop: u64
    ffi::Opt<size_t> crop_top;

    //! TSDecl: @property @optional cropBottom: u64
    ffi::Opt<size_t> crop_bottom;

    //! TSDecl: @property @optional cropLeft: u64
    ffi::Opt<size_t> crop_left;

    //! TSDecl: @property @optional cropRight: u64
    ffi::Opt<size_t> crop_right;
};
//! TSDecl: @end

class FrameViewProxy;

//! @tsdocbegin
//! A reference to a underlying buffer that stores the decoded (raw) audio or video data.
//! One frame could contain multiple separated buffers, which are called planes, and
//! how to interpret the planes and data in them depends on the format of the frame.
//!
//! Like `Packet` object, the underlying buffer of `Frame` is reference counted. Each
//! instance of `Frame` should be treated as a reference to its underlying buffer, and
//! when there is no reference to a underlying buffer, it will be freed. However, note
//! that not all the references to a certain underlying buffer are comes from `Frame`
//! instances, which means it may be referenced implicitly by internal code that the user
//! cannot touch. You should NEVER assume when a underlying buffer will be freed.
//!
//! If a frame is disposed via `disposeReusable()`, it will become a reusable frame.
//! That means to delete the reference of the current underlying buffer, but remain other
//! allocated memory as more as possible, and then refill the instance with another
//! underlying buffer. It is useful to decrease the overhead caused by frequent object
//! allocation.
//! @tsdocend
//! TSDecl: @class @nonconstructible Frame
class Frame : public ffi::JSObject
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    explicit Frame(AVFrame *frame) : frame_(frame) {}
    ~Frame() override;

    g_nodiscard AVFrame *GetAVFrame() const {
        CHECK(frame_ && "phantom Frame instance");
        return frame_;
    }

    g_nodiscard bool IsReusable() const {
        return (frame_ && GetDisposeState() == DisposeState::kDisposed);
    }

    // only used internally to reuse a `Frame` instance
    void ResetDisposeState() {
        NotifyDisposeState(DisposeState::kNot);
    }

    void AddFrameViewProxy(FrameViewProxy *proxy);
    void RemoveFrameViewProxy(FrameViewProxy *proxy);

    //! @tsdocbegin
    //! Make a `Frame` instance and set its properties to specified values, without
    //! allocating any underlying buffers. It creates an instance in the "reusable"
    //! state, just like the state when `disposeReusable()` is called on an allocated
    //! normal frame.
    //!
    //! Absent property in `spec` will be its default value.
    //! @tsdocend
    //! TSDecl: @method @static MakeUnallocated(spec: FrameSpecification): Frame
    static ffi::RetLocal<v8::Value> MakeUnallocated(const ffi::IFace<FrameSpecification>& spec);

    //! @tsdocbegin
    //! Equivalent to creating an instance by `MakeUnallocated(spec)`, and then allocate the
    //! memory by `allocate()` method.
    //! @tsdocend
    //! TSDecl: @method @static MakeAllocated(spec: FrameSpecification): Frame
    // THIS IS IMPLEMENTED IN JAVASCRIPT.

    //! @tsdocbegin
    //! Allocate new underlying buffer(s) for audio or video data.
    //! The frame must be in "reusable" state, which means it has been disposed via
    //! `disposeReusable(spec)`, or created via `MakeUnallocated(spec)`.
    //!
    //! Either `spec.pixelFormat` or `spec.sampleFormat` must be set, and:
    //!   - if `spec.pixelFormat` is set, `spec.width`, `spec.height` must be set;
    //!   - if `spec.sampleFormat` is set, `spec.nbSamples`, `spec.channelLayout` must be set.
    //!
    //! If the frame has been allocated, fails.
    //! Throws an exception on failure. In that case, the frame is not touched.
    //!
    //! Note that this method only allocates normal memory. For frames which are expected
    //! to carry hw buffers, allocate them using `HWFramesContext.getBuffer()` instead.
    //! @tsdocend
    //! TSDecl: @method allocate(): void
    ffi::Ret<void> allocate();

    //! @tsdocbegin
    //! Create a new `Frame` instance that shares the same underlying buffer.
    //! This operation increases the refcount.
    //! @tsdocend
    //! TSDecl: @method clone(): Frame
    ffi::RetLocal<v8::Value> clone();

    //! @tsdocbegin
    //! Delete the reference to the underlying buffer and invalidate the `Frame`
    //! instance. This operation decreases the refcount.
    //! @tsdocend
    //! TSDecl: @method dispose(): void
    ffi::Ret<void> dispose();

    //! @tsdocbegin
    //! Like `dispose()`, but the `Frame` instance will be reusable, and can be reused
    //! by functions like `CodecContext.receiveFrame(frame)`. This operation also reset the
    //! frame properties to what the `spec` specifies, if it is provided.
    //! This operation decreases the refcount.
    //! Throws an exception if `spec` is invalid. In that case, frame is disposed, reusable,
    //! but all the properties are set to default values.
    //! @tsdocend
    //! TSDecl: @method disposeReusable(spec: FrameSpecification): void
    ffi::Ret<void> disposeReusable(const ffi::IFace<FrameSpecification>& spec);

    //! @tsdocbegin
    //! Get the frame specification. Depending on the media type of the frame, some properties
    //! are filled while other fields are not. If a property is unavailable, it will be absent.
    //!
    //! See `FrameSpecification` for more details.
    //!
    //! This function does not cache the value. Returns a new object each call.
    //! @tsdocend
    //! TSDecl: @method specification(type: MediaType): FrameSpecification
    ffi::Ret<ffi::IFace<FrameSpecification>> specification(ffi::Enum<MediaType> type);

    //! @tsdocbegin
    //! Updates the specified frame properties. Properties not assigned in `spec` will not change.
    //! This function could cause fatal errors if wrong properties are set. Do NOT use it unless
    //! you know what are you doing.
    //! @tsdocend
    //! TSDecl: @method updateSpecification(spec: FrameSpecification): void
    ffi::Ret<void> updateSpecification(const ffi::IFace<FrameSpecification>& spec);

private:
    void NotifyViewProxiesOfDispose();

    AVFrame *frame_;
    std::list<FrameViewProxy*> view_proxies_;
};
//! TSDecl: @end

// A class controls the access of a frame from a view object, including
// `PixelFrameView` and `SampleFrameView`. It allows the view object to know
// the disposal of `Frame`.
class FrameViewProxy
{
public:
    explicit FrameViewProxy(const ffi::Class<Frame>& frame);
    ~FrameViewProxy();

    void NotifyParentDispose();

    AVFrame *TryRetrieveHandle() const;

private:
    ffi::ClassInstance<Frame> frame_;
};

// A shorthand of `FrameViewProxy::TryRetrieveHandle()`
#define FRAME_VIEW_PROXY_RETRIEVE_HANDLE(var)                                \
    AVFrame *var = proxy_.TryRetrieveHandle();                               \
    if (!var) {                                                              \
        return ffi::Fail(ffi::kErr, "the `Frame` object has been disposed"); \
    }

GALLIUM_BINDINGS_MULTIMEDIA_NS_END
#endif //COCOA_GALLIUM_BINDINGS_MULTIMEDIA_FRAME_H
