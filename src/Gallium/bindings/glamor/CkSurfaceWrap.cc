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

#include "Gallium/binder/ThrowExcept.h"
#include "Gallium/binder/Class.h"
#include "Gallium/binder/TypeTraits.h"
#include "Gallium/bindings/glamor/Exports.h"
#include "Gallium/bindings/glamor/CkSurfaceWrap.h"
#include "Gallium/bindings/glamor/CkCanvasWrap.h"
#include "Gallium/bindings/glamor/CkPaintWrap.h"
#include "Gallium/bindings/glamor/CkPixmapWrap.h"
#include "Gallium/bindings/glamor/CkImageWrap.h"
#include "Gallium/bindings/glamor/GpuDirectContext.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

v8::Local<v8::Value> CkSurface::MakeRaster(v8::Local<v8::Value> image_info)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    SkImageInfo info = ExtractCkImageInfo(isolate, image_info);
    sk_sp<SkSurface> surface = SkSurfaces::Raster(info);
    if (!surface)
        g_throw(Error, "Failed to create a surface");

    auto pixels_size = static_cast<ssize_t>(info.computeMinByteSize());
    return binder::NewObject<CkSurface>(isolate, surface, pixels_size);
}

v8::Local<v8::Value> CkSurface::MakeNull(int32_t width, int32_t height)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    sk_sp<SkSurface> surface = SkSurfaces::Null(width, height);
    if (!surface)
        g_throw(Error, "Failed to create a surface");
    return binder::NewObject<CkSurface>(isolate, surface, 0);
}

v8::Local<v8::Value> CkSurface::WrapPixels(v8::Local<v8::Value> image_info,
                                           uint64_t row_bytes,
                                           v8::Local<v8::Value> pixels)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    auto pixels_memory = binder::GetTypedArrayMemory<v8::TypedArray>(pixels);
    if (!pixels_memory)
        g_throw(TypeError, "Argument `pixels` must be a valid TypedArray");

    SkImageInfo info = ExtractCkImageInfo(isolate, image_info);

    if (info.computeByteSize(row_bytes) > pixels_memory->byte_size)
        g_throw(TypeError, "Invalid size of provided pixels buffer");

    sk_sp<SkSurface> surface = SkSurfaces::WrapPixels(
            info, pixels_memory->ptr, row_bytes);
    if (!surface)
        g_throw(Error, "Failed to create a surface from wrapping pixels");

    return binder::NewObject<CkSurface>(isolate, surface, WrappedPixels{
        .backing_store = pixels_memory->memory,
        .offset = static_cast<int64_t>(pixels_memory->byte_offset),
        .byte_length = info.computeByteSize(row_bytes),
        .ptr = pixels_memory->ptr
    });
}

CkSurface::CkSurface(sk_sp<SkSurface> surface, ssize_t increase_gc)
    : surface_(std::move(surface))
    , increase_gc_(increase_gc)
{
    if (increase_gc_)
    {
        v8::Isolate *isolate = v8::Isolate::GetCurrent();
        isolate->AdjustAmountOfExternalAllocatedMemory(increase_gc_);
    }
}

CkSurface::CkSurface(sk_sp<SkSurface> surface, WrappedPixels wrapped_pixels)
    : surface_(std::move(surface))
    , increase_gc_(0)
    , wrapped_pixels_(std::move(wrapped_pixels))
{
}

CkSurface::CkSurface(sk_sp<SkSurface> surface, v8::Local<v8::Object> gpu_direct_context)
    : surface_(std::move(surface))
    , increase_gc_(0)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    gpu_direct_context_.Reset(isolate, gpu_direct_context);
}

CkSurface::~CkSurface()
{
    if (surface_)
        dispose();
}

void CkSurface::dispose()
{
    if (!surface_)
        g_throw(Error, "Surface has been disposed");

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (isolate && increase_gc_ > 0)
    {
        isolate->AdjustAmountOfExternalAllocatedMemory(-increase_gc_);
    }

    surface_.reset();
    increase_gc_ = 0;
    wrapped_pixels_.Reset();
}

bool CkSurface::isDisposed()
{
    return !surface_;
}

void CkSurface::CheckDisposedOrThrow()
{
    if (surface_)
        return;
    g_throw(Error, "Surface has been disposed");
}

int32_t CkSurface::getWidth()
{
    CheckDisposedOrThrow();
    return surface_->width();
}

int32_t CkSurface::getHeight()
{
    CheckDisposedOrThrow();
    return surface_->height();
}

