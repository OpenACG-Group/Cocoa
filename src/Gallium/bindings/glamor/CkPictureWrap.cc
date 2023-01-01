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
#include "include/core/SkSerialProcs.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkStream.h"
#include "fmt/format.h"

#include "Gallium/binder/CallV8.h"
#include "Gallium/bindings/glamor/Exports.h"
#include "Gallium/bindings/glamor/CanvasKitTransferContext.h"
#include "Gallium/bindings/core/Exports.h"
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

    // Following code will be executed in the rendering thread
    // to touch the GPU retained resources safely.
    gl::RenderHostTaskRunner::Task sanitizer = [picture]() -> std::any {
        sk_sp<SkData> data = picture->serialize();
        if (!data)
        {
            throw std::runtime_error("Failed to serialize the picture");
        }

        // Once a picture is serialized, all the GPU resources (textures)
        // should be read back and stored in the serialized data.

        // Deserializing the serialized picture makes the new picture completely
        // isolated with GPU resources (it becomes a standalone picture, except typefaces).
        return {SkPicture::MakeFromData(data.get())};
    };

    using W = CkPictureWrap;
    using T = sk_sp<SkPicture>;
    auto closure = PromiseClosure::New(isolate, PromiseClosure::CreateObjectConverter<W, T>);

    // The picture itself may contain critical GPU resources which are owned by
    // rendering thread only. Sanitizer will download those critical GPU textures
    // into system memory and serialize them. To avoid touching them unexpectedly,
    // it is essential to execute the sanitizer function on rendering thread.
    auto task_runner = gl::GlobalScope::Ref().GetRenderHost()
            ->GetRenderHostTaskRunner();
    CHECK(task_runner);
    task_runner->Invoke(GLOP_TASKRUNNER_RUN, closure,
                        PromiseClosure::HostCallback, sanitizer);

    return closure->getPromise();
}

v8::Local<v8::Value> CriticalPictureWrap::serialize()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    if (!picture_)
        g_throw(Error, "Null CriticalPicture object");

    // See also comments in `sanitize` method.

    gl::MaybeGpuObject<SkPicture> picture(picture_);
    gl::RenderHostTaskRunner::Task serializer = [picture]() -> std::any {
        sk_sp<SkData> data = picture->serialize();
        if (!data)
            throw std::runtime_error("Failed to serialize the picture");
        return {data};
    };

    auto runner = gl::GlobalScope::Ref().GetRenderHost()
            ->GetRenderHostTaskRunner();
    CHECK(runner);

    auto converter = [](v8::Isolate *i, gl::RenderHostCallbackInfo& info) {
        // Note that `SkData` only allows accessing `writable_data()` when
        // `SkData` object does not be referenced by other owners.
        auto data = std::move(info.GetReturnValue<sk_sp<SkData>>());
        CHECK(data);

        return Buffer::MakeFromExternal(data->writable_data(), data->size(),
                                        [data]() {});
    };
    auto closure = PromiseClosure::New(isolate, converter);

    runner->Invoke(GLOP_TASKRUNNER_RUN, closure, PromiseClosure::HostCallback, serializer);

    return closure->getPromise();
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

    v8::Local<v8::Object> buffer = Buffer::MakeFromExternal(data->writable_data(), data->size(),
                                                            [data]() {});
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

namespace {

sk_sp<SkTypeface>
deserialize_typeface_in_transfer_mode(const void* data, size_t length, void *ctx)
{
    CHECK(gl::GlobalScope::Instance());

    // It is very wired that Skia provides a SkStream** pointer as a
    // `const void *data` argument. One possible reason may be that all the
    // types of SkStream does NOT expose a directly accessible memory address
    // to the contained data. Argument `length` is actually `sizeof(SkStream*)`,
    // which we do not need.
    CHECK(length == sizeof(uintptr_t));
    SkStream *stream = *reinterpret_cast<SkStream**>(const_cast<void*>(data));

    auto parsed_key = CanvasKitTransferContext::TypefaceKey::ParseFromBinary(stream);
    if (!parsed_key)
        return nullptr;

    auto *transfer_context = reinterpret_cast<CanvasKitTransferContext*>(
            gl::GlobalScope::Ref().GetExternalDataPointer());
    CHECK(transfer_context);

    return transfer_context->RequestTypeface(*parsed_key);
}

} // namespace anonymous

v8::Local<v8::Value> CkPictureWrap::MakeFromData(v8::Local<v8::Value> bufferObject,
                                                 PictureUsage usage)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    Buffer *buffer = binder::Class<Buffer>::unwrap_object(isolate, bufferObject);
    if (!buffer)
        g_throw(TypeError, "'buffer' must be an instance of core.Buffer");

    SkDeserialProcs deserial_procs{};
    if (usage == kTransfer_Usage)
        deserial_procs.fTypefaceProc = deserialize_typeface_in_transfer_mode;

    sk_sp<SkPicture> pict = SkPicture::MakeFromData(buffer->addressU8(), buffer->length(),
                                                    &deserial_procs);
    if (!pict)
        g_throw(Error, "Cannot serialize a CkPicture from buffer");

    return binder::Class<CkPictureWrap>::create_object(isolate, pict);
}

v8::Local<v8::Value> CkPictureWrap::MakeFromFile(const std::string& path,
                                                 PictureUsage usage)
{
    sk_sp<SkData> data = SkData::MakeFromFileName(path.c_str());
    if (!data)
        g_throw(Error, fmt::format("Failed to read file {}", path));

    SkDeserialProcs deserial_procs{};
    if (usage == kTransfer_Usage)
        deserial_procs.fTypefaceProc = deserialize_typeface_in_transfer_mode;

    sk_sp<SkPicture> pict = SkPicture::MakeFromData(data.get(), &deserial_procs);
    if (!pict)
        g_throw(Error, fmt::format("Cannot serialize a CkPicture from {}", path));

    return binder::Class<CkPictureWrap>::create_object(v8::Isolate::GetCurrent(), pict);
}

uint32_t CkPictureWrap::uniqueId()
{
    return picture_->uniqueID();
}

GALLIUM_BINDINGS_GLAMOR_NS_END
