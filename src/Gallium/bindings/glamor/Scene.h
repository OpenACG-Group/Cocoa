#ifndef COCOA_GALLIUM_BINDINGS_GLAMOR_SCENE_H
#define COCOA_GALLIUM_BINDINGS_GLAMOR_SCENE_H

#include "Glamor/Layers/LayerTree.h"
#include "Glamor/Layers/Layer.h"
#include "Gallium/bindings/glamor/Exports.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

/**
 * A Scene object always holds an independent layer tree which can be applied to blender
 * through `BlenderWrap::updateScene` method. It can also be constructed by `SceneBuilder`.
 */
class Scene
{
public:
    Scene(const std::shared_ptr<glamor::ContainerLayer>& rootLayer,
          const SkISize& frameSize);
    ~Scene();

    /**
     * `toImage` rasterizes current scene to a pixel image.
     * It is a slow operation which is performed by CPU rasterizer in worker
     * thread.
     */

    /* JSDecl: function toImage(): Promise<CkImage> */
    g_nodiscard v8::Local<v8::Value> toImage(int32_t width, int32_t height);

    /* JSDecl: function dispose(): void */
    void dispose();

    g_nodiscard std::unique_ptr<glamor::LayerTree> takeLayerTree() {
        return std::move(layer_tree_);
    }

private:
    bool disposed_;
    std::unique_ptr<glamor::LayerTree> layer_tree_;
};

GALLIUM_BINDINGS_GLAMOR_NS_END
#endif //COCOA_GALLIUM_BINDINGS_GLAMOR_SCENE_H