v8::Local<v8::Value> CkSurface::getImageInfo()
{
    CheckDisposedOrThrow();
    return NewCkImageInfo(v8::Isolate::GetCurrent(),
                          surface_->imageInfo());
}

uint32_t CkSurface::getGenerationID()
{
    CheckDisposedOrThrow();
    return surface_->generationID();
}

v8::Local<v8::Value> CkSurface::getCanvas()
{
    CheckDisposedOrThrow();
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (canvas_obj_.IsEmpty())
    {
        CHECK(surface_->getCanvas());
        canvas_obj_.Reset(isolate, binder::NewObject<CkCanvas>(
                isolate, surface_->getCanvas()));
    }

    return canvas_obj_.Get(isolate);
}

v8::Local<v8::Value> CkSurface::getGpuDirectContext()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (gpu_direct_context_.IsEmpty())
        return v8::Null(isolate);
    return gpu_direct_context_.Get(isolate);
}

v8::Local<v8::Value> CkSurface::makeSurface(int32_t width, int32_t height)
{
    CheckDisposedOrThrow();
    sk_sp<SkSurface> derived = surface_->makeSurface(width, height);
    if (!derived)
        g_throw(Error, "Failed to make derived surface");

    return binder::NewObject<CkSurface>(
            v8::Isolate::GetCurrent(), std::move(derived), increase_gc_);
}

v8::Local<v8::Value> CkSurface::makeImageSnapshot(v8::Local<v8::Value> bounds)
{
    CheckDisposedOrThrow();
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    SkRect rect = SkRect::MakeEmpty();
    if (!bounds->IsNullOrUndefined())
        rect = ExtractCkRect(isolate, bounds);

    sk_sp<SkImage> image;
    if (rect.isEmpty())
        image = surface_->makeImageSnapshot();
    else
        image = surface_->makeImageSnapshot(rect.round());

    if (!image)
        return v8::Null(isolate);

    return binder::NewObject<CkImageWrap>(isolate, image);
}

void CkSurface::draw(v8::Local<v8::Value> canvas, SkScalar x, SkScalar y,
                     int32_t sampling, v8::Local<v8::Value> paint)
{
    CheckDisposedOrThrow();
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto *cw = binder::UnwrapObject<CkCanvas>(isolate, canvas);
    if (!cw)
        g_throw(TypeError, "Argument `canvas` must be an instance of `CkCanvas`");

    SkPaint *maybe_paint = nullptr;
    if (!paint->IsNullOrUndefined())
    {
        auto *pw = binder::UnwrapObject<CkPaint>(isolate, paint);
        if (!pw)
            g_throw(TypeError, "Argument `paint` must be an instance of `CkPaint`");
        maybe_paint = &pw->GetPaint();
    }

    surface_->draw(cw->GetCanvas(), x, y,
                   SamplingToSamplingOptions(sampling), maybe_paint);
}

v8::Local<v8::Value> CkSurface::peekPixels(v8::Local<v8::Value> scope_callback)
{
    CheckDisposedOrThrow();
    if (!scope_callback->IsFunction())
        g_throw(TypeError, "Argument `scopeCallback` must be a Function");
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    SkPixmap pixmap;
    if (!surface_->peekPixels(&pixmap))
        g_throw(Error, "Address of pixel buffer is not accessible");
    v8::Local<v8::Value> wrapped_pixmap =
            binder::NewObject<CkPixmap>(isolate, std::move(pixmap));

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::MaybeLocal<v8::Value> maybe_ret =
            scope_callback.As<v8::Function>()->Call(
                    context, v8::Undefined(isolate), 1, &wrapped_pixmap);
    binder::UnwrapObjectFast<CkPixmap>(isolate, wrapped_pixmap)->resetEmpty();
    v8::Local<v8::Value> ret;
    if (!maybe_ret.ToLocal(&ret))
        return v8::Undefined(isolate);
    return ret;
}

void CkSurface::readPixels(v8::Local<v8::Value> dstInfo,
                           v8::Local<v8::Value> dstPixels,
                           int32_t dstRowBytes,
                           int32_t srcX,
                           int32_t srcY)
{
    CheckDisposedOrThrow();
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    SkImageInfo info = ExtractCkImageInfo(isolate, dstInfo);

    std::optional<binder::TypedArrayMemory<v8::Uint8Array>> memory =
            binder::GetTypedArrayMemory<v8::Uint8Array>(dstPixels);
    if (!memory)
        g_throw(TypeError, "Argument `dstPixels` must be an allocated `Uint8Array`");

    if (memory->byte_size < info.computeByteSize(dstRowBytes))
        g_throw(Error, "`dstPixels` is not big enough to write pixels");

    bool res = surface_->readPixels(info, memory->ptr, dstRowBytes, srcX, srcY);
    if (!res)
        g_throw(Error, "Failed to read pixels from surface");
}

