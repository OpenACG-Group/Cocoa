#include "Gallium/bindings/glamor/Scene.h"
#include "Gallium/bindings/glamor/SceneBuilder.h"

#include "Glamor/Layers/TransformLayer.h"
#include "Glamor/Layers/PictureLayer.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

SceneBuilder::SceneBuilder(int32_t width, int32_t height)
    : width_(width)
    , height_(height)
{
}

v8::Local<v8::Object> SceneBuilder::getSelfHandle()
{
    if (self_handle_.IsEmpty())
    {
        v8::Isolate *isolate = v8::Isolate::GetCurrent();
        self_handle_.Reset(isolate, binder::Class<SceneBuilder>::find_object(isolate, this));
        CHECK(!self_handle_.IsEmpty());

        // Make this weak-reference to avoid circular reference.
        self_handle_.SetWeak();
    }
    return self_handle_.Get(v8::Isolate::GetCurrent());
}

void SceneBuilder::pushLayer(const std::shared_ptr<glamor::ContainerLayer>& layer)
{
    CHECK(layer && "Invalid layer");

    if (!layer_stack_.empty())
        layer_stack_.top()->AppendChildLayer(layer);

    layer_stack_.push(layer);

    // The first layer pushed into stack will be the root layer.
    if (layer_tree_ == nullptr)
        layer_tree_ = layer;
}

void SceneBuilder::addLayer(const std::shared_ptr<glamor::Layer>& layer)
{
    CHECK(layer && "Invalid layer");
    if (layer_stack_.empty())
        g_throw(Error, "Inserting a container layer before adding other layers is required");
    layer_stack_.top()->AppendChildLayer(layer);
}

v8::Local<v8::Value> SceneBuilder::build()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!layer_tree_)
        g_throw(Error, "Building an empty scene");

    v8::Local<v8::Value> object = binder::Class<Scene>::create_object(isolate, layer_tree_,
                                                                      SkISize::Make(width_, height_));
    // Since the `Scene` object is created, `SceneBuilder` is not available anymore
    layer_tree_.reset();
    while (!layer_stack_.empty())
        layer_stack_.pop();

    return object;
}

v8::Local<v8::Value> SceneBuilder::pop()
{
    if (layer_stack_.empty())
        g_throw(Error, "Empty layer stack");
    layer_stack_.pop();
    return getSelfHandle();
}

v8::Local<v8::Value> SceneBuilder::pushOffset(SkScalar x, SkScalar y)
{
    pushLayer(std::make_shared<glamor::TransformLayer>(SkMatrix::Translate(x, y)));
    return getSelfHandle();
}

v8::Local<v8::Value> SceneBuilder::addPicture(v8::Local<v8::Value> picture, SkScalar dx, SkScalar dy)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    CkPictureWrap *unwrapped = binder::Class<CkPictureWrap>::unwrap_object(isolate, picture);
    if (unwrapped == nullptr)
        g_throw(TypeError, "\'picture\' must be an instance of CkPicture");

    glamor::MaybeGpuObject<SkPicture> picture_retained(true, unwrapped->getPicture());
    addLayer(std::make_shared<glamor::PictureLayer>(SkPoint::Make(dx, dy), picture_retained));
    return getSelfHandle();
}

GALLIUM_BINDINGS_GLAMOR_NS_END
