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

#ifndef COCOA_GALLIUM_BINDINGS_GLAMOR_CKIMAGEWRAP_H
#define COCOA_GALLIUM_BINDINGS_GLAMOR_CKIMAGEWRAP_H

#include "include/core/SkImage.h"
#include "include/v8.h"

#include "Gallium/bindings/glamor/Types.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

//! TSDecl: class CkImage
class CkImageWrap : public ExportableObjectBase
{
public:
    explicit CkImageWrap(sk_sp<SkImage> image);

    g_nodiscard inline const sk_sp<SkImage>& getImage() const {
        CheckDisposedOrThrow();
        return image_;
    }

    //! TSDecl: function MakeFromEncodedData(buffer: Uint8Array): Promise<CkImage>
    static v8::Local<v8::Value> MakeFromEncodedData(v8::Local<v8::Value> buffer);

    //! TSDecl: function MakeFromEncodedFile(path: string): Promise<CkImage>
    static v8::Local<v8::Value> MakeFromEncodedFile(const std::string& path);

    //! TSDecl: function MakeFromVideoBuffer(vbo: utau.VideoBuffer): CkImage
    static v8::Local<v8::Value> MakeFromVideoBuffer(v8::Local<v8::Value> vbo);

    //! TSDecl: function MakeDeferredFromPicture(picture: CkPicture,
    //!                                          width: number,
    //!                                          height: number,
    //!                                          matrix: CkMat3x3 | null,
    //!                                          paint: CkPaint | null,
    //!                                          bitDepth: Enum<ImageBitDepth>,
    //!                                          colorSpace: Enum<ColorSpace>): CkImage
    static v8::Local<v8::Value> MakeDeferredFromPicture(v8::Local<v8::Value> picture,
                                                        int32_t width,
                                                        int32_t height,
                                                        v8::Local<v8::Value> matrix,
                                                        v8::Local<v8::Value> paint,
                                                        int32_t bit_depth,
                                                        int32_t color_space);

    //! TSDecl: function MakeFromMemory(buffer: TypedArray,
    //!                                 info: CkImageInfo,
    //!                                 rowBytes: number,
    //!                                 sharedPixelMemory: boolean): CkImage
    static v8::Local<v8::Value> MakeFromMemoryCopy(v8::Local<v8::Value> buffer,
                                                   v8::Local<v8::Value> info,
                                                   int64_t row_bytes,
                                                   bool shared_pixel_memory);

    //! TSDecl: function MakeFromCompressedTextureData(data: TypedArray,
    //!                                                width: number,
    //!                                                height: number,
    //!                                                type: Enum<TextureCompressionType>): CkImage
    static v8::Local<v8::Value> MakeFromCompressedTextureData(v8::Local<v8::Value> data,
                                                              int32_t width,
                                                              int32_t height,
                                                              int32_t compress_type);

    //! TSDecl: readonly width: number
    g_nodiscard int32_t getWidth();

    //! TSDecl: readonly height: number
    g_nodiscard int32_t getHeight();

    //! TSDecl: readonly alphaType: number
    g_nodiscard uint32_t getAlphaType();

    //! TSDecl: readonly colorType: number
    g_nodiscard uint32_t getColorType();

    //! TSDecl: function uniqueId(): number
    g_nodiscard uint32_t uniqueId();

    //! TSDecl: function dispose(): void
    void dispose();

    //! TSDecl: function isDisposed(): boolean
    bool isDisposed();

    //! TSDecl: function hasMipmaps(): boolean
    bool hasMipmaps();

    //! TSDecl: function withDefaultMipmaps(): CkImage
    v8::Local<v8::Value> withDefaultMipmaps();

    //! TSDecl: function isTextureBacked(): boolean
    bool isTextureBacked();

    //! TSDecl: function approximateTextureSize(): number
    size_t approximateTextureSize();

    //! TSDecl: function isValid(context: GpuDirectContext | null): boolean
    bool isValid(v8::Local<v8::Value> context);

    //! TSDecl: function makeNonTextureImage(context: GpuDirectContext | null): CkImage
    v8::Local<v8::Value> makeNonTextureImage(v8::Local<v8::Value> gpu_context);

    //! TSDecl: function makeRasterImage(context: GpuDirectContext | null): CkImage
    v8::Local<v8::Value> makeRasterImage(v8::Local<v8::Value> context);

    //! TSDecl: interface FilteredImage {
    //!   image: CkImage;
    //!   offset: CkPoint;
    //!   subset: CkRect;
    //! }

    //! TSDecl: function makeWithFilter(context: GpuDirectContext | null,
    //!                                 filter: CkImageFilter,
    //!                                 subset: CkRect,
    //!                                 clipBounds: CkRect): FilteredImage
    v8::Local<v8::Value> makeWithFilter(v8::Local<v8::Value> gpu_context,
                                        v8::Local<v8::Value> filter,
                                        v8::Local<v8::Value> subset,
                                        v8::Local<v8::Value> clip_bounds);


    //! TSDecl: function peekPixels(scopeCallback: (pixmap: CkPixmap) => T): T
    v8::Local<v8::Value> peekPixels(v8::Local<v8::Value> scope_callback);

    //! TSDecl: function readPixels(dstInfo: CkImageInfo,
    //!                             dstBuffer: TypedArray,
    //!                             dstRowBytes: number,
    //!                             srcX: number,
    //!                             srcY: number): void
    void readPixels(v8::Local<v8::Value> dst_info, v8::Local<v8::Value> dst_buffer,
                    int32_t dst_row_bytes, int32_t src_x, int32_t src_y);

    //! TSDecl: function scalePixels(dstInfo: CkImageInfo,
    //!                              dstBuffer: TypedArray,
    //!                              dstRowBytes: number,
    //!                              sampling: Enum<Sampling>): void
    void scalePixels(v8::Local<v8::Value> dst_info, v8::Local<v8::Value> dst_buffer,
                     int32_t dst_row_bytes, int32_t sampling);

    //! TSDecl: function makeSubset(context: GpuDirectContext | null, subset: CkRect): CkImage
    v8::Local<v8::Value> makeSubset(v8::Local<v8::Value> gpu_context, v8::Local<v8::Value> subset);

    //! TSDecl: function makeShader(tmx: Enum<TileMode>, tmy: Enum<TileMode>,
    //!                             sampling: Enum<Sampling>, local_matrix: CkMat3x3 | null): CkShader | null
    v8::Local<v8::Value> makeShader(int32_t tmx, int32_t tmy, int32_t sampling,
                                    v8::Local<v8::Value> local_matrix);

    //! TSDecl: function makeRawShader(tmx: Enum<TileMode>, tmy: Enum<TileMode>,
    //!                                sampling: Enum<Sampling>, local_matrix: CkMat3x3 | null): CkShader | null
    v8::Local<v8::Value> makeRawShader(int32_t tmx, int32_t tmy, int32_t sampling,
                                       v8::Local<v8::Value> local_matrix);

private:
    void CheckDisposedOrThrow() const;

    sk_sp<SkImage> image_;
};

GALLIUM_BINDINGS_GLAMOR_NS_END
#endif //COCOA_GALLIUM_BINDINGS_GLAMOR_CKIMAGEWRAP_H
