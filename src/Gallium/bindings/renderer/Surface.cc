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

#include "Gallium/bindings/renderer/Surface.h"
#include "Gallium/bindings/renderer/ImageAsyncReadResult.h"
#include "Gallium/bindings/renderer/AsyncRescalable.h"
#include "Gallium/bindings/renderer/Image.h"
#include "Gallium/bindings/renderer/Canvas.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

ffi::RetLocal<v8::Value> Surface::MakeNull(int32_t width, int32_t height)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    sk_sp<SkSurface> surface = SkSurfaces::Null(width, height);
    return ffi::JSObject::New<Surface>(
            isolate, nullptr, v8::Local<v8::Uint8Array>(), std::move(surface));
}

struct PixelsReleaseCtx
{
    static void OnRelease(g_maybe_unused void *address, void *context) {
        PixelsReleaseCtx *ctx = static_cast<PixelsReleaseCtx*>(context);
        ctx->store.reset();
        delete ctx;
    }

    static PixelsReleaseCtx *New(std::shared_ptr<v8::BackingStore> bs) {
        return new PixelsReleaseCtx{ .store = std::move(bs) };
    }

    std::shared_ptr<v8::BackingStore> store;
};

ffi::RetLocal<v8::Value> Surface::MakeRaster(ffi::Class<ImageInfo> info,
                                             ffi::Opt<ffi::IFace<SurfaceProps>> maybe_props)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    const SkImageInfo& image_info = info->GetImageInfo();
    size_t byte_size = image_info.computeMinByteSize();

    // Pixel memory should be managed by V8 as it is required by the user's code
    std::shared_ptr<v8::BackingStore> store =
            v8::ArrayBuffer::NewBackingStore(isolate, byte_size);

    // unique_ptr helps us keep the ownership of `ctx` until SkSurface takes over it.
    std::unique_ptr<PixelsReleaseCtx> ctx(PixelsReleaseCtx::New(store));

    SkSurfaceProps props;
    if (maybe_props)
        props = (*maybe_props)->Make();

    sk_sp<SkSurface> surface = SkSurfaces::WrapPixels(
            image_info, store->Data(), image_info.minRowBytes(),
            PixelsReleaseCtx::OnRelease, ctx.get(), &props);
    if (!surface)
        return ffi::Fail(ffi::kErr, "failed to create raster surface");

    v8::Local<v8::ArrayBuffer> ab = v8::ArrayBuffer::New(isolate, store);

    // SkSurface has taken the ownership
    (void) ctx.release();
    return ffi::JSObject::New<Surface>(
        isolate,
        std::move(store),
        v8::Uint8Array::New(ab, 0, byte_size),
        std::move(surface)
    );
}

ffi::RetLocal<v8::Value> Surface::MakeFromPixmap(ffi::IFace<Pixmap> pixmap,
                                                 ffi::Opt<ffi::IFace<SurfaceProps>> maybe_props)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    std::shared_ptr<v8::BackingStore> store = pixmap->pixels.BackingStore();
    std::unique_ptr<PixelsReleaseCtx> ctx(PixelsReleaseCtx::New(store));

    SkSurfaceProps props;
    if (maybe_props)
        props = (*maybe_props)->Make();

    sk_sp<SkSurface> surface = SkSurfaces::WrapPixels(
            pixmap->image_info->GetImageInfo(), pixmap->pixels.Address(),
            pixmap->row_bytes, &props);
    if (!surface)
        return ffi::Fail(ffi::kErr, "failed to create raster surface from given Pixmap");

    (void) ctx.release();
    return ffi::JSObject::New<Surface>(
        isolate,
        std::move(store),
        pixmap->pixels.TypedArray(),
        std::move(surface)
    );
}

Surface::Surface(std::shared_ptr<v8::BackingStore> store,
                 v8::Local<v8::Uint8Array> store_u8array,
                 sk_sp<SkSurface> surface)
    : store_(std::move(store))
    , surface_(std::move(surface))
{
    if (!store_u8array.IsEmpty())
        store_u8array_.Reset(v8::Isolate::GetCurrent(), store_u8array);
}

ffi::Ret<void> Surface::dispose()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!canvas_cache_.IsEmpty())
    {
        Canvas *canvas = ffi::JSObject::Unwrap<Canvas>(isolate, canvas_cache_.Get(isolate));
        // Possible JavaScript execution:
        canvas->OnParentDispose();
        canvas_cache_.Reset();
    }

    surface_.reset();
    store_.reset();
    store_u8array_.Reset();
    image_info_cache_.Reset();
    NotifyDisposeState(DisposeState::kDisposed);
    return {};
}

ffi::RetLocal<v8::Object> Surface::getImageInfo()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!image_info_cache_.IsEmpty())
        return image_info_cache_.Get(isolate);

    auto object = ffi::JSObject::New<ImageInfo>(isolate, surface_->imageInfo());
    image_info_cache_.Reset(isolate, object);
    return object;
}

ffi::RetLocal<v8::Value> Surface::getCanvas()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!canvas_cache_.IsEmpty())
        return canvas_cache_.Get(isolate);

    auto canvas = ffi::JSObject::New<Canvas>(
            isolate, surface_->getCanvas(), GetThisHandle(isolate));
    canvas_cache_.Reset(isolate, canvas);
    return canvas;
}

ffi::Ret<void> Surface::notifyContentWillChange(bool discard)
{
    surface_->notifyContentWillChange(
            discard ? SkSurface::kDiscard_ContentChangeMode
                    : SkSurface::kRetain_ContentChangeMode);
    return {};
}

