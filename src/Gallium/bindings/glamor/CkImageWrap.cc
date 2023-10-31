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

#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/codec/SkCodec.h"
#include "include/gpu/ganesh/SkImageGanesh.h"

#include "fmt/format.h"

#include "Gallium/binder/TypeTraits.h"
#include "Gallium/bindings/glamor/Exports.h"
#include "Gallium/bindings/glamor/CkImageWrap.h"
#include "Gallium/bindings/glamor/CkMatrixWrap.h"
#include "Gallium/bindings/glamor/CkPaintWrap.h"
#include "Gallium/bindings/glamor/CkPixmapWrap.h"
#include "Gallium/bindings/glamor/GpuDirectContext.h"
#include "Gallium/bindings/utau/Exports.h"
#include "Utau/VideoFrameGLEmbedder.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

namespace {

GrDirectContext *extract_gr_context(v8::Isolate *isolate,
                                    v8::Local<v8::Value> gpu_context)
{
    GpuDirectContext *ctx = binder::UnwrapObject<GpuDirectContext>(
            isolate, gpu_context);
    if (!ctx || ctx->isDisposed())
        g_throw(TypeError, "Invalid GPU context was provided");
    return ctx->GetHWComposeOffscreen()->GetSkiaGpuContext();
}

GrDirectContext *extract_gr_context_nullable(v8::Isolate *isolate,
                                             v8::Local<v8::Value> gpu_context)
{
    if (gpu_context->IsNullOrUndefined())
        return nullptr;
    return extract_gr_context(isolate, gpu_context);
}

} // namespace anonymous

CkImageWrap::CkImageWrap(sk_sp<SkImage> image)
        : image_(std::move(image))
{
}


void CkImageWrap::dispose()
{
    CheckDisposedOrThrow();
    image_.reset();
}

bool CkImageWrap::isDisposed()
{
    return !static_cast<bool>(image_);
}

void CkImageWrap::CheckDisposedOrThrow() const
{
    if (!image_)
        g_throw(Error, "Image reference has been disposed");
}

v8::Local<v8::Value> CkImageWrap::MakeFromEncodedData(v8::Local<v8::Value> arraybuffer)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    std::optional<binder::TypedArrayMemory<v8::Uint8Array>> buffer_memory =
            binder::GetTypedArrayMemory<v8::Uint8Array>(arraybuffer);
    if (!buffer_memory)
        g_throw(TypeError, "Argument `buffer` must be an allocated Uint8Array");

    // Create a promise to resolve later.
    auto resolver = v8::Promise::Resolver::New(isolate->GetCurrentContext()).ToLocalChecked();
    auto global_resolver = std::make_shared<v8::Global<v8::Promise::Resolver>>(isolate, resolver);

    // Submit a task to thread pool
    using DecodeResult = std::tuple<sk_sp<SkImage>, SkCodec::Result>;
    EventLoop::GetCurrent()->enqueueThreadPoolTask<DecodeResult>([mem = *buffer_memory]() {
        // Copy data and decode asynchronously
        sk_sp<SkData> data = SkData::MakeWithCopy(mem.ptr, mem.byte_size);
        CHECK(data);

        std::unique_ptr<SkCodec> codec = SkCodec::MakeFromData(data);
        if (codec == nullptr)
        {
            return std::make_tuple<sk_sp<SkImage>, SkCodec::Result>(
                    nullptr, SkCodec::Result::kInvalidInput);
        }

        return codec->getImage();
    }, [isolate, global_resolver](DecodeResult&& result) {
        // Receive decoding result here
        v8::HandleScope scope(isolate);
        v8::Local<v8::Promise::Resolver> resolver = global_resolver->Get(isolate);
        v8::Local<v8::Context> ctx = isolate->GetCurrentContext();

        sk_sp<SkImage> image = std::get<0>(result);
        SkCodec::Result res = std::get<1>(result);

        if (image)
        {
            v8::Local<v8::Object> obj = binder::NewObject<CkImageWrap>(isolate, image);
            resolver->Resolve(ctx, obj).Check();
        }
        else
        {
            std::string err_info = fmt::format(
                    "Failed to decode: {}", SkCodec::ResultToString(res));
            v8::Local<v8::String> err_info_str =
                    v8::String::NewFromUtf8(isolate,
                                            err_info.c_str(),
                                            v8::NewStringType::kNormal,
                                            static_cast<int>(err_info.length()))
                                            .ToLocalChecked();

            resolver->Reject(ctx, err_info_str).Check();
        }

        global_resolver->Reset();
    });

    return resolver->GetPromise();
}

