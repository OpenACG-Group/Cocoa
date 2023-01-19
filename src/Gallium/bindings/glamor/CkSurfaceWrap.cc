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
#include "Gallium/bindings/glamor/Exports.h"
#include "Gallium/bindings/glamor/CkSurfaceWrap.h"
#include "Gallium/bindings/glamor/CkCanvasWrap.h"
#include "Gallium/bindings/glamor/CkPaintWrap.h"
#include "Gallium/bindings/core/Exports.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

v8::Local<v8::Value> CkSurface::MakeRaster(v8::Local<v8::Value> image_info)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    SkImageInfo info = ExtractCkImageInfo(isolate, image_info);
    sk_sp<SkSurface> surface = SkSurface::MakeRaster(info);
    if (!surface)
        g_throw(Error, "Failed to create a surface");

    auto pixels_size = static_cast<ssize_t>(info.computeMinByteSize());
    return binder::Class<CkSurface>::create_object(isolate, surface, pixels_size);
}

v8::Local<v8::Value> CkSurface::MakeNull(int32_t width, int32_t height)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    sk_sp<SkSurface> surface = SkSurface::MakeNull(width, height);
    if (!surface)
        g_throw(Error, "Failed to create a surface");
    return binder::Class<CkSurface>::create_object(isolate, surface, 0);
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
    return getSkiaObject()->width();
}

int32_t CkSurface::getHeight()
{
    return getSkiaObject()->height();
}

v8::Local<v8::Value> CkSurface::getImageInfo()
{
    return WrapCkImageInfo(v8::Isolate::GetCurrent(),
                           getSkiaObject()->imageInfo());
}

uint32_t CkSurface::getGenerationID()
{
    return getSkiaObject()->generationID();
}

v8::Local<v8::Value> CkSurface::getCanvas()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (canvas_obj_.IsEmpty())
    {
        CHECK(getSkiaObject()->getCanvas());
        canvas_obj_.Reset(isolate, binder::Class<CkCanvas>::create_object(
                isolate, getSkiaObject()->getCanvas()));
    }

    return canvas_obj_.Get(isolate);
}

v8::Local<v8::Value> CkSurface::makeSurface(int32_t width, int32_t height)
{
    return binder::Class<CkSurface>::create_object(v8::Isolate::GetCurrent(),
                                                   getSkiaObject()->makeSurface(width, height),
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
        image = getSkiaObject()->makeImageSnapshot();
    else
        image = getSkiaObject()->makeImageSnapshot(rect.round());

    if (!image)
        return v8::Null(isolate);

    return binder::Class<CkImageWrap>::create_object(isolate, image);
}

void CkSurface::draw(v8::Local<v8::Value> canvas, SkScalar x, SkScalar y,
                     int32_t sampling, v8::Local<v8::Value> paint)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto *cw = binder::Class<CkCanvas>::unwrap_object(isolate, canvas);
    if (!cw)
        g_throw(TypeError, "Argument `canvas` must be an instance of `CkCanvas`");

    SkPaint *maybe_paint = nullptr;
    if (!paint->IsNullOrUndefined())
    {
        auto *pw = binder::Class<CkPaint>::unwrap_object(isolate, paint);
        if (!pw)
            g_throw(TypeError, "Argument `paint` must be an instance of `CkPaint`");
        maybe_paint = &pw->GetPaint();
    }

    getSkiaObject()->draw(cw->GetCanvas(), x, y, SamplingToSamplingOptions(sampling),
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

    auto *buffer = binder::Class<Buffer>::unwrap_object(isolate, dstPixels);
    if (!buffer)
        g_throw(TypeError, "Argument `dstPixels` must be an instance of `core.Buffer`");

    if (buffer->length() < info.computeByteSize(dstRowBytes))
        g_throw(Error, "`dstPixels` is not big enough to write pixels");

    bool res = getSkiaObject()->readPixels(info, buffer->addressU8(),
                                           dstRowBytes, srcX, srcY);
    if (!res)
        g_throw(Error, "Failed to read pixels from surface");
}

v8::Local<v8::Value> CkSurface::peekPixels()
{
    SkPixmap pixmap;
    if (!getSkiaObject()->peekPixels(&pixmap))
        g_throw(Error, "Failed to peek pixels from the surface");

    sk_sp<SkSurface> surface = getSkiaObject();
    return Buffer::MakeFromExternal(
            pixmap.writable_addr(), pixmap.computeByteSize(), [surface] {});
}

GALLIUM_BINDINGS_GLAMOR_NS_END
