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

#include "include/core/SkSerialProcs.h"
#include "include/core/SkData.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkImage.h"
#include "include/core/SkCanvas.h"

#include "Gallium/bindings/renderer/Flattenable.h"
#include "Gallium/bindings/renderer/PathEffect.h"
#include "Gallium/bindings/renderer/Image.h"
#include "Gallium/bindings/renderer/Picture.h"
#include "Gallium/bindings/renderer/Typeface.h"
#include "Gallium/bindings/renderer/Shader.h"
#include "Gallium/bindings/renderer/ImageFilter.h"
#include "Gallium/bindings/renderer/Blender.h"
#include "Gallium/bindings/renderer/ColorFilter.h"
#include "Gallium/bindings/renderer/BufferUtils.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

namespace {

struct DeserialprocContext
{
    DeserialprocContext(v8::Local<v8::Context> jsctx_,
                        v8::Local<v8::Function> func_,
                        v8::Local<v8::Object> receiver_)
        : jsctx(jsctx_), func(func_), receiver(receiver_) {}

    v8::Local<v8::Context> jsctx;
    v8::Local<v8::Function> func;
    v8::Local<v8::Object> receiver;
};

template<typename SkT, typename WrapT>
sk_sp<SkT> deserialproc_trampoline(const void *data, size_t length, void *userdata)
{
    DeserialprocContext *ctx = static_cast<DeserialprocContext*>(userdata);
    v8::Isolate *isolate = ctx->func->GetIsolate();
    v8::HandleScope handle_scope(isolate);

    // Skia does not guarantee that `data` pointer shares the same memory
    // of the original buffer which the user provides for deserializing.
    std::shared_ptr<v8::BackingStore> memory_store = v8::ArrayBuffer::NewBackingStore(
            const_cast<void*>(data), length, +[](void*, size_t, void*) {}, nullptr);
    auto array_buffer = v8::ArrayBuffer::New(isolate, memory_store);

    v8::Local<v8::Value> args[] = {
        v8::Uint8Array::New(array_buffer, 0, length)
    };
    auto maybe_ret = ctx->func->Call(ctx->jsctx, ctx->receiver, 1, args);

    // To make sure the memory is not reachable from user JavaScript anymore.
    // Because Skia does not guarantee that `data` pointer keeps valid after the
    // callback returns.
    CHECK(array_buffer->Detach({}).IsJust());
    CHECK(memory_store.unique());

    if (maybe_ret.IsEmpty())
        return nullptr;

    v8::Local<v8::Value> ret = maybe_ret.ToLocalChecked();
    // Use Skia's default action
    if (ret->IsNullOrUndefined())
        return nullptr;
    if (!ret->IsObject())
    {
        // Exception will be caught in `DeserializeImpl()` function later.
        isolate->ThrowError("deserializer callback must return a deserialized object or null");
        return nullptr;
    }

    WrapT *wrapper = ffi::JSObject::Unwrap<WrapT>(isolate, ret.As<v8::Object>());
    if (!wrapper)
    {
        isolate->ThrowError("deserializer callback must return a deserialized object or null");
        return nullptr;
    }

    if constexpr (std::is_same_v<WrapT, Picture>)
        return wrapper->GetSkPicture();
    if constexpr (std::is_same_v<WrapT, Image>)
        return wrapper->GetSkImage();
    if constexpr (std::is_same_v<WrapT, Typeface>)
        return wrapper->GetSkTypeface();

    MARK_UNREACHABLE();
}

} // namespace anonymous

ffi::RetLocal<v8::Value>
Flattenable::DeserializeImpl(Type type, const ffi::Mem<uint8_t>& memory,
                             ffi::Opt<ffi::IFace<Deserializers>> deserials)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();

    SkDeserialProcs sk_deserial_procs;
    std::unique_ptr<DeserialprocContext> pict_proc_ctx;
    std::unique_ptr<DeserialprocContext> image_proc_ctx;
    std::unique_ptr<DeserialprocContext> typeface_proc_ctx;

    if (deserials)
    {
        ffi::IFace<Deserializers> &iface = *deserials;
        if (iface->allow_sksl)
            sk_deserial_procs.fAllowSkSL = *iface->allow_sksl;

        if (iface->on_picture)
        {
            pict_proc_ctx = std::make_unique<DeserialprocContext>(
                    ctx, *iface->on_picture, iface.Object());
            sk_deserial_procs.fPictureProc = deserialproc_trampoline<SkPicture, Picture>;
            sk_deserial_procs.fPictureCtx = pict_proc_ctx.get();
        }

        if (iface->on_image)
        {
            image_proc_ctx = std::make_unique<DeserialprocContext>(
                    ctx, *iface->on_image, iface.Object());
            sk_deserial_procs.fImageProc = deserialproc_trampoline<SkImage, Image>;
            sk_deserial_procs.fImageCtx = image_proc_ctx.get();
        }

        if (iface->on_typeface)
        {
            typeface_proc_ctx = std::make_unique<DeserialprocContext>(
                    ctx, *iface->on_typeface, iface.Object());
            sk_deserial_procs.fTypefaceProc = deserialproc_trampoline<SkTypeface, Typeface>;
            sk_deserial_procs.fTypefaceCtx = typeface_proc_ctx.get();
        }
    }

    v8::Local<v8::Value> result;
    v8::TryCatch try_catch(isolate);
    if (type == Type::kPicture)
    {
        sk_sp<SkPicture> picture = SkPicture::MakeFromData(
                memory.Address(), memory.ByteSize(), &sk_deserial_procs);
        result = picture ? ffi::JSObject::New<Picture>(isolate, picture).As<v8::Value>()
                         : v8::Null(isolate).As<v8::Value>();
    }