v8::Local<v8::Value> CkImageWrap::MakeFromEncodedFile(const std::string& path)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();

    // Create a promise to resolver later
    v8::Local<v8::Promise::Resolver> resolver = v8::Promise::Resolver::New(ctx).ToLocalChecked();
    auto global_resolver = std::make_shared<v8::Global<v8::Promise::Resolver>>(isolate, resolver);

    // Submit a task to thread pool
    EventLoop::GetCurrent()->enqueueThreadPoolTask<sk_sp<SkImage>>([path]() -> sk_sp<SkImage> {
        // Do decode here
        sk_sp<SkData> data = SkData::MakeFromFileName(path.c_str());
        if (!data)
            return nullptr;
        std::unique_ptr<SkCodec> codec = SkCodec::MakeFromData(data);
        auto [image, result] = codec->getImage();
        return image;

    }, [isolate, global_resolver](sk_sp<SkImage>&& image) -> void {
        // Receive decoding result here
        v8::HandleScope scope(isolate);
        v8::Local<v8::Promise::Resolver> resolver = global_resolver->Get(isolate);
        v8::Local<v8::Context> ctx = isolate->GetCurrentContext();

        if (image)
            resolver->Resolve(ctx, binder::NewObject<CkImageWrap>(isolate, image)).Check();
        else
            resolver->Reject(ctx, binder::to_v8(isolate, "Failed to decode image from file")).Check();

        // We do not need them anymore
        global_resolver->Reset();
    });

    return resolver->GetPromise();
}

v8::Local<v8::Value> CkImageWrap::MakeFromVideoBuffer(v8::Local<v8::Value> vbo)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto *wrapper = binder::UnwrapObject<utau_wrap::VideoBufferWrap>(isolate, vbo);
    if (!wrapper)
        g_throw(TypeError, "Argument `vbo` must be an instance of `utau.VideoBuffer`");

    auto *embedder = utau::GlobalContext::Ref().GetVideoFrameGLEmbedder();
    CHECK(embedder);
    
    sk_sp<SkImage> image = embedder->ConvertToRasterImage(wrapper->GetBuffer());
    if (!image)
        g_throw(Error, "Failed to convert video buffer to an image");

    return binder::NewObject<CkImageWrap>(isolate, image);
}

v8::Local<v8::Value>
CkImageWrap::MakeDeferredFromPicture(v8::Local<v8::Value> picture,
                                     int32_t width,
                                     int32_t height,
                                     v8::Local<v8::Value> matrix,
                                     v8::Local<v8::Value> paint,
                                     int32_t bit_depth,
                                     int32_t color_space)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    auto *picture_wrap =
            binder::UnwrapObject<CkPictureWrap>(isolate, picture);
    if (!picture_wrap)
        g_throw(TypeError, "Argument `picture` must be an instance of `CkPicture`");

    if (width <= 0 || height <= 0)
        g_throw(RangeError, "Invalid image dimension provided by `width` and `height`");

    SkMatrix *p_matrix = nullptr;
    if (!matrix->IsNullOrUndefined())
    {
        auto *matrix_wrap = binder::UnwrapObject<CkMatrix>(isolate, matrix);
        if (!matrix_wrap)
            g_throw(TypeError, "Argument `matrix` must be `CkMatrix | null`");
        p_matrix = &matrix_wrap->GetMatrix();
    }

    SkPaint *p_paint = nullptr;
    if (!paint->IsNullOrUndefined())
    {
        auto *paint_wrap = binder::UnwrapObject<CkPaint>(isolate, paint);
        if (!paint_wrap)
            g_throw(TypeError, "Argument `paint` must be `CkPaint | null`");
        p_paint = &paint_wrap->GetPaint();
    }

    if (bit_depth < 0 || bit_depth > static_cast<int>(SkImages::BitDepth::kF16))
        g_throw(RangeError, "Argument `bitDepth` has an invalid enumeration value");

    sk_sp<SkImage> image = SkImages::DeferredFromPicture(
            picture_wrap->getPicture(),
            SkISize::Make(width, height),
            p_matrix,
            p_paint,
            static_cast<SkImages::BitDepth>(bit_depth),
            ExtrackCkColorSpace(color_space));
    if (!image)
        g_throw(Error, "Failed to create image from CkPicture");

    return binder::NewObject<CkImageWrap>(isolate, image);
}

