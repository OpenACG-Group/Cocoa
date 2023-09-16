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

#ifndef COCOA_GALLIUM_BINDINGS_GLAMOR_GPUDIRECTCONTEXT_H
#define COCOA_GALLIUM_BINDINGS_GLAMOR_GPUDIRECTCONTEXT_H

#include <memory>

#include "include/gpu/GrBackendSemaphore.h"

#include "include/v8.h"

#include "Gallium/bindings/glamor/Types.h"
#include "Gallium/bindings/ExportableObjectBase.h"
#include "Glamor/HWComposeOffscreen.h"

GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

class GpuDirectContext;

//! TSDecl: class GpuBinarySemaphore
class GpuBinarySemaphore : public ExportableObjectBase
{
public:
    explicit GpuBinarySemaphore(v8::Local<v8::Object> gpu_object,
                                VkSemaphore semaphore)
        : context_object_(v8::Isolate::GetCurrent(), gpu_object)
        , semaphore_(semaphore) {}
    ~GpuBinarySemaphore() = default;

    g_nodiscard VkSemaphore GetVkSemaphore() {
        CheckDisposedOrThrow();
        return semaphore_;
    }

    g_nodiscard GpuDirectContext *GetGpuContext();

    //! TSDecl: function dispose(): void
    void dispose();

    //! TSDecl: function detach(): void
    void detach();

    //! TSDecl: function isDetachedOrDisposed(): boolean
    bool isDetachedOrDisposed();

private:
    void CheckDisposedOrThrow();

    v8::Global<v8::Object>  context_object_;
    VkSemaphore             semaphore_;
};

//! TSDecl: class GpuExportedFd
class GpuExportedFd : public ExportableObjectBase
{
public:
    GpuExportedFd(v8::Local<v8::Object> gpu_context, int32_t fd);
    ~GpuExportedFd();

    int32_t CheckAndTakeDescriptor();

    //! TSDecl: function close(): void
    void close();

    //! TSDecl: function isImportedOrClosed(): boolean
    bool isImportedOrClosed() const;

private:
    v8::Global<v8::Object>  gpu_context_;
    int32_t                 fd_;
};

//! TSDecl: class GpuDirectContext
class GpuDirectContext : public ExportableObjectBase
{
public:
    //! TSDecl: interface GpuFlushInfo {
    //!   signalSemaphores?: Array<GpuBinarySemaphore>;
    //!   onFinished?: () => void;
    //!   onSubmitted?: (success: boolean) => void;
    //! }
    static std::tuple<GrFlushInfo, std::unique_ptr<GrBackendSemaphore[]>>
    ExtractGrFlushInfo(v8::Isolate *isolate, v8::Local<v8::Object> object,
                       v8::Local<v8::Object> direct_context);

    explicit GpuDirectContext(std::unique_ptr<gl::HWComposeOffscreen> context);
    ~GpuDirectContext() = default;

    g_nodiscard gl::HWComposeOffscreen *GetHWComposeOffscreen() {
        CheckDisposedOrThrow();
        return context_.get();
    }

    //! TSDecl: function Make(): GpuDirectContext
    static v8::Local<v8::Value> Make();

    //! TSDecl: function dispose(): void
    void dispose();

    //! TSDecl: function isDisposed(): boolean
    bool isDisposed();

    //! TSDecl: function isOutOfHostOrDeviceMemory(): boolean
    bool isOutOfHostOrDeviceMemory();

    //! TSDecl: function getResourceCacheLimit(): number
    size_t getResourceCacheLimit();

    //! TSDecl: function getResourceCacheUsage(): {count: number, totalBytes: number}
    v8::Local<v8::Value> getResourceCacheUsage();

    //! TSDecl: function getResourceCachePurgeableBytes(): number
    size_t getResourceCachePurgeableBytes();

    //! TSDecl: function setResourceCacheLimit(bytes: number): void
    void setResourceCacheLimit(size_t bytes);

    //! TSDecl: function freeGpuResources(): void
    void freeGpuResources();

    //! TSDecl: function performDeferredCleanup(msNotUsed: number, scratchOnly: boolean): void
    void performDeferredCleanup(double ms_not_used, bool scratch_only);

    //! TSDecl: function supportsDistanceFieldText() boolean
    bool supportsDistanceFieldText();

    //! TSDecl: function makeSurface(imageInfo: CkImageInfo,
    //!                              budgeted: boolean,
    //!                              aaSamplesPerPixel: number): CkSurface
    v8::Local<v8::Value> makeSurface(v8::Local<v8::Value> image_info,
                                     bool budgeted,
                                     int32_t aa_samples_per_pixel);

    //! TSDecl: function exportSemaphoreFd(semaphore: GpuBinarySemaphore): GpuExportedFd
    v8::Local<v8::Value> exportSemaphoreFd(v8::Local<v8::Value> semaphore);

    //! TSDecl: function importSemaphoreFd(fd: GpuExportedFd): GpuBinarySemaphore
    v8::Local<v8::Value> importSemaphoreFd(v8::Local<v8::Value> fd);

    //! TSDecl: function makeBinarySemaphore(): GpuBinarySemaphore
    v8::Local<v8::Value> makeBinarySemaphore();

    //! TSDecl: function flush(info: GpuFlushInfo): Enum<GpuSemaphoreSubmitted>
    int32_t flush(v8::Local<v8::Value> info);

    //! TSDecl: function submit(waitForSubmit: boolean): boolean
    bool submit(bool wait_for_submit);

private:
    void CheckDisposedOrThrow();

    std::unique_ptr<gl::HWComposeOffscreen> context_;
    std::list<int32_t> dangling_fds_;
};

GALLIUM_BINDINGS_GLAMOR_NS_END
#endif //COCOA_GALLIUM_BINDINGS_GLAMOR_GPUDIRECTCONTEXT_H
