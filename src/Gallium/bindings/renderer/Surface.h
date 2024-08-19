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

#ifndef COCOA_GALLIUM_BINDINGS_RENDERER_SURFACE_H
#define COCOA_GALLIUM_BINDINGS_RENDERER_SURFACE_H

#include "include/core/SkSurface.h"

#include "Gallium/ffi/JSObject.h"
#include "Gallium/bindings/renderer/Types.h"
#include "Gallium/bindings/renderer/ImageInfo.h"
#include "Gallium/bindings/renderer/Pixmap.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

//! TSDecl: @interface SurfaceProps
struct SurfaceProps
{
    SkSurfaceProps Make() const {
        uint32_t flags = 0;
        if (use_device_independent_fonts && *use_device_independent_fonts)
            flags |= SkSurfaceProps::kUseDeviceIndependentFonts_Flag;
        if (dynamic_msaa && *dynamic_msaa)
            flags |= SkSurfaceProps::kDynamicMSAA_Flag;
        if (always_dither && *always_dither)
            flags |= SkSurfaceProps::kAlwaysDither_Flag;
        return {flags, pixel_geometry ? **pixel_geometry
                                      : SkPixelGeometry::kUnknown_SkPixelGeometry};
    }

    //! TSDecl: @property @optional useDeviceIndependentFonts: boolean
    ffi::Opt<bool> use_device_independent_fonts;

    //! TSDecl: @property @optional dynamicMSAA: boolean
    ffi::Opt<bool> dynamic_msaa;

    //! TSDecl: @property @optional alwaysDither: boolean
    ffi::Opt<bool> always_dither;

    //! TSDecl: @property @optional pixelGeometry: PixelGeometry
    ffi::Opt<ffi::Enum<SkPixelGeometry>> pixel_geometry;
};
//! TSDecl: @end

