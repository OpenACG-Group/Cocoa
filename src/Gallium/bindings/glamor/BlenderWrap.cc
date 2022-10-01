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

#include "Gallium/bindings/core/Exports.h"
#include "Gallium/bindings/glamor/Exports.h"
#include "Gallium/bindings/glamor/PromiseHelper.h"
#include "Gallium/bindings/glamor/Scene.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

BlenderWrap::BlenderWrap(gl::Shared<gl::RenderClientObject> object)
    : RenderClientObjectWrap(std::move(object))
{
}

BlenderWrap::~BlenderWrap() = default;

namespace {

template<typename...ArgsT>
v8::Local<v8::Value> invoke_void_return(v8::Isolate *isolate,
                                        uint32_t op,
                                        RenderClientObjectWrap *wrap,
                                        ArgsT&&...args)
{
    auto closure = PromiseClosure::New(isolate, nullptr);
    wrap->getObject()->Invoke(op, closure, PromiseClosure::HostCallback,
                              std::forward<ArgsT>(args)...);
    return closure->getPromise();
}

template<typename Ret, typename...ArgsT>
v8::Local<v8::Value> invoke_primitive_type_return(v8::Isolate *isolate,
                                                  uint32_t op,
                                                  RenderClientObjectWrap *wrap,
                                                  ArgsT&&...args)
{
    auto acceptor = [](v8::Isolate *i, gl::RenderHostCallbackInfo& info) {
        return binder::to_v8(i, info.GetReturnValue<Ret>());
    };

    auto closure = PromiseClosure::New(isolate, acceptor);
    wrap->getObject()->Invoke(op, closure, PromiseClosure::HostCallback,
                              std::forward<ArgsT>(args)...);
    return closure->getPromise();
}

} // namespace anonymous

v8::Local<v8::Value> BlenderWrap::dispose()
{
    return invoke_void_return(v8::Isolate::GetCurrent(),
                              GLOP_BLENDER_DISPOSE, this);
}

v8::Local<v8::Value> BlenderWrap::update(v8::Local<v8::Value> sceneObject)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    Scene *scene = binder::Class<Scene>::unwrap_object(isolate, sceneObject);
    if (scene == nullptr)
        g_throw(TypeError, "Argument 'scene' must be an instance of Scene");

    std::shared_ptr<gl::LayerTree> layer_tree(scene->takeLayerTree());
    CHECK(layer_tree.unique());

    auto closure = PromiseClosure::New(isolate, nullptr);
    getObject()->Invoke(GLOP_BLENDER_UPDATE, closure, PromiseClosure::HostCallback, layer_tree);

    return closure->getPromise();
}

v8::Local<v8::Value> BlenderWrap::deleteTexture(int64_t id)
{
    return invoke_void_return(v8::Isolate::GetCurrent(),
                              GLOP_BLENDER_DELETE_TEXTURE,
                              this,
                              id);
}

v8::Local<v8::Value>
BlenderWrap::newTextureDeletionSubscriptionSignal(int64_t id,
                                                  const std::string& sigName)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    auto acceptor = [this, sigName](v8::Isolate *i, gl::RenderHostCallbackInfo& info) {
        int32_t signal_num = info.GetReturnValue<int32_t>();

        // Register a named signal on the `RenderClientObjectWrap` interface.
        this->defineSignal(sigName.c_str(), signal_num, {});
        return v8::Undefined(i);
    };

    auto closure = PromiseClosure::New(isolate, acceptor);
    getObject()->Invoke(GLOP_BLENDER_NEW_TEXTURE_DELETION_SUBSCRIPTION_SIGNAL,
                        closure,
                        PromiseClosure::HostCallback,
                        id);

    return closure->getPromise();
}

v8::Local<v8::Value> BlenderWrap::createTextureFromImage(v8::Local<v8::Value> image,
                                                         const std::string& annotation)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    CkImageWrap *wrapper = binder::Class<CkImageWrap>::unwrap_object(isolate, image);
    if (!wrapper)
        g_throw(TypeError, "`image` must be an instance of `CkImage`");

    // `SkImage` object contained in the `CkImage` must not be a GPU-backend image,
    // and it is safe to reference and retain in another thread.
    sk_sp<SkImage> skia_image = wrapper->getImage();

    return invoke_primitive_type_return<int64_t>(isolate,
                                                 GLOP_BLENDER_CREATE_TEXTURE_FROM_IMAGE,
                                                 this,
                                                 skia_image,
                                                 annotation);
}

v8::Local<v8::Value>
BlenderWrap::createTextureFromEncodedData(v8::Local<v8::Value> buffer,
                                          v8::Local<v8::Value> alphaType,
                                          const std::string& annotation)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    Buffer *wrapper = binder::Class<Buffer>::unwrap_object(isolate, buffer);
    if (!wrapper)
        g_throw(TypeError, "`buffer` must be an instance of `Buffer`");

    std::optional<SkAlphaType> alpha_type_enum;
    if (!alphaType->IsNull() || !alphaType->IsNumber())
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
    auto data = Data::MakeFromPtrWithoutCopy(wrapper->addressU8(),
                                             wrapper->length(), false);

    // We use the persistent handle to prevent `buffer` from being destroyed
    // by V8's garbage collector.
    auto persistent_buffer = std::make_shared<v8::Global<v8::Value>>(isolate, buffer);
    using InfoT = gl::RenderHostCallbackInfo;
    auto acceptor = [persistent_buffer](v8::Isolate *i, InfoT& info) {
        persistent_buffer->Reset();
        return binder::to_v8(i, info.GetReturnValue<int64_t>());
    };

    auto closure = PromiseClosure::New(isolate, acceptor);
    getObject()->Invoke(GLOP_BLENDER_CREATE_TEXTURE_FROM_ENCODED_DATA,
                        closure,
                        PromiseClosure::HostCallback,
                        data,
                        alpha_type_enum,
                        annotation);

    return closure->getPromise();
}

v8::Local<v8::Value>
BlenderWrap::createTextureFromPixmap(v8::Local<v8::Value> buffer,
                                     int32_t width,
                                     int32_t height,
                                     int32_t colorType,
                                     int32_t alphaType,
                                     const std::string& annotation)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    Buffer *wrapped = binder::Class<Buffer>::unwrap_object(isolate, buffer);
    if (!wrapped)
        g_throw(TypeError, "`buffer` must be an instance of `Buffer`");

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

    // We use the persistent handle to prevent `buffer` from being destroyed
    // by V8's garbage collector.
    auto persistent_buffer = std::make_shared<v8::Global<v8::Value>>(isolate, buffer);
    using InfoT = gl::RenderHostCallbackInfo;
    auto acceptor = [persistent_buffer](v8::Isolate *i, InfoT& info) {
        persistent_buffer->Reset();
        return binder::to_v8(i, info.GetReturnValue<int64_t>());
    };

    auto closure = PromiseClosure::New(isolate, acceptor);
    getObject()->Invoke(GLOP_BLENDER_CREATE_TEXTURE_FROM_PIXMAP,
                        closure,
                        PromiseClosure::HostCallback,
                        static_cast<const void*>(wrapped->addressU8()),
                        image_info,
                        annotation);

    return closure->getPromise();
}

GALLIUM_BINDINGS_GLAMOR_NS_END
