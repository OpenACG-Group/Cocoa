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

#include "Gallium/bindings/ExportableObjectBase.h"
#include "Gallium/bindings/glamor/Exports.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

/**
 * A Scene object always holds an independent layer tree which can be applied to
 * the ContentAggregator, through `ContentAggregatorWrap::updateScene` method.
 */
class Scene : public ExportableObjectBase
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

    g_nodiscard std::unique_ptr<gl::LayerTree> takeLayerTree() {
        return std::move(layer_tree_);
    }

private:
    std::unique_ptr<gl::LayerTree> layer_tree_;
};

GALLIUM_BINDINGS_GLAMOR_NS_END
#endif //COCOA_GALLIUM_BINDINGS_GLAMOR_SCENE_H
