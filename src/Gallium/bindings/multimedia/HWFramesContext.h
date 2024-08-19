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

#ifndef COCOA_GALLIUM_BINDINGS_MULTIMEDIA_HWFRAMESCONTEXT_H
#define COCOA_GALLIUM_BINDINGS_MULTIMEDIA_HWFRAMESCONTEXT_H

#include "Gallium/bindings/multimedia/ffwrappers/libavutil.h"
#include "Gallium/bindings/multimedia/Types.h"
#include "Gallium/ffi/Enum.h"
#include "Gallium/ffi/JSObject.h"
#include "Gallium/ffi/Class.h"
GALLIUM_BINDINGS_MULTIMEDIA_NS_BEGIN

class Frame;
class HWDeviceContext;

//! TSDecl: @interface HWFrameTransferFormatsInfo
    //! @tsdocbegin
    //! Possible source formats when transfer the data to the frame.
    //! @tsdocend
    //! TSDecl: @property src: @array(PixelFormat)

    //! @tsdocbegin
    //! Possible destination formats when transfer the data from the frame.
    //! @tsdocend
    //! TSDecl: @property dst: @array(PixelFormat)
//! TSDecl: @end

//! TSDecl: @interface HWFramesConstraints
    //! TSDecl: @property formats: @array(PixelFormat)

    //! @tsdocbegin
    //! The minimum size of frames. Zero if not known.
    //! @tsdocend
    //! TSDecl: @property minWidth: i32
    //! TSDecl: @property minHeight: i32

    //! @tsdocbegin
    //! The maximum size of frames. Infinity if not known / no limit.
    //! @tsdocend
    //! TSDecl: @property maxWidth: i32
    //! TSDecl: @property maxHeight: i32
//! TSDecl: @end

//! @tsdocbegin
//! This class describes a set or pool of "hardware" frames (i.e. those with
//! data not located in normal system memory). All the frames in the pool are
//! assumed to be allocated in the same way and interchangeable.
//! @tsdocend
//! TSDecl: @class @nonconstructible HWFramesContext
class HWFramesContext : public ffi::JSObject
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    explicit HWFramesContext(AVBufferRef *hwframes_ctx) : hwframes_ctx_(hwframes_ctx) {}
    ~HWFramesContext() override;

    g_nodiscard AVBufferRef *GetAVBufferRef() const {
        return hwframes_ctx_;
    }

    //! @tsdocbegin
    //! Returns the constraints of hardware frames on current platform.
    //! These constraints apply on creating the `HWFramesContext` via `HWFrameContext.Make()`.
    //! This method may cause the initialization of device context.
    //! @tsdocend
    //! TSDecl: @method @static GetPlatformConstraints(device: HWDeviceContext): HWFramesConstraints
    static ffi::RetLocal<v8::Value> GetPlatformConstraints(ffi::Class<HWDeviceContext> device);

    //! @tsdocbegin
    //! Creates an instance of `HWFramesContext`. The underlying device, backend, and
    //! all the related low-level resources are selected, created, and managed by Cocoa
    //! internally.
    //!
    //! This method may cause the initialization of device context.
    //! Once the first `HWFramesContext` is created, the corresponding device
    //! context has been initialized globally. When another `HWFramesContext` need to be
    //! created, it will use the same underlying device context, and share common resources.
    //!
    //! The `format` argument specifies the actual data layout in the device memory,
    //! and choosing which format depends on your purpose, but for most cases, using NV12 format
    //! will always produce the correct result.
    //!
    //! Throws an exception on failure.
    //!
    //! @param width, height        Dimensions of frames that are created from the context.
    //! @param format               Pixel format, it specifies the actual data layout
    //!                             of frames in the device memory.
    //! @param initialPoolSize      An integer >= 0, specifies the initial size of the pool.
    //!                             Some devices do not support dynamic pool size, and in that
    //!                             case, this is also the maximum size of the pool.
    //! @tsdocend
    //! TSDecl: @method @static Make(device: HWDeviceContext, width: i32, height: i32, format: PixelFormat,
    //! TSDecl:                      initialPoolSize: i32): HWFramesContext
    static ffi::RetLocal<v8::Value> Make(ffi::Class<HWDeviceContext> device, int32_t width, int32_t height,
                                         ffi::Enum<PixelFormat> format, int32_t initial_pool_size);

    //! @tsdocbegin
    //! Copy data to or from a hw frame. At least one of dst/src must be hw frame.
    //!
    //! Supposing the return value of `queryTransferFormats()` is `formats`, then:
    //!  - if `src` is a hw frame, the format of `dst` (if set) must use one of the `formats.dst`;
    //!  - if `dst` is a hw frame, the format of `src` must use one of the `formats.src`.
    //!
    //! If `dst` is a frame in "reusable" state, allocates memory for it. In that case, if
    //! a format is set in `FrameSpecification`, this format will be used, otherwise the first
    //! acceptable format will be chosen. The dimensions (if set) must matches the dimensions
    //! of `src`, since not all devices support transferring a sub-rectangle of the whole surface.
    //!
    //! If `dst` has been allocated, writes contents into the `dst` frame directly.
    //!
    //! Throws a exception on failure. In that case, `dst` is not touched.
    //! @tsdocend
    //! TSDecl: @method @static Transfer(dst: Frame, src: Frame): void
    static ffi::Ret<void> Transfer(ffi::Class<Frame> dst, ffi::Class<Frame> src);

    // TODO(sora): Map(), Unmap()

    //! @tsdocbegin
    //! Allocate a new frame attached to the HWFramesContext.
    //! If `reuse` is not null, the provided `Frame` instance will be reused, refilled
    //! with newly allocated buffers. In that case, returns the original `reuse` instance.
    //! Otherwise, a new `Frame` instance is created and returned.
    //! Throws an exception on failure.
    //!
    //! Note that if `reuse` is not null, it must be in "reusable" state, with the default
    //! specification (i.e. created by `Frame.MakeUnallocated({})`, or disposed by
    //! `Frame.disposeReusable({})`).
    //! @tsdocend
    //! TSDecl: @method getBuffer(reuse: @union(Frame, null)): Frame
    ffi::RetLocal<v8::Value> getBuffer(const ffi::Opt<ffi::Class<Frame>>& reuse);

    //! @tsdocbegin
    //! Returns true if the underlying context is identical.
    //! @tsdocend
    //! TSDecl: @method equalTo(other: HWFramesContext): boolean
    ffi::Ret<bool> equalTo(ffi::Class<HWFramesContext> other);

    //! @tsdocbegin
    //! Get a list of possible formats usable when transfer the data from/to frame created
    //! from this `HWFramesContext`.
    //! Throws an exception on failure.
    //! @tsdocend
    //! TSDecl: @method queryTransferFormats(): HWFrameTransferFormatsInfo
    ffi::RetLocal<v8::Value> queryTransferFormats();

private:
    AVBufferRef *hwframes_ctx_;
};
//! TSDecl: @end

GALLIUM_BINDINGS_MULTIMEDIA_NS_END
#endif //COCOA_GALLIUM_BINDINGS_MULTIMEDIA_HWFRAMESCONTEXT_H
