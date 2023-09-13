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

#ifndef COCOA_GALLIUM_BINDINGS_GLAMOR_CKPIXMAPWRAP_H
#define COCOA_GALLIUM_BINDINGS_GLAMOR_CKPIXMAPWRAP_H

#include "include/core/SkPixmap.h"
#include "include/v8.h"

#include "Gallium/bindings/ExportableObjectBase.h"
#include "Gallium/bindings/glamor/Types.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

//! TSDecl: class CkPixmap
class CkPixmap : public ExportableObjectBase
{
public:
    //! TSDecl: constructor(imageInfo: CkImageInfo, rowBytes: number, buffer: TypedArray)
    //!         constructor()
    explicit CkPixmap(const v8::FunctionCallbackInfo<v8::Value>& call_info);

    explicit CkPixmap(SkPixmap pixmap);
    ~CkPixmap() = default;

    g_nodiscard SkPixmap& GetInnerPixmap() {
        return pixmap_;
    }

    //! TSDecl: function resetEmpty(): void
    void resetEmpty();

    //! TSDecl: function reset(imageInfo: CkImageInfo,
    //!                        rowBytes: number, buffer: TypedArray): void
    void reset(v8::Local<v8::Value> image_info, int64_t row_bytes, v8::Local<v8::Value> buffer);

    //! TSDecl: function extractSubset(area: CkRect): CkPixmap | null
    v8::Local<v8::Value> extractSubset(v8::Local<v8::Value> area);

    //! TSDecl: readonly info: CkImageInfo
    v8::Local<v8::Value> getInfo();

    //! TSDecl: readonly rowBytes: number
    v8::Local<v8::Value> getRowBytes();

    //! TSDecl: readonly width: number
    int32_t getWidth();

    //! TSDecl: readonly height: number
    int32_t getHeight();

    //! TSDecl: readonly colorType: Enum<ColorType>
    int32_t getColorType();

    //! TSDecl: readonly alphaType: Enum<AlphaType>
    int32_t getAlphaType();

    //! TSDecl: readonly isOpaque: boolean
    bool getIsOpaque();

    //! TSDecl: readonly bounds: CkRect
    v8::Local<v8::Value> getBounds();

    //! TSDecl: readonly rowBytesAsPixels: number
    int32_t getRowBytesAsPixels();

    //! TSDecl: readonly shiftPerPixel: number
    int32_t getShiftPerPixel();

    //! TSDecl: function computeByteSize(): number
    int64_t computeByteSize();

    //! TSDecl: function computeIsOpaque(): boolean
    bool computeIsOpaque();

    //! TSDecl: function getColor4f(x: number, y: number): CkColor4f
    v8::Local<v8::Value> getColor4f(int32_t x, int32_t y);

    //! TSDecl: function getAlphaf(x: number, y: number): number
    float getAlphaf(int32_t x, int32_t y);

    //! TSDecl: function readPixels(dstInfo: CkImageInfo, dstBuffer: TypedArray,
    //!                             dstRowBytes: number, srcX: number, srcY: number): void
    void readPixels(v8::Local<v8::Value> dst_info,
                    v8::Local<v8::Value> dst_buffer,
                    int64_t dst_row_bytes,
                    int32_t src_x,
                    int32_t src_y);

    //! TSDecl: function copy(dst: CkPixmap, srcX: number, srcY: number): void
    void copy(v8::Local<v8::Value> dst, int32_t src_x, int32_t src_y);

    //! TSDecl: function scale(dst: CkPixmap, sampling: Enum<Sampling>): void
    void scale(v8::Local<v8::Value> dst, int32_t sampling);

    //! TSDecl: function erase(color: CkColor4f, subset: CkRect | null): void
    void erase(v8::Local<v8::Value> color, v8::Local<v8::Value> subset);

private:
    void CheckEmptyOrThrow();
    SkPixmap pixmap_;
};

GALLIUM_BINDINGS_GLAMOR_NS_END
#endif //COCOA_GALLIUM_BINDINGS_GLAMOR_CKPIXMAPWRAP_H