v8::Local<v8::Value>
CkImageWrap::MakeFromMemoryCopy(v8::Local<v8::Value> buffer,
                                v8::Local<v8::Value> info,
                                int64_t row_bytes,
                                bool shared_pixel_memory)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    auto memory = binder::GetTypedArrayMemory<v8::TypedArray>(buffer);
    if (!memory)
        g_throw(TypeError, "Argument `buffer` must be an allocated TypedArray");

    SkImageInfo image_info = ExtractCkImageInfo(isolate, info);
    if (row_bytes < image_info.minRowBytes())
        g_throw(RangeError, "Row bytes are not large enough to hold one row pixels");
    if (image_info.computeByteSize(row_bytes) > memory->byte_size)
        g_throw(Error, "Size of the pixel buffer does not fit image info");

    SkPixmap pixmap(image_info, memory->ptr, row_bytes);
    sk_sp<SkImage> image;

    if (shared_pixel_memory)
    {
        auto *ctx = new TAMemoryForSkData{ memory->memory };
        image = SkImages::RasterFromPixmap(pixmap, [](const void*, void *ctx) {
            CHECK(ctx);
            delete static_cast<TAMemoryForSkData*>(ctx);
        }, ctx);
    }
    else
        image = SkImages::RasterFromPixmapCopy(pixmap);
    if (!image)
        g_throw(Error, "Failed to create image from pixel memory");

    return binder::NewObject<CkImageWrap>(isolate, image);
}

v8::Local<v8::Value>
CkImageWrap::MakeFromCompressedTextureData(v8::Local<v8::Value> data,
                                           int32_t width,
                                           int32_t height,
                                           int32_t compress_type)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    if (width <= 0 || height <= 0)
        g_throw(RangeError, "Invalid image dimension provided by `width` and `height`");

    auto memory = binder::GetTypedArrayMemory<v8::TypedArray>(data);
    if (!memory)
        g_throw(TypeError, "Argument `data` must be an allocated TypedArray");

    if (compress_type < 0 || compress_type > static_cast<int>(SkTextureCompressionType::kLast))
        g_throw(RangeError, "Argument `type` has an invalid enumeration value");

    sk_sp<SkData> shared_data = MakeSkDataFromTypedArrayMem(*memory);
    CHECK(shared_data);
    sk_sp<SkImage> image = SkImages::RasterFromCompressedTextureData(
            shared_data, width, height, static_cast<SkTextureCompressionType>(compress_type));
    if (!image)
        g_throw(Error, "Failed to create image from compressed texture data");

    return binder::NewObject<CkImageWrap>(isolate, image);
}

uint32_t CkImageWrap::uniqueId()
{
    CheckDisposedOrThrow();
    return image_->uniqueID();
}

int32_t CkImageWrap::getWidth()
{
    CheckDisposedOrThrow();
    return image_->width();
}

int32_t CkImageWrap::getHeight()
{
    CheckDisposedOrThrow();
    return image_->height();
}

uint32_t CkImageWrap::getAlphaType()
{
    CheckDisposedOrThrow();
    return image_->alphaType();
}

uint32_t CkImageWrap::getColorType()
{
    CheckDisposedOrThrow();
    return image_->colorType();
}

v8::Local<v8::Value> CkImageWrap::peekPixels(v8::Local<v8::Value> scope_callback)
{
    CheckDisposedOrThrow();
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!scope_callback->IsFunction())
        g_throw(TypeError, "Argument `scopeCallback` must be a Function");
    SkPixmap pixmap;
    if (!image_->peekPixels(&pixmap))
        g_throw(Error, "Pixel address in the image is not accessible");
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

