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

#ifndef COCOA_GALLIUM_BINDINGS_RENDERER_IMAGE_H
#define COCOA_GALLIUM_BINDINGS_RENDERER_IMAGE_H

#include "include/core/SkImage.h"
#include "include/core/SkImageGenerator.h"

#include "Gallium/bindings/renderer/Types.h"
#include "Gallium/bindings/renderer/Pixmap.h"
#include "Gallium/bindings/renderer/GpuDirectContext.h"
#include "Gallium/bindings/renderer/Matrix.h"
#include "Gallium/ffi/JSObject.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

class Picture;
class ColorSpace;
class Paint;
class ImageFilter;
class Shader;

//! TSDecl: @enum ImageMemoryMutability
enum class ImageMemoryMutability
{
    kMutableAndCopy,        //! TSDecl: @enumitem MutableAndCopy
    kImmutableAndShare      //! TSDecl: @enumitem ImmutableAndShare
};
//! TSDecl: @end

//! TSDecl: @interface ImageBatchResult
//! TSDecl: @property success: boolean
//! TSDecl: @property @optional error: string
//! TSDecl: @property @optional image: Image
//! TSDecl: @end

//! TSDecl: @interface MakeWithFilterResult
//! TSDecl: @property outSubset: Rect
//! TSDecl: @property offset: @tuple(i32, i32)
//! TSDecl: @property image: Image
//! TSDecl: @end

