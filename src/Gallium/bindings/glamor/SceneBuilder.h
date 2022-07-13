#ifndef COCOA_GALLIUM_BINDINGS_GLAMOR_SCENEBUILDER_H
#define COCOA_GALLIUM_BINDINGS_GLAMOR_SCENEBUILDER_H

#include <stack>

#include "Gallium/bindings/glamor/Exports.h"
#include "Glamor/Layers/Layer.h"
#include "Glamor/Layers/ContainerLayer.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

/* JSDecl: class SceneBuilder */
class SceneBuilder
{
public:
    SceneBuilder(int32_t width, int32_t height);
    ~SceneBuilder() = default;

    /* JSDecl: function build(): Scene */
    v8::Local<v8::Value> build();

    /* JSDecl: function pop(): SceneBuilder */
    v8::Local<v8::Value> pop();

    /* JSDecl: function pushOffset(x: number, y: number): SceneBuilder */
    v8::Local<v8::Value> pushOffset(SkScalar x, SkScalar y);

    /* JSDecl: function addPicture(picture: CkPicture): SceneBuilder */
    v8::Local<v8::Value> addPicture(v8::Local<v8::Value> picture, SkScalar dx, SkScalar dy);

private:
    v8::Local<v8::Object> getSelfHandle();
    void pushLayer(const std::shared_ptr<glamor::ContainerLayer>& layer);
    void addLayer(const std::shared_ptr<glamor::Layer>& layer);

    v8::Global<v8::Object> self_handle_;
    int32_t     width_;
    int32_t     height_;
    std::shared_ptr<glamor::ContainerLayer> layer_tree_;
    std::stack<std::shared_ptr<glamor::ContainerLayer>> layer_stack_;
};

GALLIUM_BINDINGS_GLAMOR_NS_END
#endif //COCOA_GALLIUM_BINDINGS_GLAMOR_SCENEBUILDER_H