void CkImageWrap::readPixels(v8::Local<v8::Value> dst_info,
                             v8::Local<v8::Value> dst_buffer,
                             int32_t dst_row_bytes,
                             int32_t src_x,
                             int32_t src_y)
{
    CheckDisposedOrThrow();
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto *info_wrap = binder::UnwrapObject<CkImageInfo>(isolate, dst_info);
    if (!info_wrap)
        g_throw(TypeError, "Argument `dstInfo` must be an instance of `CkImageInfo`");
    if (info_wrap->GetWrapped().minRowBytes() > dst_row_bytes)
        g_throw(Error, "`dstRowBytes` is too small to contain one row of pixels");

    auto dst_mem = binder::GetTypedArrayMemory<v8::TypedArray>(dst_buffer);
    if (!dst_mem)
        g_throw(TypeError, "Argument `dstBuffer` must be an allocated TypedArray");

    if (!image_->readPixels(info_wrap->GetWrapped(), dst_mem->ptr,
                            dst_row_bytes, src_x, src_y, SkImage::kAllow_CachingHint))
    {
        g_throw(Error, "Failed to read pixels");
    }
}

void CkImageWrap::scalePixels(v8::Local<v8::Value> dst_info,
                              v8::Local<v8::Value> dst_buffer,
                              int32_t dst_row_bytes,
                              int32_t sampling)
{
    CheckDisposedOrThrow();
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto *info_wrap = binder::UnwrapObject<CkImageInfo>(isolate, dst_info);
    if (!info_wrap)
        g_throw(TypeError, "Argument `dstInfo` must be an instance of `CkImageInfo`");
    if (info_wrap->GetWrapped().minRowBytes() > dst_row_bytes)
        g_throw(Error, "`dstRowBytes` is too small to contain one row of pixels");

    SkSamplingOptions sampling_opt = SamplingToSamplingOptions(sampling);

    auto dst_mem = binder::GetTypedArrayMemory<v8::TypedArray>(dst_buffer);
    if (!dst_mem)
        g_throw(TypeError, "Argument `dstBuffer` must be an allocated TypedArray");

    SkPixmap pixmap(info_wrap->GetWrapped(), dst_mem->ptr, dst_row_bytes);
    if (!image_->scalePixels(pixmap, sampling_opt, SkImage::kAllow_CachingHint))
        g_throw(Error, "Failed to scale pixels: pixel conversion is not possible");
}

v8::Local<v8::Value> CkImageWrap::makeSubset(v8::Local<v8::Value> gpu_context,
                                             v8::Local<v8::Value> subset)
{
    CheckDisposedOrThrow();
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    sk_sp<SkImage> result = image_->makeSubset(
            extract_gr_context_nullable(isolate, gpu_context),
            ExtractCkRect(isolate, subset).round()
    );
    if (!result)
        g_throw(Error, "Failed to make subset of image");
    return binder::NewObject<CkImageWrap>(isolate, result);
}

namespace {

template<bool RAW_SHADER>
v8::Local<v8::Value> make_shader_generic(const sk_sp<SkImage>& image, int32_t tmx, int32_t tmy,
                                         int32_t sampling, v8::Local<v8::Value> local_matrix)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (tmx < 0 || tmx > static_cast<int32_t>(SkTileMode::kLastTileMode))
        g_throw(RangeError, "Invalid enumeration value for argument `tmx`");
    if (tmy < 0 || tmy > static_cast<int32_t>(SkTileMode::kLastTileMode))
        g_throw(RangeError, "Invalid enumeration value for argument `tmx`");

    SkMatrix *matrix = nullptr;
    if (!local_matrix->IsNullOrUndefined())
    {
        auto *wrap = binder::UnwrapObject<CkMatrix>(isolate, local_matrix);
        if (!wrap)
            g_throw(TypeError, "Argument `local_matrix` must be an instance of `CkMatrix` or null");
        matrix = &wrap->GetMatrix();
    }

    sk_sp<SkShader> shader;
    if constexpr (RAW_SHADER)
    {
        shader = image->makeRawShader(static_cast<SkTileMode>(tmx),
                                      static_cast<SkTileMode>(tmy),
                                      SamplingToSamplingOptions(sampling),
                                      matrix);
    }
    else
    {
        shader = image->makeShader(static_cast<SkTileMode>(tmx),
                                   static_cast<SkTileMode>(tmy),
                                   SamplingToSamplingOptions(sampling),
                                   matrix);
    }

    if (!shader)
        return v8::Null(isolate);

    return binder::NewObject<CkShaderWrap>(isolate, shader);
}

} // namespace

v8::Local<v8::Value> CkImageWrap::makeShader(int32_t tmx, int32_t tmy, int32_t sampling,
                                             v8::Local<v8::Value> local_matrix)
{
    CheckDisposedOrThrow();
    return make_shader_generic<false>(image_, tmx, tmy, sampling, local_matrix);
}

