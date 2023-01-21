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

#include "Gallium/bindings/glamor/Exports.h"
#include "Gallium/bindings/glamor/CkMatrixWrap.h"
#include "Gallium/bindings/core/Exports.h"
#include "Gallium/bindings/utau/Exports.h"
#include "Utau/VideoFrameGLEmbedder.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

CkImageWrap::CkImageWrap(sk_sp<SkImage> image)
        : image_(std::move(image))
{
}


v8::Local<v8::Value> CkImageWrap::MakeFromEncodedData(v8::Local<v8::Value> bufferObject)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    Buffer *buffer = binder::Class<Buffer>::unwrap_object(isolate, bufferObject);
    if (buffer == nullptr)
        g_throw(TypeError, "'buffer' must be an instance of core.Buffer");

    // Not used here, but we do need it to prevent `buffer` being freed
    // by garbage collector before using it.
    auto global_buffer = std::make_shared<v8::Global<v8::Value>>(isolate, bufferObject);

    // Create a promise to resolve later.
    auto resolver = v8::Promise::Resolver::New(isolate->GetCurrentContext()).ToLocalChecked();
    auto global_resolver = std::make_shared<v8::Global<v8::Promise::Resolver>>(isolate, resolver);

    // Submit a task to thread pool
    EventLoop::Ref().enqueueThreadPoolTask<sk_sp<SkImage>>([buffer]() -> sk_sp<SkImage> {
        // Do decode here
        sk_sp<SkData> data = SkData::MakeWithCopy(buffer->addressU8(), buffer->length());
        std::unique_ptr<SkCodec> codec = SkCodec::MakeFromData(data);
        auto [image, result] = codec->getImage();

        return image;
    }, [isolate, global_buffer, global_resolver](sk_sp<SkImage>&& image) {
        // Receive decoding result here
        v8::HandleScope scope(isolate);
        v8::Local<v8::Promise::Resolver> resolver = global_resolver->Get(isolate);
        v8::Local<v8::Context> ctx = isolate->GetCurrentContext();

        if (image)
            resolver->Resolve(ctx, binder::Class<CkImageWrap>::create_object(isolate, image)).Check();
        else
            resolver->Reject(ctx, binder::to_v8(isolate, "Failed to decode image from buffer")).Check();

        // We do not need them anymore
        global_buffer->Reset();
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
    EventLoop::Ref().enqueueThreadPoolTask<sk_sp<SkImage>>([path]() -> sk_sp<SkImage> {
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
            resolver->Resolve(ctx, binder::Class<CkImageWrap>::create_object(isolate, image)).Check();
        else
            resolver->Reject(ctx, binder::to_v8(isolate, "Failed to decode image from buffer")).Check();

        // We do not need them anymore
        global_resolver->Reset();
    });

    return resolver->GetPromise();
}

v8::Local<v8::Value> CkImageWrap::MakeFromVideoBuffer(v8::Local<v8::Value> vbo)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto *wrapper = binder::Class<utau_wrap::VideoBufferWrap>::unwrap_object(isolate, vbo);
    if (!wrapper)
        g_throw(TypeError, "Argument `vbo` must be an instance of `utau.VideoBuffer`");

    auto *embedder = utau::GlobalContext::Ref().GetVideoFrameGLEmbedder();
    CHECK(embedder);
    
    sk_sp<SkImage> image = embedder->ConvertToRasterImage(wrapper->GetBuffer());
    if (!image)
        g_throw(Error, "Failed to convert video buffer to an image");

    return binder::Class<CkImageWrap>::create_object(isolate, image);
}

v8::Local<v8::Value> CkImageWrap::encodeToData(uint32_t format, int quality)
{
    sk_sp<SkData> data = image_->encodeToData(static_cast<SkEncodedImageFormat>(format), quality);
    if (!data)
        g_throw(Error, "Failed to encode image");

    // Maybe there is a way to avoid this expensive copy
    return Buffer::MakeFromPtrCopy(data->data(), data->size());
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

    sk_sp<SkImage> image_ref = image_;
    v8::Local<v8::Value> buffer = Buffer::MakeFromExternal(pixmap.writable_addr(),
                                                           pixmap.computeByteSize(),
                                                           [image_ref] {});

    std::unordered_map<std::string_view, v8::Local<v8::Value>> result{
        { "buffer", buffer },
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
        auto *wrap = binder::Class<CkMatrix>::unwrap_object(isolate, local_matrix);
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

    return binder::Class<CkShaderWrap>::create_object(isolate, shader);
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
