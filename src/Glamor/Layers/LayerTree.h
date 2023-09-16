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

#ifndef COCOA_GLAMOR_LAYERS_LAYERTREE_H
#define COCOA_GLAMOR_LAYERS_LAYERTREE_H

#include <list>
#include <sstream>

#include "include/core/SkSize.h"
#include "include/core/SkPicture.h"

#include "Glamor/Glamor.h"
#include "Glamor/ContentAggregator.h"
#include "Glamor/Layers/Layer.h"
#include "Glamor/MaybeGpuObject.h"
GLAMOR_NAMESPACE_BEGIN

class ContainerLayer;
class RasterDrawOpObserver;

class LayerTree
{
public:
    CO_NONCOPYABLE(LayerTree)
    CO_NONASSIGNABLE(LayerTree)

    explicit LayerTree(const SkISize& frameSize);
    ~LayerTree();

    MaybeGpuObject<SkPicture> Flatten(const SkRect& bounds);

    bool Preroll(Layer::PrerollContext *context);
    void Paint(Layer::PaintContext *context);

    g_inline void SetRootLayer(const std::shared_ptr<ContainerLayer>& root) {
        root_layer_ = root;
    }

    g_inline void SetFrameSize(const SkISize& size) {
        frame_size_ = size;
    }

    g_nodiscard g_inline ContainerLayer *GetRootLayer() const {
        return root_layer_.get();
    }

    g_nodiscard g_inline const SkISize& GetFrameSize() const {
        return frame_size_;
    }

    g_inline void AppendObserver(const std::shared_ptr<RasterDrawOpObserver>& observer) {
        auto itr = std::find(raster_draw_op_observers_.begin(),
                             raster_draw_op_observers_.end(),
                             observer);
        if (itr == raster_draw_op_observers_.end())
            raster_draw_op_observers_.push_back(observer);
    }

    g_nodiscard g_inline const auto& GetObservers() const {
        return raster_draw_op_observers_;
    }

    g_inline void RemoveObserver(const std::shared_ptr<RasterDrawOpObserver>& observer) {
        auto itr = std::find(raster_draw_op_observers_.begin(),
                             raster_draw_op_observers_.end(),
                             observer);
        if (itr != raster_draw_op_observers_.end())
            raster_draw_op_observers_.erase(itr);
    }

    std::string ToString();

private:
    using ObserverList = std::list<std::shared_ptr<RasterDrawOpObserver>>;

    SkISize                         frame_size_;
    std::shared_ptr<ContainerLayer> root_layer_;
    ObserverList                    raster_draw_op_observers_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_LAYERS_LAYERTREE_H
