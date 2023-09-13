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

#include "Gallium/bindings/ExportableObjectBase.h"
#include "Gallium/bindings/glamor/Types.h"
#include "Gallium/bindings/glamor/TrivialInterface.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

//! TSDecl: class CkSurface
class CkSurface : public ExportableObjectBase
{
public:
    struct WrappedPixels
    {
        std::shared_ptr<v8::BackingStore> backing_store = nullptr;
        int64_t offset = 0;
        size_t byte_length = 0;
        uint8_t *ptr = nullptr;

        void Reset() {
            backing_store = nullptr;
            offset = 0;
            byte_length = 0;
            ptr = nullptr;
        }
    };

    CkSurface(sk_sp<SkSurface> surface, ssize_t increase_gc);
    CkSurface(sk_sp<SkSurface> surface, WrappedPixels wrapped_pixels);
    CkSurface(sk_sp<SkSurface> surface, v8::Local<v8::Object> gpu_direct_context);
    ~CkSurface();

    //! TSDecl: function MakeRaster(imageInfo: CkImageInfo): CkSurface
    static v8::Local<v8::Value> MakeRaster(v8::Local<v8::Value> image_info);

    //! TSDecl: function MakeNull(width: number, height: number): CkSurface
    static v8::Local<v8::Value> MakeNull(int32_t width, int32_t height);

    //! TSDecl: function WrapPixels(imageInfo: CkImageInfo, rowBytes: number,
    //!                             pixels: TypedArray): CkSurface
    static v8::Local<v8::Value> WrapPixels(v8::Local<v8::Value> image_info,
                                           uint64_t row_bytes,
                                           v8::Local<v8::Value> pixels);

    //! TSDecl: function dispose(): void
    void dispose();

    //! TSDecl: function isDisposed(): boolean
    bool isDisposed();

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

    //! TSDecl: function getGpuDirectContext(): GpuDirectContext | null
    v8::Local<v8::Value> getGpuDirectContext();

    //! TSDecl: function makeSurface(width: number, height: number): CkSurface
    v8::Local<v8::Value> makeSurface(int32_t width, int32_t height);

    //! TSDecl: function makeImageSnapshot(bounds: CkRect | null): CkImage
    v8::Local<v8::Value> makeImageSnapshot(v8::Local<v8::Value> bounds);

    //! TSDecl: function draw(canvas: CkCanvas, x: number, y: number,
    //!                       sampling: Sampling, paint: CkPaint | null): void
    void draw(v8::Local<v8::Value> canvas, SkScalar x, SkScalar y, int32_t sampling,
              v8::Local<v8::Value> paint);

    //! TSDecl: function peekPixels(scopeCallback: (pixmap: CkPixmap) => T): T
    v8::Local<v8::Value> peekPixels(v8::Local<v8::Value> scope_callback);

    //! TSDecl: function readPixels(dstInfo: CkImageInfo, dstPixels: Uint8Array,
    //!                             dstRowBytes: number, srcX: number, srcY: number): void
    void readPixels(v8::Local<v8::Value> dstInfo, v8::Local<v8::Value> dstPixels,
                    int32_t dstRowBytes, int32_t  srcX, int32_t srcY);

    //! TSDecl: function readPixelsToPixmap(pixmap: CkPixmap, srcX: number, srcY: number): void
    void readPixelsToPixmap(v8::Local<v8::Value> pixmap, int32_t src_x, int32_t src_y);

    //! TSDecl: function writePixels(pixmap: CkPixmap, dstX: number, dstY: number): void
    void writePixels(v8::Local<v8::Value> pixmap, int32_t dst_x, int32_t dst_y);

    //! TSDecl: function notifyContentWillChange(mode: Enum<CkSurfaceContentChangeMode>): void
    void notifyContentWillChange(int32_t mode);

    //! TSDecl: function waitOnGpu(waitSemaphores: Array<GpuBinarySemaphore>,
    //!                            takeSemaphoresOwnership: boolean): boolean
    bool waitOnGpu(v8::Local<v8::Value> wait_semaphores, bool take_semaphores_ownership);

    //! TSDecl: function flush(info: GpuFlushInfo): Enum<GpuSemaphoresSubmitted>
    int32_t flush(v8::Local<v8::Value> info);

private:
    void CheckDisposedOrThrow();

    sk_sp<SkSurface>                    surface_;
    ssize_t                             increase_gc_;
    v8::Global<v8::Object>              canvas_obj_;
    WrappedPixels                       wrapped_pixels_;
    v8::Global<v8::Object>              gpu_direct_context_;
};

GALLIUM_BINDINGS_GLAMOR_NS_END
#endif //COCOA_GALLIUM_BINDINGS_GLAMOR_CKSURFACEWRAP_H
