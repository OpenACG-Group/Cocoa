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

#ifndef COCOA_GALLIUM_BINDINGS_MULTIMEDIA_PIXELFRAMEVIEW_H
#define COCOA_GALLIUM_BINDINGS_MULTIMEDIA_PIXELFRAMEVIEW_H

#include "Gallium/ffi/JSObject.h"
#include "Gallium/bindings/multimedia/Types.h"
#include "Gallium/bindings/multimedia/Frame.h"
GALLIUM_BINDINGS_MULTIMEDIA_NS_BEGIN

//! @tsdocbegin
//! Helper class for reading/writing the underlying buffer of `Frame` instance.
//!
//! Note that all methods except `applyCropping()` will ignore the cropping settings of frame.
//! To manipulate a cropped frame, `applyCropping()` to create a new frame with cropping applied,
//! then create another `PixelFrameView` for it.
//! @tsdocend
//! TSDecl: @class PixelFrameView
class PixelFrameView : public ffi::JSObject
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    //! @tsdocbegin
    //! Create the instance from a specified `frame` object.
    //! The frame must be a video frame, otherwise it is undefined behaviour.
    //! The new instance accesses the underlying buffer through the provided `frame` object,
    //! when the frame is disposed, all its views will become unavailable (throws an exception
    //! when the user attempts to access them).
    //!
    //! Note that creating a view from a hardware frame is not allowed, and an exception will
    //! be thrown in that case.
    //! @tsdocend
    //! TSDecl: @constructor(frame: Frame)
    explicit PixelFrameView(const ffi::Class<Frame>& frame);
    ~PixelFrameView() override = default;

    //! @tsdocbegin
    //! Returns true if the frame is writable.
    //! A frame is writable, if and only if each of the underlying buffers has only one reference,
    //! namely the one stored in this frame.
    //! @tsdocend
    //! TSDecl: @method isWritable(): boolean
    ffi::Ret<bool> isWritable();

    //! @tsdocbegin
    //! Ensure that the frame data is writable, avoiding data copy if possible.
    //!
    //! Do nothing if the frame is writable, allocate new buffers and copy the data
    //! if it is not.
    //! Throws an exception on failure.
    //! @tsdocend
    //! TSDecl: @method makeWritable(): void
    ffi::Ret<void> makeWritable();

    //! @tsdocbegin
    //! Copy image data in `srcBuffers` to the frame storage. Length of `srcBuffers` and `srcRowBytes`
    //! must be the same and match the number of planes of the frame. The destination frame must be
    //! writable (`Frame.isWritable()` returns true). Neither format conversion nor scaling will be
    //! performed, so the format and dimensions of src buffers must match the frame's specification.
    //!
    //! Throws an exception on failure.
    //! @tsdocend
    //! TSDecl: @method writeImageFrom(srcBuffers: @array(@mem(u8)), srcRowBytes: @array(i32)): void
    ffi::Ret<void> writeImageFrom(const std::vector<v8::Local<v8::Uint8Array>>& src_buffers,
                                  const std::vector<int32_t>& src_row_bytes);

    //! @tsdocbegin
    //! Copy pixels in `src_rect` of `plane` to `dst` buffer.
    //!
    //! Note that `src_rect` is supposed to have the plane's coordinates. E.g. if a 1920x1080 frame is
    //! YUV420 format with Y, U, V three planes, then the rect representing the whole Y plane is
    //! `RectXYWH(0, 0, 1920, 1080)`. However, for U or V plane, it is `RectXYWH(0, 0, 960, 540)` instead.
    //!
    //! Fails if `src_rect` contains pixels out of the plane, or if the `dst` buffer has an insufficient
    //! size to receive the pixels.
    //!
    //! Throws an exception on failure.
    //! @tsdocend
    //! TSDecl: @method readPlaneRectTo(plane: i32, srcRect: @import(renderer) Rect,
    //! TSDecl:                         dst: @mem(u8), dstRowBytes: i32): void
    ffi::Ret<void> readPlaneRectTo(int32_t plane, const renderer::RectAdapter& src_rect,
                                   const ffi::Mem<uint8_t>& dst, int32_t dst_row_bytes);

    //! @tsdocbegin
    //! Creates a `renderer.ColorSpace` according to the color characteristics of the frame.
    //!
    //! Color characteristics are stored in the frame specification that the user can retrieve
    //! by `Frame.specification(mediaType)`. It includes several properties:
    //!   - `colorSpace` defines the YUV colorspace (i.e. a transformation between YUV and RGB);
    //!   - `colorPrimaries` defines the three primary colors and a white point (reference white);
    //!   - `colorTrc` defines a gamma transfer function;
    //!   - `colorRange` defines the visual content value range.
    //!
    //! Among these properties, `colorPrimaries` and `colorTrc` sufficiently describe a color space
    //! (note that property `colorSpace` ONLY specifies the YUV <=> RGB transformation, instead of
    //! a real color space).
    //!
    //! This method uses `colorPrimaries` and `colorTrc` to create a `ColorSpace` object that can
    //! be used in `renderer` module. For the source frame, supported color primaries include BT709,
    //! SMPTE432, BT2020, SMPTE170M, SMPTE240M,  BT470M, BT470BG (BT610), SMPTE431, EBU3213, and
    //! supported trc include BT709, GAMMA22, GAMMA28, SMPTE170M, SMPTE240M, LINEAR,  IEC61966-2-1 (sRGB),
    //! BT2020.
    //!
    //! Returns the created `ColorSpace` object on success, otherwise, returns `fallback`. If `fallback`
    //! is `null`, returns `null`.
    //! @tsdocend
    //! TSDecl: @method resolveColorSpace(fallback: @union(@import(renderer) ColorSpace, null))
    //! TSDecl:                          : @union(@import(renderer) ColorSpace, null)
    ffi::RetLocal<v8::Value> resolveColorSpace(const ffi::Opt<ffi::Class<renderer::ColorSpace>>& fallback);

    //! @tsdocbegin
    //! Copies the pixels into the specified Pixmap `dst`, with scaling and color conversion if
    //! necessary. The resampling options is specified by `sampling` (if `null`, bilinear is used).
    //!
    //! If `dst` does not specify a color space (i.e. `dst.imageInfo.refColorSpace()` returns `null`),
    //! this method uses the result of `resolveColorSpace(sRGB)` as the destination color space.
    //! If `dst` specifies a color space, only sRGB is supported, otherwise, it throws an exception.
    //! Returns a `renderer.ColorSpace` object that indicates the color space of pixels transferred
    //! into `dst`.
    //!
    //! Pixels may be copied more than once during conversion. This operation will not change the
    //! result of `PixelFrameView.isWritable()`.
    //!
    //! Throws an exception if the pixmap is invalid, or the conversion is impossible.
    //! @tsdocend
    //! TSDecl: @method blitToPixmap(srcRegion: @import(renderer) Rect,
    //! TSDecl:                      dstRegion: @import(renderer) Rect,
    //! TSDecl:                      resampler: ScaleResampler,
    //! TSDecl:                      dst: @import(renderer) Pixmap): @import(renderer) ColorSpace
    ffi::RetLocal<v8::Value> blitToPixmap(const renderer::RectAdapter& src_region,
                                          const renderer::RectAdapter& dst_region,
                                          ffi::Enum<utau::ScaleResampler> scale_resampler,
                                          const renderer::PixmapAdapter& dst);

    //! @tsdocbegin
    //! Wraps the frame to a `renderer.Image` instance without memory copy. The result image shares
    //! the same underlying buffer with the frame. Note that this operation makes a writable frame
    //! not writable, since new buffer references are created.
    //!
    //! It requires valid color characteristics to be set (color primaries, range, and trc), i.e.
    //! those properties must not be `UNSPECIFIED`, otherwise it fails.
    //!
    //! Not all formats and color spaces support this, and throws an exception on failure.
    //! @tsdocend
    //! TSDecl: @method wrapToImage(): @import(renderer) Image
    ffi::RetLocal<v8::Value> wrapToImage();

    //! @tsdocbegin
    //! Applies the cropping settings which is specified in frame specification. Returns a new `Frame`
    //! instance that shares the same underlying buffer, but its width, height, and buffer views has
    //! been adjusted according to the cropping settings. All cropping settings will be set to 0 in
    //! the returned `Frame` instance.
    //!
    //! In all cases, the cropping boundaries will be rounded to the inherent alignment of the pixel
    //! format.
    //!
    //! Specially, if all the cropping settings are 0, it acts like `Frame.clone()`.
    //! Throws an exception on failure.
    //! @tsdocend
    //! TSDecl: @method applyCropping(): Frame
    ffi::RetLocal<v8::Value> applyCropping();

private:
    FrameViewProxy proxy_;
};
//! TSDecl: @end

GALLIUM_BINDINGS_MULTIMEDIA_NS_END
#endif //COCOA_GALLIUM_BINDINGS_MULTIMEDIA_PIXELFRAMEVIEW_H