void CkSurface::readPixelsToPixmap(v8::Local<v8::Value> pixmap,
                                   int32_t src_x, int32_t src_y)
{
    CheckDisposedOrThrow();
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    CkPixmap *wrapped_pixmap = binder::UnwrapObject<CkPixmap>(isolate, pixmap);
    if (!wrapped_pixmap || !wrapped_pixmap->GetInnerPixmap().addr())
        g_throw(TypeError, "Argument `pixmap` must be a non-empty CkPixmap");
    if (!surface_->readPixels(wrapped_pixmap->GetInnerPixmap(), src_x, src_y))
        g_throw(Error, "Failed to read pixels to pixmap");
}

void CkSurface::writePixels(v8::Local<v8::Value> pixmap,
                            int32_t dst_x, int32_t dst_y)
{
    CheckDisposedOrThrow();
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    CkPixmap *wrapped_pixmap = binder::UnwrapObject<CkPixmap>(isolate, pixmap);
    if (!wrapped_pixmap || !wrapped_pixmap->GetInnerPixmap().addr())
        g_throw(TypeError, "Argument `pixmap` must be a non-empty pixmap");
    surface_->writePixels(wrapped_pixmap->GetInnerPixmap(), dst_x, dst_y);
}

void CkSurface::notifyContentWillChange(int32_t mode)
{
    CheckDisposedOrThrow();
    if (mode < 0 || mode > SkSurface::kRetain_ContentChangeMode)
        g_throw(RangeError, "Invalid enumeration value for argument `mode`");
    surface_->notifyContentWillChange(static_cast<SkSurface::ContentChangeMode>(mode));
}

bool CkSurface::waitOnGpu(v8::Local<v8::Value> wait_semaphores,
                          bool take_semaphores_ownership)
{
    constexpr const char *kInvalidArray =
            "Argument `waitSemaphores` must be an array of GpuBinarySemaphore";

    CheckDisposedOrThrow();
    if (gpu_direct_context_.IsEmpty())
        return false;

    if (!wait_semaphores->IsArray())
        g_throw(TypeError, kInvalidArray);

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    auto array = wait_semaphores.As<v8::Array>();

    int32_t num_semaphores = static_cast<int32_t>(array->Length());
    std::vector<GrBackendSemaphore> vk_semaphores(num_semaphores);
    std::vector<GpuBinarySemaphore*> wraps(num_semaphores);
    for (int32_t i = 0; i < num_semaphores; i++)
    {
        v8::Local<v8::Value> element;
        if (!array->Get(context, i).ToLocal(&element))
            g_throw(TypeError, kInvalidArray);
        GpuBinarySemaphore *wrap =
                binder::UnwrapObject<GpuBinarySemaphore>(isolate, element);
        if (!wrap)
            g_throw(TypeError, kInvalidArray);
        wraps[i] = wrap;
        vk_semaphores[i].initVulkan(wrap->GetVkSemaphore());
    }

    bool ret = surface_->wait(num_semaphores,
                              vk_semaphores.data(),
                              take_semaphores_ownership);
    if (!ret)
        return false;

    // If `take_semaphores_ownership` is true, `SkSurface` will delete
    // the semaphores automatically, which is equivalent to that `SkSurface`
    // has taken over the ownership of semaphores.
    if (take_semaphores_ownership)
    {
        for (GpuBinarySemaphore *wrap : wraps)
            wrap->detach();
    }
    return true;
}

int32_t CkSurface::flush(v8::Local<v8::Value> info)
{
    CheckDisposedOrThrow();
    if (gpu_direct_context_.IsEmpty())
        return static_cast<int32_t>(GrSemaphoresSubmitted::kNo);

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!info->IsObject())
        g_throw(TypeError, "Argument `info` must be an object");
    auto [flush_info, owned_semaphores] = GpuDirectContext::ExtractGrFlushInfo(
            isolate, info.As<v8::Object>(), gpu_direct_context_.Get(isolate));
    GpuDirectContext *direct_context = binder::UnwrapObjectFast<GpuDirectContext>(
            isolate, gpu_direct_context_.Get(isolate));
    CHECK(direct_context);
    GrDirectContext *gr_context = direct_context->GetHWComposeOffscreen()->GetSkiaGpuContext();
    return static_cast<int32_t>(gr_context->flush(surface_, flush_info));
}

GALLIUM_BINDINGS_GLAMOR_NS_END
