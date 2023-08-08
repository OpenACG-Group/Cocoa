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

#include "fmt/format.h"

#include "Gallium/binder/TypeTraits.h"
#include "Gallium/bindings/glamor/Exports.h"
#include "Gallium/bindings/glamor/CkMatrixWrap.h"
#include "Gallium/bindings/utau/Exports.h"
#include "Utau/VideoFrameGLEmbedder.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

CkImageWrap::CkImageWrap(sk_sp<SkImage> image)
        : image_(std::move(image))
{
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

uint32_t CkImageWrap::uniqueId()
{
    return image_->uniqueID();
}

int32_t CkImageWrap::getWidth()
{
    return image_->width();
}

int32_t CkImageWrap::getHeight()
{
    return image_->height();
}

uint32_t CkImageWrap::getAlphaType()
{
    return image_->alphaType();
}

uint32_t CkImageWrap::getColorType()
{
    return image_->colorType();
}

v8::Local<v8::Value> CkImageWrap::makeSharedPixelsBuffer()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    SkPixmap pixmap;
    if (!image_->peekPixels(&pixmap))
        g_throw(Error, "Image is not readable");

    image_->ref();
    std::shared_ptr<v8::BackingStore> store =
            v8::ArrayBuffer::NewBackingStore(pixmap.writable_addr(),
                                             pixmap.computeByteSize(),
                                             [](void *data, size_t length, void *image) {
                                                 CHECK(image);
                                                 reinterpret_cast<SkImage*>(image)->unref();
                                             },
                                             image_.get());

    CHECK(store);

    std::unordered_map<std::string_view, v8::Local<v8::Value>> result{
        { "buffer", v8::ArrayBuffer::New(isolate, store) },
        { "width", binder::to_v8(isolate, image_->width()) },
        { "height", binder::to_v8(isolate, image_->height()) },
        { "colorType", binder::to_v8(isolate, static_cast<int32_t>(image_->colorType())) },
        { "alphaType", binder::to_v8(isolate, static_cast<int32_t>(image_->alphaType())) },
        { "rowBytes", binder::to_v8(isolate, pixmap.rowBytes()) }
    };

    return binder::to_v8(isolate, result);
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
    return make_shader_generic<false>(image_, tmx, tmy, sampling, local_matrix);
}

v8::Local<v8::Value> CkImageWrap::makeRawShader(int32_t tmx, int32_t tmy, int32_t sampling,
                                                v8::Local<v8::Value> local_matrix)
{
    return make_shader_generic<true>(image_, tmx, tmy, sampling, local_matrix);
}

GALLIUM_BINDINGS_GLAMOR_NS_END
