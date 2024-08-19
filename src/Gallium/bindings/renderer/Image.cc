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

#include "fmt/format.h"
#include "include/codec/SkCodec.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/core/SkStream.h"

#include "Core/EventLoop.h"
#include "Core/TraceEvent.h"
#include "Gallium/bindings/renderer/Picture.h"
#include "Gallium/bindings/renderer/Color.h"
#include "Gallium/bindings/renderer/Paint.h"
#include "Gallium/bindings/renderer/AsyncRescalable.h"
#include "Gallium/bindings/renderer/Image.h"
#include "Gallium/bindings/renderer/ImageFilter.h"
#include "Gallium/bindings/renderer/ImageAsyncReadResult.h"
#include "Gallium/bindings/renderer/Shader.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

namespace {

struct SharedMemRetainer
{
    static std::unique_ptr<SharedMemRetainer>
    New(std::shared_ptr<v8::BackingStore> store) {
        // Use `new` instead of `make_unique` since we will use `delete`
        // operator to release the memory.
        auto *ptr = new SharedMemRetainer{std::move(store)};
        return std::unique_ptr<SharedMemRetainer>(ptr);
    }

    static void Release(const void*, void *ctx) {
        CHECK(ctx);
        auto *retainer = static_cast<SharedMemRetainer*>(ctx);
        delete retainer;
    }

    std::shared_ptr<v8::BackingStore> store;
};

} // namespace anonymous

ffi::RetLocal<v8::Value> Image::FromPixmap(ffi::Enum<ImageMemoryMutability> mutability,
                                           PixmapAdapter pixmap_adapter)
{
    sk_sp<SkImage> image;
    std::unique_ptr<SharedMemRetainer> retainer;

    if (*mutability == ImageMemoryMutability::kMutableAndCopy)
        image = SkImages::RasterFromPixmapCopy(*pixmap_adapter);
    else if (*mutability == ImageMemoryMutability::kImmutableAndShare)
    {
        retainer = SharedMemRetainer::New(pixmap_adapter.memory.BackingStore());
        image = SkImages::RasterFromPixmap(
                *pixmap_adapter, SharedMemRetainer::Release, retainer.get());
    }
    else
    {
        MARK_UNREACHABLE();
    }

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!image)
        return ffi::Fail(ffi::kErr, "failed to create image from pixmap");
    if (retainer)
        (void) retainer.release();
    return ffi::JSObject::New<Image>(isolate, std::move(image));
}

ffi::RetLocal<v8::Value>
Image::FromCompressedTextureData(const ffi::Mem<uint8_t>& memory,
                                 int32_t width, int32_t height,
                                 ffi::Enum<SkTextureCompressionType> type)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto retainer = SharedMemRetainer::New(memory.BackingStore());
    sk_sp<SkData> data = SkData::MakeWithProc(memory.Address(), memory.ByteSize(),
                                              SharedMemRetainer::Release, retainer.get());
    CHECK(data);

    sk_sp<SkImage> image = SkImages::RasterFromCompressedTextureData(
            std::move(data), width, height, *type);
    if (!image)
        return ffi::Fail(ffi::kErr, "failed to create image from compressed texture data");
    (void) retainer.release();
    return ffi::JSObject::New<Image>(isolate, std::move(image));
}

ffi::RetLocal<v8::Value>
Image::DeferredFromEncodedData(const ffi::Mem<uint8_t>& encoded,
                               ffi::Opt<ffi::Enum<SkAlphaType>> alpha_type)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto retainer = SharedMemRetainer::New(encoded.BackingStore());
    sk_sp<SkData> data = SkData::MakeWithProc(encoded.Address(), encoded.ByteSize(),
                                              SharedMemRetainer::Release, retainer.get());
    CHECK(data);
    std::optional<SkAlphaType> optional_at;
    if (alpha_type)
        optional_at = **alpha_type;
    sk_sp<SkImage> image = SkImages::DeferredFromEncodedData(data, optional_at);
    if (!image)
        return ffi::Fail(ffi::kErr, "failed to create image from encoded data");
    (void) retainer.release();
    return ffi::JSObject::New<Image>(isolate, std::move(image));
}

ffi::RetLocal<v8::Value>
Image::DeferredFromEncodedFile(const std::string& path,
                               ffi::Opt<ffi::Enum<SkAlphaType>> alpha_type)
{
    sk_sp<SkData> data = SkData::MakeFromFileName(path.c_str());
    if (!data)
        return ffi::Fail(ffi::kErr, fmt::format("failed to read file `{}`", path));
    std::optional<SkAlphaType> optional_at;
    if (alpha_type)
        optional_at = **alpha_type;
    sk_sp<SkImage> image = SkImages::DeferredFromEncodedData(data, optional_at);
    if (!image)
        return ffi::Fail(ffi::kErr, "failed to create image from encoded data");
    return ffi::JSObject::New<Image>(v8::Isolate::GetCurrent(), std::move(image));
}