//! TSDecl: @class @nonconstructible Surface
class Surface : public ffi::JSObject
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    Surface(std::shared_ptr<v8::BackingStore> store,
            v8::Local<v8::Uint8Array> store_u8array, sk_sp<SkSurface> surface);

    //! TSDecl: @method @static MakeRaster(info: ImageInfo, props: @union(null, SurfaceProps)): Surface
    static ffi::RetLocal<v8::Value> MakeRaster(ffi::Class<ImageInfo> info,
                                               ffi::Opt<ffi::IFace<SurfaceProps>> props);

    //! TSDecl: @method @static MakeFromPixmap(pixmap: Pixmap, props: @union(null, SurfaceProps)): Surface
    static ffi::RetLocal<v8::Value> MakeFromPixmap(ffi::IFace<Pixmap> pixmap,
                                                   ffi::Opt<ffi::IFace<SurfaceProps>> props);

    //! TSDecl: @method @static MakeNull(width: i32, height: i32): Surface
    static ffi::RetLocal<v8::Value> MakeNull(int32_t width, int32_t height);

    //! @tsdocbegin
    //! Destroy the Surface and free all its memory. The attached `Canvas` instance
    //! will also be disposed automatically.
    //! @tsdocend
    //! TSDecl: @method dispose(): void
    ffi::Ret<void> dispose();

    //! TSDecl: @property @readonly width: i32
    ffi::Ret<int32_t> getWidth() {
        return surface_->width();
    }

    //! TSDecl: @property @readonly height: i32
    ffi::Ret<int32_t> getHeight() {
        return surface_->height();
    }

    //! TSDecl: @property @readonly imageInfo: ImageInfo
    ffi::RetLocal<v8::Object> getImageInfo();

    //! TSDecl: @property @readonly generationID: u32
    ffi::Ret<uint32_t> getGenerationID() {
        return surface_->generationID();
    }

    //! TSDecl: @property @readonly canvas: Canvas
    ffi::RetLocal<v8::Value> getCanvas();

    //! TSDecl: @method notifyContentWillChange(discard: boolean): void
    ffi::Ret<void> notifyContentWillChange(bool discard);

    //! TSDecl: @method makeImageSnapshot(bounds: @union(Rect, null)): @union(Image, null)
    ffi::RetLocal<v8::Value> makeImageSnapshot(ffi::Opt<RectAdapter> bounds);

    //! TSDecl: @method peekPixels(): @union(null, Pixmap)
    ffi::RetLocal<v8::Value> peekPixels();

    //! TSDecl: @method readPixels(dst: Pixmap, srcX: i32, srcY: i32): boolean
    ffi::Ret<bool> readPixels(PixmapAdapter dst, int32_t src_x, int32_t src_y);

    //! TSDecl: @method writePixels(src: Pixmap, srcX: i32, srcY: i32): void
    ffi::Ret<void> writePixels(PixmapAdapter src, int32_t src_x, int32_t src_y);

    //! TSDecl: @method asyncRescaleAndReadPixels(info: ImageInfo,
    //! TSDecl:                                   srcRect: Rect,
    //! TSDecl:                                   rescaleGamma: ImageRescaleGamma,
    //! TSDecl:                                   rescaleMode: ImageRescaleMode)
    //! TSDecl:                                   : @generic(Promise, ImageAsyncReadResult)
    ffi::RetLocal<v8::Value>
    asyncRescaleAndReadPixels(ffi::Class<ImageInfo> image_info,
                              RectAdapter src_rect,
                              ffi::Enum<SkImage::RescaleGamma> rescale_gamma,
                              ffi::Enum<SkImage::RescaleMode> rescale_mode);

    //! TSDecl: @method asyncRescaleAndReadPixelsYUV420(yuvColorSpace: YUVColorSpace,
    //! TSDecl:                                         dstColorSpace: ColorSpace,
    //! TSDecl:                                         srcRect: Rect,
    //! TSDecl:                                         dstSize: @tuple(i32, i32),
    //! TSDecl:                                         rescaleGamma: ImageRescaleGamma,
    //! TSDecl:                                         rescaleMode: ImageRescaleMode)
    //! TSDecl:                                         : @generic(Promise, ImageAsyncReadResult)
    ffi::RetLocal<v8::Value>
    asyncRescaleAndReadPixelsYUV420(ffi::Enum<SkYUVColorSpace> yuv_color_space,
                                    ffi::Class<ColorSpace> dst_color_space,
                                    RectAdapter src_rect,
                                    const std::tuple<int32_t, int32_t>& dst_size,
                                    ffi::Enum<SkImage::RescaleGamma> rescale_gamma,
                                    ffi::Enum<SkImage::RescaleMode> rescale_mode);

    //! TSDecl: @method asyncRescaleAndReadPixelsYUVA420(yuvColorSpace: YUVColorSpace,
    //! TSDecl:                                          dstColorSpace: ColorSpace,
    //! TSDecl:                                          srcRect: Rect,
    //! TSDecl:                                          dstSize: @tuple(i32, i32),
    //! TSDecl:                                          rescaleGamma: ImageRescaleGamma,
    //! TSDecl:                                          rescaleMode: ImageRescaleMode)
    //! TSDecl:                                          : @generic(Promise, ImageAsyncReadResult)
    ffi::RetLocal<v8::Value>
    asyncRescaleAndReadPixelsYUVA420(ffi::Enum<SkYUVColorSpace> yuv_color_space,
                                     ffi::Class<ColorSpace> dst_color_space,
                                     RectAdapter src_rect,
                                     const std::tuple<int32_t, int32_t>& dst_size,
                                     ffi::Enum<SkImage::RescaleGamma> rescale_gamma,
                                     ffi::Enum<SkImage::RescaleMode> rescale_mode);

    // TODO(sora): implement other methods.

private:
    std::shared_ptr<v8::BackingStore>   store_;
    v8::Global<v8::Uint8Array>          store_u8array_;
    sk_sp<SkSurface>                    surface_;
    v8::Global<v8::Object>              image_info_cache_;
    v8::Global<v8::Object>              canvas_cache_;
};
//! TSDecl: @end

GALLIUM_BINDINGS_RENDERER_NS_END
#endif //COCOA_GALLIUM_BINDINGS_RENDERER_SURFACE_H
