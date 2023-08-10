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

#include "Glamor/Blender.h"

#include "Core/TraceEvent.h"
#include "Gallium/binder/TypeTraits.h"
#include "Gallium/bindings/glamor/Exports.h"
#include "Gallium/bindings/glamor/PromiseHelper.h"
#include "Gallium/bindings/glamor/Scene.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

BlenderWrap::BlenderWrap(gl::Shared<gl::RenderClientObject> handle)
    : handle_(std::move(handle))
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    using PictCast = CreateObjCast<gl::MaybeGpuObject<SkPicture>, CriticalPictureWrap>;
    DefineSignalEventsOnEventEmitter(this, handle_, {
        { "picture-captured", GLSI_BLENDER_PICTURE_CAPTURED,
          GenericInfoAcceptor<PictCast, NoCast<int32_t>> }
    });

    gl::Shared<gl::Blender> blender = handle_->As<gl::Blender>();
    if (blender->GetAttachedProfiler())
    {
        gl::Shared<gl::GProfiler> profiler = blender->GetAttachedProfiler();
        wrapped_profiler_.Reset(isolate,
            binder::NewObject<GProfilerWrap>(isolate, profiler));
    }
}

BlenderWrap::~BlenderWrap() = default;

v8::Local<v8::Value> BlenderWrap::getProfiler()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (wrapped_profiler_.IsEmpty())
        return v8::Null(isolate);

    return wrapped_profiler_.Get(isolate);
}

v8::Local<v8::Value> BlenderWrap::dispose()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return PromisifiedRemoteCall::Call(isolate, handle_, {}, GLOP_BLENDER_DISPOSE);
}

v8::Local<v8::Value> BlenderWrap::update(v8::Local<v8::Value> sceneObject)
{
    TRACE_EVENT("main", "BlenderWrap::update");

    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    auto *scene = binder::UnwrapObject<Scene>(isolate, sceneObject);
    if (scene == nullptr)
        g_throw(TypeError, "Argument 'scene' must be an instance of Scene");

    std::shared_ptr<gl::LayerTree> layer_tree(scene->takeLayerTree());
    CHECK(layer_tree.unique());

    return PromisifiedRemoteCall::Call(
            isolate, handle_, {}, GLOP_BLENDER_UPDATE, layer_tree);
}

v8::Local<v8::Value> BlenderWrap::deleteTexture(int64_t id)
{
    TRACE_EVENT("main", "BlenderWrap::deleteTexture");
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return PromisifiedRemoteCall::Call(
            isolate, handle_, {}, GLOP_BLENDER_DELETE_TEXTURE, id);
}

v8::Local<v8::Value>
BlenderWrap::newTextureDeletionSubscriptionSignal(int64_t id,
                                                  const std::string& signal_name)
{
    TRACE_EVENT("main", "BlenderWrap::newTextureDeletionSubscriptionSignal");

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return PromisifiedRemoteCall::Call(
        isolate, handle_,
        [this, signal_name](v8::Isolate *i, gl::RenderHostCallbackInfo& info) {
            int32_t signal_num = info.GetReturnValue<int32_t>();
            DefineSignalEventsOnEventEmitter(this, this->handle_, {
                    { signal_name.c_str(), static_cast<uint32_t>(signal_num) }
            });
            return v8::Undefined(i);
        },
        GLOP_BLENDER_NEW_TEXTURE_DELETION_SUBSCRIPTION_SIGNAL,
        id
    );
}

v8::Local<v8::Value> BlenderWrap::createTextureFromImage(v8::Local<v8::Value> image,
                                                         const std::string& annotation)
{
    TRACE_EVENT("main", "BlenderWrap::createTextureFromImage");

    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    auto *wrapper = binder::UnwrapObject<CkImageWrap>(isolate, image);
    if (!wrapper)
        g_throw(TypeError, "`image` must be an instance of `CkImage`");

    // `SkImage` object contained in the `CkImage` must not be a GPU-backend image,
    // and it is safe to reference and retain in another thread.
    sk_sp<SkImage> skia_image = wrapper->getImage();

    return PromisifiedRemoteCall::Call(
            isolate, handle_,
            PromisifiedRemoteCall::GenericConvert<NoCast<int64_t>>,
            GLOP_BLENDER_CREATE_TEXTURE_FROM_IMAGE,
            skia_image, annotation
    );
}

