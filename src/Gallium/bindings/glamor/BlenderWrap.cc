#include "Glamor/Blender.h"

#include "Gallium/bindings/glamor/Exports.h"
#include "Gallium/bindings/glamor/PromiseHelper.h"
#include "Gallium/bindings/glamor/Scene.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

BlenderWrap::BlenderWrap(glamor::Shared<glamor::RenderClientObject> object)
    : RenderClientObjectWrap(std::move(object))
{
}

BlenderWrap::~BlenderWrap() = default;

namespace {

v8::Local<v8::Value> void_invoke_noparam(v8::Isolate *isolate, uint32_t op, RenderClientObjectWrap *wrap)
{
    auto closure = PromiseClosure::New(isolate, nullptr);
    wrap->getObject()->Invoke(op, closure, PromiseClosure::HostCallback);
    return closure->getPromise();
}

} // namespace anonymous

v8::Local<v8::Value> BlenderWrap::dispose()
{
    return void_invoke_noparam(v8::Isolate::GetCurrent(), CROP_BLENDER_DISPOSE, this);
}

v8::Local<v8::Value> BlenderWrap::update(v8::Local<v8::Value> sceneObject)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    Scene *scene = binder::Class<Scene>::unwrap_object(isolate, sceneObject);
    if (scene == nullptr)
        g_throw(TypeError, "Argument 'scene' must be an instance of Scene");

    std::shared_ptr<glamor::LayerTree> layer_tree(scene->takeLayerTree());

    auto closure = PromiseClosure::New(isolate, nullptr);
    getObject()->Invoke(CROP_BLENDER_UPDATE, closure, PromiseClosure::HostCallback, layer_tree);

    return closure->getPromise();
}

GALLIUM_BINDINGS_GLAMOR_NS_END