#define ELSE_IF_XXX_TYPE(TYPE)                                                      \
    else if (type == Type::k##TYPE) {                                               \
        sk_sp<Sk##TYPE> sk(static_cast<Sk##TYPE*>(SkFlattenable::Deserialize(       \
            SkFlattenable::kSk##TYPE##_Type, memory.Address(), memory.ByteSize(),   \
            &sk_deserial_procs).release()));                                        \
        result = sk ? ffi::JSObject::New<TYPE>(isolate, sk).As<v8::Value>()         \
                    : v8::Null(isolate).As<v8::Value>();                            \
    }
    ELSE_IF_XXX_TYPE(PathEffect)
    ELSE_IF_XXX_TYPE(Shader)
    ELSE_IF_XXX_TYPE(ColorFilter)
    ELSE_IF_XXX_TYPE(ImageFilter)
    ELSE_IF_XXX_TYPE(Blender)
    else
    {
        return ffi::Fail(ffi::kErr, "not implemented yet");
    }
#undef ELSE_IF_XXX_TYPE

    // TODO(sora): support other types(Drawable, MaskFilter)

    FFI_RET_TO_PROPAGATE_IF_CAUGHT(try_catch)
    return result;
}

namespace {

struct SerialprocContext
{
    v8::Local<v8::Function> func;
    v8::Local<v8::Object> receiver;
};

template<typename SkT, typename WrapT>
sk_sp<SkData> serialproc_trampoline(SkT *flattenable, void *userdata)
{
    SerialprocContext *proc_ctx = static_cast<SerialprocContext*>(userdata);
    v8::Isolate *isolate = proc_ctx->func->GetIsolate();
    v8::HandleScope handle_scope(isolate);

    flattenable->ref();
    v8::Local<v8::Value> args[] = {
        ffi::JSObject::New<WrapT>(isolate, sk_sp<SkT>(flattenable))
    };
    auto context = isolate->GetCurrentContext();
    auto maybe_ret = proc_ctx->func->Call(context, proc_ctx->receiver, 1, args);
    if (maybe_ret.IsEmpty())
    {
        // An exception was thrown by the function call. But there is no way to interrupt
        // the serialization process, and just return nullptr to let Skia take its default
        // action (use Skia's internal format to serialize).
        // The exception will be handled later when we return to `Flattenable::serialize()`.
        return nullptr;
    }

    v8::Local<v8::Value> ret = maybe_ret.ToLocalChecked();
    if (ret->IsNullOrUndefined())
        return nullptr;

    if (!ret->IsArrayBufferView())
    {
        isolate->ThrowError("serializer callbacks must return a ArrayBufferView or null");
        return nullptr;
    }
    return WrapArrayBufferToSkData(isolate, ret.As<v8::ArrayBufferView>(), kNoCopy_MemoryFlag);
}

} // namespace anonymous

ffi::RetLocal<v8::Value>
Flattenable::serialize(ffi::Opt<ffi::IFace<Serializers>> serials)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    SkSerialProcs sk_serial_procs;

    std::unique_ptr<SerialprocContext> image_proc_ctx;
    std::unique_ptr<SerialprocContext> picture_proc_ctx;
    std::unique_ptr<SerialprocContext> typeface_proc_ctx;

    if (serials)
    {
        ffi::IFace<Serializers>& iface = *serials;
        if (iface->on_image)
        {
            sk_serial_procs.fImageProc = serialproc_trampoline<SkImage, Image>;
            image_proc_ctx = std::make_unique<SerialprocContext>();
            image_proc_ctx->func = *iface->on_image;
            image_proc_ctx->receiver = serials->Object();
            sk_serial_procs.fImageCtx = image_proc_ctx.get();
        }

        if (iface->on_picture)
        {
            sk_serial_procs.fPictureProc = serialproc_trampoline<SkPicture, Picture>;
            picture_proc_ctx = std::make_unique<SerialprocContext>();
            picture_proc_ctx->func = *iface->on_picture;
            picture_proc_ctx->receiver = serials->Object();
            sk_serial_procs.fPictureCtx = picture_proc_ctx.get();
        }

        if (iface->on_typeface)
        {
            sk_serial_procs.fTypefaceProc = serialproc_trampoline<SkTypeface, Typeface>;
            typeface_proc_ctx = std::make_unique<SerialprocContext>();
            typeface_proc_ctx->func = *iface->on_typeface;
            typeface_proc_ctx->receiver = serials->Object();
            sk_serial_procs.fTypefaceCtx = typeface_proc_ctx.get();
        }
    }

    v8::TryCatch try_catch(isolate);
    sk_sp<SkData> data = OnSerializeImpl(&sk_serial_procs);
    FFI_RET_TO_PROPAGATE_IF_CAUGHT(try_catch)

    if (!data)
        return ffi::Fail(ffi::kErr, "failed to serialize the object");

    CHECK(data->unique());
    size_t size = data->size();
    void *address = data->writable_data();
    return ffi::Mem<uint8_t>(isolate, std::move(data), address, size).TypedArray();
}

GALLIUM_BINDINGS_RENDERER_NS_END