v8::Local<v8::Value> CkImageWrap::makeRawShader(int32_t tmx, int32_t tmy, int32_t sampling,
                                                v8::Local<v8::Value> local_matrix)
{
    CheckDisposedOrThrow();
    return make_shader_generic<true>(image_, tmx, tmy, sampling, local_matrix);
}

bool CkImageWrap::hasMipmaps()
{
    CheckDisposedOrThrow();
    return image_->hasMipmaps();
}

v8::Local<v8::Value> CkImageWrap::withDefaultMipmaps()
{
    CheckDisposedOrThrow();
    sk_sp<SkImage> result = image_->withDefaultMipmaps();
    if (!result)
        g_throw(Error, "Failed to create an image with default mipmaps");
    return binder::NewObject<CkImageWrap>(v8::Isolate::GetCurrent(), result);
}

bool CkImageWrap::isTextureBacked()
{
    CheckDisposedOrThrow();
    return image_->isTextureBacked();
}

size_t CkImageWrap::approximateTextureSize()
{
    CheckDisposedOrThrow();
    return image_->textureSize();
}

bool CkImageWrap::isValid(v8::Local<v8::Value> context)
{
    CheckDisposedOrThrow();
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return image_->isValid(extract_gr_context_nullable(isolate, context));
}

v8::Local<v8::Value> CkImageWrap::makeNonTextureImage(v8::Local<v8::Value> gpu_context)
{
    CheckDisposedOrThrow();
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    sk_sp<SkImage> result = image_->makeNonTextureImage(
            extract_gr_context_nullable(isolate, gpu_context));
    if (!result)
        g_throw(Error, "Failed to copy texture from GPU memory");
    return binder::NewObject<CkImageWrap>(isolate, result);
}

v8::Local<v8::Value> CkImageWrap::makeRasterImage(v8::Local<v8::Value> context)
{
    CheckDisposedOrThrow();
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    sk_sp<SkImage> result = image_->makeRasterImage(
            extract_gr_context_nullable(isolate, context));
    if (!result)
        g_throw(Error, "Failed to decode lazy image or copy texture from GPU");
    return binder::NewObject<CkImageWrap>(isolate, result);
}

v8::Local<v8::Value>
CkImageWrap::makeWithFilter(v8::Local<v8::Value> gpu_context,
                            v8::Local<v8::Value> filter,
                            v8::Local<v8::Value> subset,
                            v8::Local<v8::Value> clip_bounds)
{
    CheckDisposedOrThrow();
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    GrDirectContext *ctx = extract_gr_context_nullable(isolate, gpu_context);
    auto *image_filter = binder::UnwrapObject<CkImageFilterWrap>(isolate, filter);
    if (!image_filter)
        g_throw(TypeError, "Argument `filter` must be a CkImageFilter");

    SkIRect filtered_subset = SkIRect::MakeEmpty();
    SkIPoint filtered_offset = SkIPoint::Make(0, 0);

    SkImageFilter *filter_ptr = image_filter->GetSkObject().get();
    sk_sp<SkImage> result;
    if (ctx)
    {
        result = SkImages::MakeWithFilter(
                ctx, image_,
                filter_ptr,
                ExtractCkRect(isolate, subset).round(),
                ExtractCkRect(isolate, clip_bounds).round(),
                &filtered_subset,
                &filtered_offset);
    }
    else
    {
        result = SkImages::MakeWithFilter(
                image_,
                filter_ptr,
                ExtractCkRect(isolate, subset).round(),
                ExtractCkRect(isolate, clip_bounds).round(),
                &filtered_subset,
                &filtered_offset);
    }
    if (!result)
        g_throw(Error, "Image could not be created or GPU context mismatched");

    std::unordered_map<std::string_view, v8::Local<v8::Value>> ret{
        { "image", binder::NewObject<CkImageWrap>(isolate, result) },
        { "subset", NewCkRect(isolate, SkRect::Make(filtered_subset)) },
        { "offset", NewCkPoint(isolate, SkPoint::Make(static_cast<float>(filtered_offset.fX),
                                                      static_cast<float>(filtered_offset.fY))) }
    };
    return binder::to_v8(isolate, ret);
}

GALLIUM_BINDINGS_GLAMOR_NS_END
