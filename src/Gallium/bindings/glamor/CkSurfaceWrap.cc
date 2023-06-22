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

v8::Local<v8::Value> CkSurface::MakeSharedPixels(v8::Local<v8::Value> image_info,
                                                 uint64_t byte_offset,
                                                 uint64_t row_bytes,
                                                 v8::Local<v8::Value> pixels)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!binder::IsSome<v8::ArrayBuffer>(pixels))
        g_throw(TypeError, "Argument `pixels` must be an ArrayBuffer");

    v8::Local<v8::ArrayBuffer> pixels_ab = v8::Local<v8::ArrayBuffer>::Cast(pixels);
    SkImageInfo info = ExtractCkImageInfo(isolate, image_info);

    if (info.computeByteSize(row_bytes) > pixels_ab->ByteLength() - byte_offset)
        g_throw(TypeError, "Invalid size of provided pixels buffer");

    auto *ptr = reinterpret_cast<uint8_t*>(pixels_ab->Data()) + byte_offset;
    sk_sp<SkSurface> surface = SkSurfaces::WrapPixels(info, ptr, row_bytes);
    if (!surface)
        g_throw(Error, "Failed to create a surface");

    return binder::NewObject<CkSurface>(isolate, surface, pixels_ab);
}

CkSurface::CkSurface(const sk_sp<SkSurface>& surface, ssize_t increase_gc)
    : SkiaObjectWrapper(surface)
    , increase_gc_(increase_gc)
{
    if (increase_gc_)
    {
        v8::Isolate *isolate = v8::Isolate::GetCurrent();
        isolate->AdjustAmountOfExternalAllocatedMemory(increase_gc_);
    }
}

CkSurface::CkSurface(const sk_sp<SkSurface>& surface, v8::Local<v8::ArrayBuffer> ab)
    : SkiaObjectWrapper(surface)
    , increase_gc_(0)
    , shared_pixels_(ab->GetIsolate(), ab)
{
}

CkSurface::~CkSurface()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (isolate && increase_gc_ > 0)
    {
        isolate->AdjustAmountOfExternalAllocatedMemory(-increase_gc_);
    }
}

int32_t CkSurface::getWidth()
{
    return GetSkObject()->width();
}

int32_t CkSurface::getHeight()
{
    return GetSkObject()->height();
}

v8::Local<v8::Value> CkSurface::getImageInfo()
{
    return NewCkImageInfo(v8::Isolate::GetCurrent(),
                          GetSkObject()->imageInfo());
}

uint32_t CkSurface::getGenerationID()
{
    return GetSkObject()->generationID();
}

v8::Local<v8::Value> CkSurface::getSharedPixels()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (shared_pixels_.IsEmpty())
        return v8::Null(isolate);
    return shared_pixels_.Get(isolate);
}

v8::Local<v8::Value> CkSurface::getCanvas()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (canvas_obj_.IsEmpty())
    {
        CHECK(GetSkObject()->getCanvas());
        canvas_obj_.Reset(isolate, binder::NewObject<CkCanvas>(
                isolate, GetSkObject()->getCanvas()));
    }

    return canvas_obj_.Get(isolate);
}

v8::Local<v8::Value> CkSurface::makeSurface(int32_t width, int32_t height)
{
    return binder::NewObject<CkSurface>(v8::Isolate::GetCurrent(),
                                                   GetSkObject()->makeSurface(width, height),
                                                   increase_gc_);
}

v8::Local<v8::Value> CkSurface::makeImageSnapshot(v8::Local<v8::Value> bounds)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    SkRect rect = SkRect::MakeEmpty();
    if (!bounds->IsNullOrUndefined())
        rect = ExtractCkRect(isolate, bounds);

    sk_sp<SkImage> image;
    if (rect.isEmpty())
        image = GetSkObject()->makeImageSnapshot();
    else
        image = GetSkObject()->makeImageSnapshot(rect.round());

    if (!image)
        return v8::Null(isolate);

    return binder::NewObject<CkImageWrap>(isolate, image);
}

void CkSurface::draw(v8::Local<v8::Value> canvas, SkScalar x, SkScalar y,
                     int32_t sampling, v8::Local<v8::Value> paint)
{
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

    GetSkObject()->draw(cw->GetCanvas(), x, y, SamplingToSamplingOptions(sampling),
                        maybe_paint);
}

void CkSurface::readPixels(v8::Local<v8::Value> dstInfo,
                           v8::Local<v8::Value> dstPixels,
                           int32_t dstRowBytes,
                           int32_t srcX,
                           int32_t srcY)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    SkImageInfo info = ExtractCkImageInfo(isolate, dstInfo);

    std::optional<binder::TypedArrayMemory<v8::Uint8Array>> memory =
            binder::GetTypedArrayMemory<v8::Uint8Array>(dstPixels);
    if (!memory)
        g_throw(TypeError, "Argument `dstPixels` must be an allocated `Uint8Array`");

    if (memory->byte_size < info.computeByteSize(dstRowBytes))
        g_throw(Error, "`dstPixels` is not big enough to write pixels");

    bool res = GetSkObject()->readPixels(info, memory->ptr, dstRowBytes, srcX, srcY);
    if (!res)
        g_throw(Error, "Failed to read pixels from surface");
}

void CkSurface::notifyContentWillChange(int32_t mode)
{
    if (mode < 0 || mode > SkSurface::kRetain_ContentChangeMode)
        g_throw(RangeError, "Invalid enumeration value for argument `mode`");
    GetSkObject()->notifyContentWillChange(static_cast<SkSurface::ContentChangeMode>(mode));
}

GALLIUM_BINDINGS_GLAMOR_NS_END