v8::Local<v8::Value>
BlenderWrap::createTextureFromEncodedData(v8::Local<v8::Value> buffer,
                                          v8::Local<v8::Value> alphaType,
                                          const std::string& annotation)
{
    TRACE_EVENT("main", "BlenderWrap::createTextureFromEncodedData");

    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    std::optional<binder::TypedArrayMemory<v8::Uint8Array>> array_memory =
            binder::GetTypedArrayMemory<v8::Uint8Array>(buffer);
    if (!array_memory)
        g_throw(TypeError, "Argument `buffer` must be an allocated `Uint8Array`");

    std::optional<SkAlphaType> alpha_type_enum;
    if (!alphaType->IsNull() && !alphaType->IsNumber())
        g_throw(TypeError, "`alphaType` must be `null` or an integer");

    if (alphaType->IsNumber())
    {
        using T = typename std::underlying_type<SkAlphaType>::type;
        T value = binder::from_v8<T>(isolate, alphaType);
        if (value < 0 || value > SkAlphaType::kLastEnum_SkAlphaType)
            g_throw(RangeError, "Invalid enumeration value for `alphaType`");
        alpha_type_enum = static_cast<SkAlphaType>(value);
    }

    // Convert `buffer` to a `Data` object without memory duplicating.
    // `Data` only contains a reference to the buffer, and the ownership
    // of array buffer still belongs to `buffer`.
    auto data = Data::MakeFromPtrWithoutCopy(array_memory->ptr, array_memory->byte_size, false);

    using InfoT = gl::RenderHostCallbackInfo;
    return PromisifiedRemoteCall::Call(
            isolate, handle_,
            [bufref = array_memory->memory](v8::Isolate *i, InfoT& info) {
                int64_t v = info.GetReturnValue<int64_t>();
                CHECK(v >= 0 && v <= 0xffffffff);
                return v8::Uint32::NewFromUnsigned(i, static_cast<uint32_t>(v));
            },
            GLOP_BLENDER_CREATE_TEXTURE_FROM_ENCODED_DATA,
            data, alpha_type_enum, annotation
    );
}

v8::Local<v8::Value>
BlenderWrap::createTextureFromPixmap(v8::Local<v8::Value> buffer,
                                     int32_t width,
                                     int32_t height,
                                     int32_t colorType,
                                     int32_t alphaType,
                                     const std::string& annotation)
{
    TRACE_EVENT("main", "BlenderWrap::createTextureFromPixmap");

    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    std::optional<binder::TypedArrayMemory<v8::Uint8Array>> array_memory =
            binder::GetTypedArrayMemory<v8::Uint8Array>(buffer);
    if (!array_memory)
        g_throw(TypeError, "Argument `buffer` must be an allocated `Uint8Array`");

    SkColorType color_type_enum;
    SkAlphaType alpha_type_enum;
    color_type_enum = static_cast<SkColorType>(colorType);
    alpha_type_enum = static_cast<SkAlphaType>(alphaType);

    if (color_type_enum < 0 || color_type_enum > SkColorType::kLastEnum_SkColorType)
        g_throw(RangeError, "Invalid enumeration value for `colorType`");

    if (alpha_type_enum < 0 || alpha_type_enum > SkAlphaType::kLastEnum_SkAlphaType)
        g_throw(RangeError, "Invalid enumeration value for `alphaType`");

    if (width <= 0 || height <= 0)
        g_throw(RangeError, "Invalid width or height for texture");

    SkImageInfo image_info = SkImageInfo::Make(width, height,
                                               color_type_enum, alpha_type_enum);

    using InfoT = gl::RenderHostCallbackInfo;
    return PromisifiedRemoteCall::Call(
            isolate, handle_,
            [bufref = array_memory->memory](v8::Isolate *i, InfoT& info) {
                int64_t v = info.GetReturnValue<int64_t>();
                CHECK(v >= 0 && v <= 0xffffffff);
                return v8::Uint32::NewFromUnsigned(i, static_cast<uint32_t>(v));
            },
            GLOP_BLENDER_CREATE_TEXTURE_FROM_PIXMAP,
            array_memory->ptr, image_info, annotation
    );
}

v8::Local<v8::Value> BlenderWrap::captureNextFrameAsPicture()
{
    TRACE_EVENT("main", "BlenderWrap::captureNextFrameAsPicture");
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return PromisifiedRemoteCall::Call(
            isolate, handle_, PromisifiedRemoteCall::GenericConvert<NoCast<int32_t>>,
            GLOP_BLENDER_CAPTURE_NEXT_FRAME_AS_PICTURE);
}

v8::Local<v8::Value> BlenderWrap::purgeRasterCacheResources()
{
    TRACE_EVENT("main", "BlenderWrap::purgeRasterCacheResources");
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return PromisifiedRemoteCall::Call(
            isolate, handle_, {}, GLOP_BLENDER_PURGE_RASTER_CACHE_RESOURCES);
}

v8::Local<v8::Object> BlenderWrap::OnGetObjectSelf(v8::Isolate *isolate)
{
    return GetObjectWeakReference().Get(isolate);
}

GALLIUM_BINDINGS_GLAMOR_NS_END