ffi::RetLocal<v8::Value> Image::DeferredFromPicture(ffi::Class<Picture> picture,
                                                    std::tuple<int32_t, int32_t> dimensions,
                                                    ffi::Opt<Mat3x3Adapter> matrix,
                                                    ffi::Opt<ffi::Class<Paint>> paint,
                                                    bool f16_bit_depth,
                                                    ffi::Class<ColorSpace> color_space)
{
    auto [width, height] = dimensions;
    sk_sp<SkImage> image = SkImages::DeferredFromPicture(
        picture->GetSkPicture(),
        SkISize::Make(width, height),
        matrix ? &matrix->matrix : nullptr,
        paint ? &(*paint)->GetSkPaint() : nullptr,
        f16_bit_depth ? SkImages::BitDepth::kF16 : SkImages::BitDepth::kU8,
        color_space->GetColorSpace()
    );
    if (!image)
        return ffi::Fail(ffi::kErr, "failed to create image from Picture");
    return ffi::JSObject::New<Image>(v8::Isolate::GetCurrent(), image);
}

#define GRCTX_NULL_SELECT(ctx) (ctx ? (*ctx)->GetGrDirectContext().get() : nullptr)

ffi::RetLocal<v8::Value> Image::MakeWithFilter(ffi::Opt<ffi::Class<GpuDirectContext>> context,
                                               ffi::Class<Image> image,
                                               ffi::Class<ImageFilter> filter,
                                               const RectAdapter& subset,
                                               const RectAdapter& clip_bounds)
{
    SkIRect out_subset{};
    SkIPoint offset{};
    sk_sp<SkImage> result_image = context ? SkImages::MakeWithFilter(
        (*context)->GetGrDirectContext().get(),
        image->GetSkImage(),
        filter->GetSkImageFilter().get(),
        (*subset).round(),
        (*clip_bounds).round(),
        &out_subset,
        &offset
    ) : SkImages::MakeWithFilter(
        image->GetSkImage(),
        filter->GetSkImageFilter().get(),
        (*subset).round(),
        (*clip_bounds).round(),
        &out_subset,
        &offset
    );
    if (!result_image)
        return ffi::Fail(ffi::kErr, "failed to create a filtered image");

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Name> keys[] = {
        v8::String::NewFromUtf8Literal(isolate, "outSubset"),
        v8::String::NewFromUtf8Literal(isolate, "offset"),
        v8::String::NewFromUtf8Literal(isolate, "image")
    };
    using Tuple = std::tuple<int32_t, int32_t>;
    v8::Local<v8::Value> values[] = {
        CreateJSRect(isolate, SkRect::Make(out_subset)),
        ffi::Cast<Tuple>::To(isolate, std::make_tuple(offset.fX, offset.fY)).ToLocalChecked(),
        ffi::JSObject::New<Image>(isolate, result_image)
    };

    return v8::Object::New(isolate, v8::Null(isolate), keys, values, 3);
}

namespace {

struct DecodeTaskGroup
{
    void OnPerTaskExecute(uint32_t idx, const std::string& path)
    {
        TRACE_EVENT("renderer", "DecodeImageTask");
        auto stream = SkFILEStream::Make(path.c_str());
        if (!stream)
        {
            results[idx].error = fmt::format("failed to open file `{}`", path);
            return;
        }
        SkCodec::Result status;
        auto codec = SkCodec::MakeFromStream(std::move(stream), &status);
        if (!codec)
        {
            results[idx].error = fmt::format("failed to create codec for `{}`: {}",
                                             path, SkCodec::ResultToString(status));
            return;
        }
        auto [image, decode_status] = codec->getImage();
        if (decode_status != SkCodec::Result::kSuccess)
        {
            results[idx].error = fmt::format("failed to decode `{}`: {}",
                                             path, SkCodec::ResultToString(decode_status));
            return;
        }
        CHECK(image);
        results[idx].image = image;
    }

