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
#include "include/core/SkPicture.h"
#include "include/core/SkStream.h"
#include "fmt/format.h"

#include "Glamor/PresentThread.h"
#include "Gallium/binder/CallV8.h"
#include "Gallium/bindings/glamor/Exports.h"
#include "Gallium/bindings/glamor/PromiseHelper.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

void CriticalPictureWrap::setCollectionCallback(v8::Local<v8::Value> F)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!F->IsFunction())
        g_throw(TypeError, "callback argument must be a function");

    callback_.Reset(isolate, v8::Local<v8::Function>::Cast(F));

    picture_.SetObjectCollectedCallback([this, isolate]() {
        v8::HandleScope scope(isolate);
        v8::Local<v8::Function> func = this->callback_.Get(isolate);
        binder::Invoke(isolate, func, isolate->GetCurrentContext()->Global());
    });
}

void CriticalPictureWrap::discardOwnership()
{
    if (!picture_)
        g_throw(Error, "CriticalPicture is empty");

    picture_.reset();
}

v8::Local<v8::Value> CriticalPictureWrap::sanitize()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!picture_)
        g_throw(Error, "Null CriticalPicture object");
    gl::MaybeGpuObject<SkPicture> picture(picture_);
    return PromisifiedRemoteTask::Submit<sk_sp<SkPicture>>(
        isolate,
        [picture] {
            sk_sp<SkData> data = picture->serialize();
            if (!data)
            {
                // This exception could be handled by the remote task
                // mechanism, then converted to a string to reject the
                // corresponding promise automatically.
                throw std::runtime_error("Failed to serialize the picture");
            }

            // Once a picture is serialized, all the GPU resources (textures)
            // should be read back and stored in the serialized data.

            // Deserializing the serialized picture makes the new picture completely
            // isolated with GPU resources (it becomes a standalone picture, except typefaces).
            return SkPicture::MakeFromData(data.get());
        }, [](sk_sp<SkPicture> picture) {
            v8::Isolate *isolate = v8::Isolate::GetCurrent();
            return binder::NewObject<CkPictureWrap>(isolate, std::move(picture));
        }
    );
}

v8::Local<v8::Value> CriticalPictureWrap::serialize()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!picture_)
        g_throw(Error, "Null CriticalPicture object");
    gl::MaybeGpuObject<SkPicture> picture(picture_);
    return PromisifiedRemoteTask::Submit<sk_sp<SkData>>(
        isolate,
        [picture] {
            sk_sp<SkData> data = picture->serialize();
            if (!data)
                throw std::runtime_error("Failed to serialize the picture");
            return data;
        }, [](sk_sp<SkData> data) {
            v8::Isolate *isolate = v8::Isolate::GetCurrent();
            void *writable_data = data->writable_data();
            auto backing_store = binder::CreateBackingStoreFromSmartPtrMemory(
                    data, writable_data, data->size());
            return v8::ArrayBuffer::New(isolate, backing_store);
        }
    );
}

CkPictureWrap::CkPictureWrap(sk_sp<SkPicture> picture)
        : picture_(std::move(picture))
        , picture_size_hint_(0)
{
    CHECK(picture_);

    picture_size_hint_ = picture_->approximateBytesUsed();

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    isolate->AdjustAmountOfExternalAllocatedMemory(
            static_cast<int64_t>(picture_size_hint_));
}

CkPictureWrap::~CkPictureWrap()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (isolate)
    {
        isolate->AdjustAmountOfExternalAllocatedMemory(
                -static_cast<int64_t>(picture_size_hint_));
    }
}

v8::Local<v8::Value> CkPictureWrap::serialize()
{
    sk_sp<SkData> data = picture_->serialize();
    if (data == nullptr)
        g_throw(Error, "Failed to serialize SkPicture object");

    // `SkData::writable_data()` assert the refcnt is 1.
    // However, the `binder::CreateBackingStoreFromStartPtrMemory` call copies
    // a reference of `SkData`, so we should get the writable data pointer
    // before calling `binder::CreateBackingStoreFromStartPtrMemory`.
    void *writable_data = data->writable_data();
    auto backing_store = binder::CreateBackingStoreFromSmartPtrMemory(
            data, writable_data, data->size());

    return v8::ArrayBuffer::New(v8::Isolate::GetCurrent(), backing_store);
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

v8::Local<v8::Value> CkPictureWrap::MakeFromData(v8::Local<v8::Value> buffer)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!buffer->IsTypedArray())
        g_throw(TypeError, "Argument `buffer` must be a TypedArray");

    auto memory = binder::GetTypedArrayMemory<v8::TypedArray>(buffer);
    if (!memory)
        g_throw(Error, "Not a valid TypedArray");

    sk_sp<SkPicture> pict = SkPicture::MakeFromData(memory->ptr, memory->size);
    if (!pict)
        g_throw(Error, "Cannot serialize a CkPicture from buffer");

    return binder::NewObject<CkPictureWrap>(isolate, pict);
}

v8::Local<v8::Value> CkPictureWrap::MakeFromFile(const std::string& path)
{
    sk_sp<SkData> data = SkData::MakeFromFileName(path.c_str());
    if (!data)
        g_throw(Error, fmt::format("Failed to read file {}", path));

    sk_sp<SkPicture> pict = SkPicture::MakeFromData(data.get());
    if (!pict)
        g_throw(Error, fmt::format("Cannot serialize a CkPicture from {}", path));

    return binder::NewObject<CkPictureWrap>(v8::Isolate::GetCurrent(), pict);
}

uint32_t CkPictureWrap::uniqueId()
{
    return picture_->uniqueID();
}

GALLIUM_BINDINGS_GLAMOR_NS_END
