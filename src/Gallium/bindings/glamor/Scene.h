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
    Scene(const std::shared_ptr<gl::ContainerLayer>& rootLayer,
          const SkISize& frameSize);
    ~Scene();

    /**
     * `toImage` rasterizes current scene to a pixel image.
     * It is a slow operation which is performed by CPU rasterizer in worker
     * thread.
     */

    //! TSDecl: function toImage(): Promise<CkImage>
    g_nodiscard v8::Local<v8::Value> toImage(int32_t width, int32_t height);

    //! TSDecl: function toString(): string
    g_nodiscard std::string toString();

    //! TSDecl: function dispose(): void
    void dispose();

    //! TSDecl: property readonly isDisposed: boolean
    g_nodiscard g_inline bool isDisposed() const {
        return (!layer_tree_);
    }

    g_nodiscard std::unique_ptr<gl::LayerTree> takeLayerTree() {
        return std::move(layer_tree_);
    }

    /**
     * Cocoa itself does not use this API. It is designed for third-party
     * language bindings (like cairo-embedder in `//natives/cairo-embedder`) to
     * allow them to access the layer tree temporarily (not take its ownership).
     * Language bindings must make sure that they never take the ownership of
     * `LayerTree` object which is from `Scene` object.
     */
    g_nodiscard std::unique_ptr<gl::LayerTree>& GetLayerTree() {
        return layer_tree_;
    }

private:
    bool disposed_;
    std::unique_ptr<gl::LayerTree> layer_tree_;
};

GALLIUM_BINDINGS_GLAMOR_NS_END
#endif //COCOA_GALLIUM_BINDINGS_GLAMOR_SCENE_H