ffi::RetLocal<v8::Value> Surface::makeImageSnapshot(ffi::Opt<RectAdapter> bounds)
{
    sk_sp<SkImage> image;
    if (bounds)
        image = surface_->makeImageSnapshot((**bounds).round());
    else
        image = surface_->makeImageSnapshot();

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!image)
        return v8::Null(isolate);
    return ffi::JSObject::New<Image>(isolate, std::move(image));
}

ffi::RetLocal<v8::Value> Surface::peekPixels()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!store_ || store_u8array_.IsEmpty())
        return v8::Null(isolate);

    SkPixmap pixmap;
    if (!surface_->peekPixels(&pixmap))
        return v8::Null(isolate);

    std::unordered_map<std::string, v8::Local<v8::Value>> result{
        { "pixels", store_u8array_.Get(isolate) },
        { "rowBytes", ffi::Cast<size_t>::ToChecked(isolate, pixmap.rowBytes()) },
        { "imageInfo", getImageInfo().Extract() }
    };
    return ffi::Cast<decltype(result)>::ToChecked(isolate, result);
}

ffi::Ret<bool> Surface::readPixels(PixmapAdapter dst, int32_t src_x, int32_t src_y)
{
    return surface_->readPixels(*dst, src_x, src_y);
}

ffi::Ret<void> Surface::writePixels(PixmapAdapter src, int32_t src_x, int32_t src_y)
{
    surface_->writePixels(*src, src_x, src_y);
    return {};
}

ffi::RetLocal<v8::Value>
Surface::asyncRescaleAndReadPixels(ffi::Class<ImageInfo> image_info,
                                   RectAdapter src_rect_adapter,
                                   ffi::Enum<SkImage::RescaleGamma> rescale_gamma,
                                   ffi::Enum<SkImage::RescaleMode> rescale_mode)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    const SkImageInfo& dst_image_info = image_info->GetImageInfo();
    SkIRect src_rect = (*src_rect_adapter).round();

    auto [async_ctx, promise] = AsyncRescaleContext::Create(isolate, 1);
    async_ctx->planes[0] = { .size = dst_image_info.dimensions(),
                             .bytes_per_pixel = dst_image_info.bytesPerPixel() };

    surface_->asyncRescaleAndReadPixels(dst_image_info,
                                        src_rect,
                                        *rescale_gamma,
                                        *rescale_mode,
                                        AsyncRescaleContext::Callback,
                                        async_ctx);
    return promise;
}

ffi::RetLocal<v8::Value>
Surface::asyncRescaleAndReadPixelsYUV420(ffi::Enum<SkYUVColorSpace> yuv_color_space,
                                         ffi::Class<ColorSpace> dst_color_space,
                                         RectAdapter src_rect_adapter,
                                         const std::tuple<int32_t, int32_t>& dst_size_tuple,
                                         ffi::Enum<SkImage::RescaleGamma> rescale_gamma,
                                         ffi::Enum<SkImage::RescaleMode> rescale_mode)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    SkIRect src_rect = (*src_rect_adapter).round();
    SkISize dst_size = SkISize::Make(std::get<0>(dst_size_tuple), std::get<1>(dst_size_tuple));
    SkISize dst_uv_size = SkISize::Make(dst_size.width() >> 1, dst_size.height() >> 1);

    auto [async_ctx, promise] = AsyncRescaleContext::Create(isolate, 3);
    async_ctx->planes[0] = { .size = dst_size, .bytes_per_pixel = 1 };      // Y plane
    async_ctx->planes[1] = { .size = dst_uv_size, .bytes_per_pixel = 1 };   // U plane
    async_ctx->planes[2] = { .size = dst_uv_size, .bytes_per_pixel = 1 };   // V plane

    surface_->asyncRescaleAndReadPixelsYUV420(*yuv_color_space,
                                              (*dst_color_space)->GetColorSpace(),
                                              src_rect,
                                              dst_size,
                                              *rescale_gamma,
                                              *rescale_mode,
                                              AsyncRescaleContext::Callback,
                                              async_ctx);
    return promise;
}

ffi::RetLocal<v8::Value>
Surface::asyncRescaleAndReadPixelsYUVA420(ffi::Enum<SkYUVColorSpace> yuv_color_space,
                                          ffi::Class<ColorSpace> dst_color_space,
                                          RectAdapter src_rect_adapter,
                                          const std::tuple<int32_t, int32_t>& dst_size_tuple,
                                          ffi::Enum<SkImage::RescaleGamma> rescale_gamma,
                                          ffi::Enum<SkImage::RescaleMode> rescale_mode)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    SkIRect src_rect = (*src_rect_adapter).round();
    SkISize dst_size = SkISize::Make(std::get<0>(dst_size_tuple), std::get<1>(dst_size_tuple));
    SkISize dst_uv_size = SkISize::Make(dst_size.width() >> 1, dst_size.height() >> 1);

    auto [async_ctx, promise] = AsyncRescaleContext::Create(isolate, 4);
    async_ctx->planes[0] = { .size = dst_size, .bytes_per_pixel = 1 };      // Y plane
    async_ctx->planes[1] = { .size = dst_uv_size, .bytes_per_pixel = 1 };   // U plane
    async_ctx->planes[2] = { .size = dst_uv_size, .bytes_per_pixel = 1 };   // V plane
    async_ctx->planes[3] = { .size = dst_size, .bytes_per_pixel = 1 };      // A plane

    surface_->asyncRescaleAndReadPixelsYUVA420(*yuv_color_space,
                                               (*dst_color_space)->GetColorSpace(),
                                               src_rect,
                                               dst_size,
                                               *rescale_gamma,
                                               *rescale_mode,
                                               AsyncRescaleContext::Callback,
                                               async_ctx);
    return promise;
}

GALLIUM_BINDINGS_RENDERER_NS_END
