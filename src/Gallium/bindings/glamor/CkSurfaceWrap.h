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

#ifndef COCOA_GALLIUM_BINDINGS_GLAMOR_CKSURFACEWRAP_H
#define COCOA_GALLIUM_BINDINGS_GLAMOR_CKSURFACEWRAP_H

#include "include/v8.h"
#include "include/core/SkSurface.h"

#include "Gallium/bindings/glamor/Types.h"
#include "Gallium/bindings/glamor/TrivialInterface.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

//! TSDecl: class CkSurface
class CkSurface : public SkiaObjectWrapper<SkSurface>
{
public:
    CkSurface(const sk_sp<SkSurface>& surface, ssize_t increase_gc);
    ~CkSurface();

    //! TSDecl: function MakeRaster(imageInfo: CkImageInfo): CkSurface
    static v8::Local<v8::Value> MakeRaster(v8::Local<v8::Value> image_info);

    //! TSDecl: function MakeNull(width: number, height: number): CkSurface
    static v8::Local<v8::Value> MakeNull(int32_t width, int32_t height);

    //! TSDecl: readonly width: number
    int32_t getWidth();

    //! TSDecl: readonly height: number
    int32_t  getHeight();

    //! TSDecl: readonly generationID: number
    uint32_t getGenerationID();

    //! TSDecl: readonly imageInfo: CkImageInfo
    v8::Local<v8::Value> getImageInfo();

    //! TSDecl: function getCanvas(): CkCanvas
    v8::Local<v8::Value> getCanvas();

    //! TSDecl: function makeSurface(width: number, height: number): CkSurface
    v8::Local<v8::Value> makeSurface(int32_t width, int32_t height);

    //! TSDecl: function makeImageSnapshot(bounds: CkRect | null): CkImage
    v8::Local<v8::Value> makeImageSnapshot(v8::Local<v8::Value> bounds);

    //! TSDecl: function draw(canvas: CkCanvas, x: number, y: number,
    //!                       sampling: Sampling, paint: CkPaint | null): void
    void draw(v8::Local<v8::Value> canvas, SkScalar x, SkScalar y, int32_t sampling,
              v8::Local<v8::Value> paint);

    //! TSDecl: function readPixels(dstInfo: CkImageInfo, dstPixels: core.Buffer,
    //!                             dstRowBytes: number, srcX: number, srcY: number): void
    void readPixels(v8::Local<v8::Value> dstInfo, v8::Local<v8::Value> dstPixels,
                    int32_t dstRowBytes, int32_t  srcX, int32_t srcY);

    //! TSDecl: function peekPixels(): core.Buffer
    v8::Local<v8::Value> peekPixels();

private:
    ssize_t                     increase_gc_;
    v8::Global<v8::Object>      canvas_obj_;
};

GALLIUM_BINDINGS_GLAMOR_NS_END
#endif //COCOA_GALLIUM_BINDINGS_GLAMOR_CKSURFACEWRAP_H