//! TSDecl: @class @nonconstructible Image
class Image : public ffi::JSObject
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    explicit Image(sk_sp<SkImage> image) : image_(std::move(image)) {}
    ~Image() override = default;

    //! TSDecl: @method @static FromPixmap(mutability: ImageMemoryMutability, pixmap: Pixmap): Image
    static ffi::RetLocal<v8::Value> FromPixmap(ffi::Enum<ImageMemoryMutability> mutability,
                                               PixmapAdapter pixmap);

    //! TSDecl: @method @static FromCompressedTextureData(data: @mem(u8),
    //! TSDecl:                                           width: i32,
    //! TSDecl:                                           height: i32,
    //! TSDecl:                                           type: TextureCompressionType): Image
    static ffi::RetLocal<v8::Value> FromCompressedTextureData(const ffi::Mem<uint8_t>& data,
                                                              int32_t width,
                                                              int32_t height,
                                                              ffi::Enum<SkTextureCompressionType> type);

    //! TSDecl: @method @static DeferredFromEncodedData(encoded: @mem(u8),
    //! TSDecl:                                         alphaType: @union(AlphaType, null)): Image
    static ffi::RetLocal<v8::Value> DeferredFromEncodedData(const ffi::Mem<uint8_t>& encoded,
                                                            ffi::Opt<ffi::Enum<SkAlphaType>> alpha_type);

    //! TSDecl: @method @static DeferredFromEncodedFile(path: string,
    //! TSDecl:                                         alphaType: @union(AlphaType, null)): Image
    static ffi::RetLocal<v8::Value> DeferredFromEncodedFile(const std::string& path,
                                                            ffi::Opt<ffi::Enum<SkAlphaType>> alpha_type);

    //! TSDecl: @method @static BatchFromEncodedFiles(paths: @array(string))
    //! TSDecl:                                       : @generic(Promise, @array(ImageBatchResult))
    static ffi::RetLocal<v8::Value> BatchFromEncodedFile(std::vector<std::string> paths);

    //! TSDecl: @method @static DeferredFromPicture(picture: Picture,
    //! TSDecl:                                     dimensions: @tuple(i32, i32),
    //! TSDecl:                                     matrix: @union(null, Mat3x3),
    //! TSDecl:                                     paint: @union(null, Paint),
    //! TSDecl:                                     f16BitDepth: boolean,
    //! TSDecl:                                     colorSpace: ColorSpace): Image
    static ffi::RetLocal<v8::Value> DeferredFromPicture(ffi::Class<Picture> picture,
                                                        std::tuple<int32_t, int32_t> dimensions,
                                                        ffi::Opt<Mat3x3Adapter> matrix,
                                                        ffi::Opt<ffi::Class<Paint>> paint,
                                                        bool f16_bit_depth,
                                                        ffi::Class<ColorSpace> color_space);

    //! TSDecl: @method @static MakeWithFilter(context: @union(null, GpuDirectContext),
    //! TSDecl:                                src: Image, filter: ImageFilter,
    //! TSDecl:                                subset: Rect, clipBounds: Rect): MakeWithFilterResult
    static ffi::RetLocal<v8::Value> MakeWithFilter(ffi::Opt<ffi::Class<GpuDirectContext>> context,
                                                   ffi::Class<Image> image,
                                                   ffi::Class<ImageFilter> filter,
                                                   const RectAdapter& subset,
                                                   const RectAdapter& clip_bounds);

    // TODO(sora): GPU related factories

    const sk_sp<SkImage>& GetSkImage() const {
        return image_;
    }

    //! TSDecl: @method dispose(): void
    ffi::Ret<void> dispose();

    //! TSDecl: @property @readonly imageInfo: ImageInfo
    ffi::RetLocal<v8::Value> getImageInfo();

    //! TSDecl: @property @readonly width: i32
    ffi::Ret<int32_t> getWidth() {
        return image_->width();
    }

    //! TSDecl: @property @readonly height: i32
    ffi::Ret<int32_t> getHeight() {
        return image_->height();
    }

    //! TSDecl: @property @readonly bounds: Rect
    ffi::RetLocal<v8::Value> getBounds();

    //! TSDecl: @property @readonly uniqueID: u32
    ffi::Ret<uint32_t> getUniqueID() {
        return image_->uniqueID();
    }

    //! TSDecl: @method makeShader(tmx: TileMode, tmy: TileMode, sampling: SamplingOptions,
    //! TSDecl:                    localMatrix: @union(null, Mat3x3)): Shader
    ffi::RetLocal<v8::Value> makeShader(ffi::Enum<SkTileMode> tmx,
                                        ffi::Enum<SkTileMode> tmy,
                                        const SamplingOptionsAdapter& sampling,
                                        const ffi::Opt<Mat3x3Adapter>& local_matrix);

    //! TSDecl: @method makeRawShader(tmx: TileMode, tmy: TileMode, sampling: SamplingOptions,
    //! TSDecl:                      localMatrix: @union(null, Mat3x3)): Shader
    ffi::RetLocal<v8::Value> makeRawShader(ffi::Enum<SkTileMode> tmx,
                                          ffi::Enum<SkTileMode> tmy,
                                          const SamplingOptionsAdapter& sampling,
                                          const ffi::Opt<Mat3x3Adapter>& local_matrix);

    //! TSDecl: @method isTextureBacked(): boolean
    ffi::Ret<bool> isTextureBacked();

    //! TSDecl: @method textureSize(): u64
    ffi::Ret<size_t> textureSize();

    //! TSDecl: @method isValid(context: @union(GpuDirectContext, null)): boolean
    ffi::Ret<bool> isValid(ffi::Opt<ffi::Class<GpuDirectContext>> context);

    //! TSDecl: @method readPixels(ctx: @union(GpuDirectContext, null),
    //! TSDecl:                    dst: Pixmap, srcX: i32, srcY: i32,
    //! TSDecl:                    allowCaching: boolean): boolean
    ffi::Ret<bool> readPixels(ffi::Opt<ffi::Class<GpuDirectContext>> ctx,
                              PixmapAdapter dst, int32_t src_x, int32_t src_y,
                              bool allow_caching);

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

    //! TSDecl: @method scalePixels(dst: Pixmap, sampling: SamplingOptions, allowCaching: boolean): boolean
    ffi::Ret<bool> scalePixels(PixmapAdapter dst, SamplingOptionsAdapter sampling,
                               bool allow_caching);

    //! TSDecl: @method makeSubset(context: @union(GpuDirectContext, null),
    //! TSDecl:                    subset: Rect): @union(Image, null)
    ffi::RetLocal<v8::Value> makeSubset(ffi::Opt<ffi::Class<GpuDirectContext>> context,
                                        RectAdapter subset);

    //! TSDecl: @property @readonly hasMipmaps: boolean
    ffi::Ret<bool> getHasMipmaps() {
        return image_->hasMipmaps();
    }

    //! TSDecl: @property @readonly isProtected: boolean
    ffi::Ret<bool> getIsProtected() {
        return image_->isProtected();
    }

    //! TSDecl: @method withDefaultMipmaps(): @union(Image, null)
    ffi::RetLocal<v8::Value> withDefaultMipmaps();

    //! TSDecl: @method makeNonTextureImage(context: @union(GpuDirectContext, null)): @union(Image, null)
    ffi::RetLocal<v8::Value> makeNonTextureImage(ffi::Opt<ffi::Class<GpuDirectContext>> context);

    //! TSDecl: @method makeRasterImage(context: @union(GpuDirectContext, null),
    //! TSDecl:                         allowCaching: boolean): @union(Image, null)
    ffi::RetLocal<v8::Value> makeRasterImage(ffi::Opt<ffi::Class<GpuDirectContext>> context,
                                             bool allow_caching);

    //! TSDecl: @property @readonly isLazyGenerated: boolean
    ffi::Ret<bool> getIsLazyGenerated() {
        return image_->isLazyGenerated();
    }

    //! TSDecl: @method makeColorSpace(context: @union(GpuDirectContext, null),
    //! TSDecl:                        target: ColorSpace): @union(Image, null)
    ffi::RetLocal<v8::Value> makeColorSpace(ffi::Opt<ffi::Class<GpuDirectContext>> context,
                                            ffi::Class<ColorSpace> target);

    //! TSDecl: @method makeColorTypeAndColorSpace(context: @union(GpuDirectContext, null),
    //! TSDecl:                                    targetColorType: ColorType,
    //! TSDecl:                                    targetCS: ColorSpace): @union(Image, null)
    ffi::RetLocal<v8::Value> makeColorTypeAndColorSpace(ffi::Opt<ffi::Class<GpuDirectContext>> context,
                                                        ffi::Enum<SkColorType> target_color_type,
                                                        ffi::Class<ColorSpace> target_cs);

    //! TSDecl: @method reinterpretColorSpace(cs: ColorSpace): @union(Image, null)
    ffi::RetLocal<v8::Value> reinterpretColorSpace(ffi::Class<ColorSpace> cs);

private:
    sk_sp<SkImage> image_;
    v8::Global<v8::Object> image_info_cache_;
    v8::Global<v8::Object> bounds_cache_;
};
//! TSDecl: @end

GALLIUM_BINDINGS_RENDERER_NS_END
#endif //COCOA_GALLIUM_BINDINGS_RENDERER_IMAGE_H