    void OnPerTaskFinish()
    {
        if (--pending_tasks > 0)
            return;

        v8::HandleScope handle_scope(isolate);

        std::vector<v8::Local<v8::Value>> values;
        values.reserve(results.size());
        for (const Result& result : results)
        {
            std::unordered_map<std::string, v8::Local<v8::Value>> iface;
            if (!result.error.empty())
            {
                iface["success"] = v8::Boolean::New(isolate, false);
                iface["error"] = ffi::Cast<std::string>::ToChecked(isolate, result.error);
            }
            else
            {
                iface["success"] = v8::Boolean::New(isolate, true);
                iface["image"] = ffi::JSObject::New<Image>(isolate, result.image);
            }
            values.emplace_back(ffi::Cast<decltype(iface)>::ToChecked(isolate, iface));
        }

        auto ctx = isolate->GetCurrentContext();
        resolver.Get(isolate)->Resolve(
                ctx, v8::Array::New(isolate, values.data(), values.size())).Check();
    }

    uint32_t pending_tasks;
    struct Result
    {
        std::string error;
        sk_sp<SkImage> image;
    };
    std::vector<Result> results;

    v8::Isolate *isolate;
    v8::Global<v8::Promise::Resolver> resolver;
};

} // namespace anonymous

ffi::RetLocal<v8::Value>
Image::BatchFromEncodedFile(std::vector<std::string> paths)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto ctx = isolate->GetCurrentContext();
    auto resolver = v8::Promise::Resolver::New(ctx).ToLocalChecked();
    if (paths.empty())
    {
        // fastpath: resolve with an empty array
        resolver->Resolve(ctx, v8::Array::New(isolate)).Check();
        return resolver->GetPromise();
    }

    uint32_t task_count = paths.size();
    DecodeTaskGroup *task = new DecodeTaskGroup{};
    task->pending_tasks = task_count;
    task->results = std::vector<DecodeTaskGroup::Result>(task_count);
    task->isolate = isolate;
    task->resolver.Reset(isolate, resolver);

    EventLoop *evloop = EventLoop::GetCurrent();
    for (uint32_t i = 0; i < task_count; i++)
    {
        evloop->enqueueThreadPoolTrivialTask([path = paths[i], i, task] {
            task->OnPerTaskExecute(i, path);
        }, [task] {
            task->OnPerTaskFinish();
        });
    }

    return resolver->GetPromise();
}

ffi::Ret<void> Image::dispose()
{
    image_info_cache_.Reset();
    bounds_cache_.Reset();
    image_.reset();
    NotifyDisposeState(DisposeState::kDisposed);
    return {};
}

ffi::RetLocal<v8::Value> Image::getImageInfo()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!image_info_cache_.IsEmpty())
        return image_info_cache_.Get(isolate);

    auto info = ffi::JSObject::New<ImageInfo>(isolate, image_->imageInfo());
    image_info_cache_.Reset(isolate, info);
    return info;
}

ffi::RetLocal<v8::Value> Image::getBounds()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!bounds_cache_.IsEmpty())
        return bounds_cache_.Get(isolate);

    auto bounds = CreateJSRect(isolate, SkRect::Make(image_->bounds()));
    bounds_cache_.Reset(isolate, bounds);
    return bounds;
}

ffi::Ret<bool> Image::isTextureBacked()
{
    return image_->isTextureBacked();
}

ffi::Ret<size_t> Image::textureSize()
{
    return image_->textureSize();
}

#define CACHEHINT_SELECT(v) (v ? SkImage::CachingHint::kAllow_CachingHint \
                               : SkImage::CachingHint::kDisallow_CachingHint)
#define RET_NULL_OR_NEW(image) \
    if (!image) { return v8::Null(isolate); } \
    else { return ffi::JSObject::New<Image>(isolate, std::move(image)); }

ffi::Ret<bool> Image::isValid(ffi::Opt<ffi::Class<GpuDirectContext>> context)
{
    return image_->isValid(GRCTX_NULL_SELECT(context));
}

ffi::Ret<bool> Image::readPixels(ffi::Opt<ffi::Class<GpuDirectContext>> ctx,
                                 PixmapAdapter dst, int32_t src_x, int32_t src_y,
                                 bool allow_caching)
{
    return image_->readPixels(GRCTX_NULL_SELECT(ctx), *dst, src_x, src_y,
                              CACHEHINT_SELECT(allow_caching));
}

ffi::RetLocal<v8::Value>
Image::asyncRescaleAndReadPixels(ffi::Class<ImageInfo> image_info,
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

    image_->asyncRescaleAndReadPixels(dst_image_info,
                                      src_rect,
                                      *rescale_gamma,
                                      *rescale_mode,
                                      AsyncRescaleContext::Callback,
                                      async_ctx);
    return promise;
}

