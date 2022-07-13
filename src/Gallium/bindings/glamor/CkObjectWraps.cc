#include "include/core/SkImageEncoder.h"
#include "include/core/SkData.h"
#include "include/core/SkPicture.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkBitmap.h"
#include "include/codec/SkCodec.h"

#include "Core/EventLoop.h"
#include "Gallium/binder/Class.h"
#include "Gallium/bindings/core/Exports.h"
#include "Gallium/bindings/glamor/Exports.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

CkPictureWrap::CkPictureWrap(sk_sp<SkPicture> picture)
    : picture_(std::move(picture))
{
    CHECK(picture_);
}

v8::Local<v8::Value> CkPictureWrap::serialize()
{
    sk_sp<SkData> data = picture_->serialize();
    if (data == nullptr)
        g_throw(Error, "Failed to serialize SkPicture object");

    // Maybe there is a way to avoid this expensive memory copy
    v8::Local<v8::Object> buffer = Buffer::MakeFromPtrCopy(data->data(), data->size());
    return buffer;
}

const sk_sp<SkPicture>& CkPictureWrap::getPicture() const
{
    return picture_;
}

uint32_t CkPictureWrap::approximateOpCount(bool nested)
{
    return picture_->approximateOpCount(nested);
}

uint32_t CkPictureWrap::approximateByteUsed()
{
    return picture_->approximateBytesUsed();
}

v8::Local<v8::Value> CkPictureWrap::MakeFromData(v8::Local<v8::Value> bufferObject)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    Buffer *buffer = binder::Class<Buffer>::unwrap_object(isolate, bufferObject);
    if (!buffer)
        g_throw(TypeError, "'buffer' must be an instance of core.Buffer");

    sk_sp<SkData> data = SkData::MakeWithoutCopy(buffer->getWriteableDataPointerByte(),
                                                 buffer->length());
    CHECK(data);

    sk_sp<SkPicture> pict = SkPicture::MakeFromData(data.get());
    if (!pict)
        g_throw(Error, "Cannot serialize a CkPicture from buffer");

    return binder::Class<CkPictureWrap>::create_object(isolate, pict);
}

v8::Local<v8::Value> CkPictureWrap::MakeFromFile(const std::string& path)
{
    sk_sp<SkData> data = SkData::MakeFromFileName(path.c_str());
    sk_sp<SkPicture> pict = SkPicture::MakeFromData(data.get());
    if (!pict)
        g_throw(Error, fmt::format("Cannot serialize a CkPicture from {}", path));

    return binder::Class<CkPictureWrap>::create_object(v8::Isolate::GetCurrent(), pict);
}

uint32_t CkPictureWrap::uniqueId()
{
    return picture_->uniqueID();
}


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
        sk_sp<SkData> data = SkData::MakeWithCopy(buffer->getWriteableDataPointerByte(), buffer->length());
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

v8::Local<v8::Value> CkImageWrap::encodeToData(uint32_t format, int quality)
{
    sk_sp<SkData> data = image_->encodeToData(static_cast<SkEncodedImageFormat>(format), quality);
    if (!data)
        g_throw(Error, "Failed to encode image");

    // Maybe there is a way to avoid this expensive copy
    return Buffer::MakeFromPtrCopy(data->data(), data->size());
}

GALLIUM_BINDINGS_GLAMOR_NS_END