ffi::RetLocal<v8::Value>
Image::asyncRescaleAndReadPixelsYUV420(ffi::Enum<SkYUVColorSpace> yuv_color_space,
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

    image_->asyncRescaleAndReadPixelsYUV420(*yuv_color_space,
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
Image::asyncRescaleAndReadPixelsYUVA420(ffi::Enum<SkYUVColorSpace> yuv_color_space,
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

    image_->asyncRescaleAndReadPixelsYUVA420(*yuv_color_space,
                                             (*dst_color_space)->GetColorSpace(),
                                             src_rect,
                                             dst_size,
                                             *rescale_gamma,
                                             *rescale_mode,
                                             AsyncRescaleContext::Callback,
                                             async_ctx);
    return promise;
}

ffi::Ret<bool> Image::scalePixels(PixmapAdapter dst,
                                  SamplingOptionsAdapter sampling,
                                  bool allow_caching)
{
    return image_->scalePixels(*dst, *sampling, CACHEHINT_SELECT(allow_caching));
}

#define MAKE_IMAGE_XXX(method, ...) \
    v8::Isolate *isolate = v8::Isolate::GetCurrent(); \
    sk_sp<SkImage> image = image_->method(__VA_ARGS__); \
    RET_NULL_OR_NEW(image);

ffi::RetLocal<v8::Value> Image::makeSubset(ffi::Opt<ffi::Class<GpuDirectContext>> context,
                                           RectAdapter subset)
{
    MAKE_IMAGE_XXX(makeSubset, GRCTX_NULL_SELECT(context), (*subset).round())
}

ffi::RetLocal<v8::Value> Image::withDefaultMipmaps()
{
    MAKE_IMAGE_XXX(withDefaultMipmaps)
}

ffi::RetLocal<v8::Value>
Image::makeNonTextureImage(ffi::Opt<ffi::Class<GpuDirectContext>> context)
{
    MAKE_IMAGE_XXX(makeNonTextureImage, GRCTX_NULL_SELECT(context))
}

ffi::RetLocal<v8::Value>
Image::makeRasterImage(ffi::Opt<ffi::Class<GpuDirectContext>> context,
                       bool allow_caching)
{
    MAKE_IMAGE_XXX(makeRasterImage, GRCTX_NULL_SELECT(context),
                   CACHEHINT_SELECT(allow_caching))
}

ffi::RetLocal<v8::Value>
Image::makeColorSpace(ffi::Opt<ffi::Class<GpuDirectContext>> context,
                      ffi::Class<ColorSpace> target)
{
    MAKE_IMAGE_XXX(makeColorSpace, GRCTX_NULL_SELECT(context),
                   (*target)->GetColorSpace())
}

ffi::RetLocal<v8::Value>
Image::makeColorTypeAndColorSpace(ffi::Opt<ffi::Class<GpuDirectContext>> context,
                                  ffi::Enum<SkColorType> target_color_type,
                                  ffi::Class<ColorSpace> target_cs)
{
    MAKE_IMAGE_XXX(makeColorTypeAndColorSpace, GRCTX_NULL_SELECT(context),
                   *target_color_type, (*target_cs)->GetColorSpace())
}

ffi::RetLocal<v8::Value> Image::reinterpretColorSpace(ffi::Class<ColorSpace> cs)
{
    MAKE_IMAGE_XXX(reinterpretColorSpace, (*cs)->GetColorSpace())
}

ffi::RetLocal<v8::Value> Image::makeShader(ffi::Enum<SkTileMode> tmx,
                                           ffi::Enum<SkTileMode> tmy,
                                           const SamplingOptionsAdapter& sampling,
                                           const ffi::Opt<Mat3x3Adapter>& local_matrix)
{
    sk_sp<SkShader> shader = image_->makeShader(
            *tmx, *tmy, *sampling, local_matrix ? &(**local_matrix) : nullptr);
    if (!shader)
        return ffi::Fail(ffi::kErr, "failed to create shader from the image");
    return ffi::JSObject::New<Shader>(v8::Isolate::GetCurrent(), shader);
}

ffi::RetLocal<v8::Value> Image::makeRawShader(ffi::Enum<SkTileMode> tmx,
                                              ffi::Enum<SkTileMode> tmy,
                                              const SamplingOptionsAdapter& sampling,
                                              const ffi::Opt<Mat3x3Adapter>& local_matrix)
{
    sk_sp<SkShader> shader = image_->makeRawShader(
            *tmx, *tmy, *sampling, local_matrix ? &(**local_matrix) : nullptr);
    if (!shader)
        return ffi::Fail(ffi::kErr, "failed to create raw shader from the image");
    return ffi::JSObject::New<Shader>(v8::Isolate::GetCurrent(), shader);
}

GALLIUM_BINDINGS_RENDERER_NS_END
