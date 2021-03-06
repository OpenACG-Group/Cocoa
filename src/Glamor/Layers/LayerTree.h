#ifndef COCOA_GLAMOR_LAYERS_LAYERTREE_H
#define COCOA_GLAMOR_LAYERS_LAYERTREE_H

#include <list>

#include "include/core/SkSize.h"
#include "include/core/SkPicture.h"

#include "Glamor/Glamor.h"
#include "Glamor/Blender.h"
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

    g_inline void SetRootLayer(const Shared<ContainerLayer>& root) {
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

    g_inline void AppendObserver(const Shared<RasterDrawOpObserver>& observer) {
        auto itr = std::find(raster_draw_op_observers_.begin(),
                             raster_draw_op_observers_.end(),
                             observer);
        if (itr == raster_draw_op_observers_.end())
            raster_draw_op_observers_.push_back(observer);
    }

    g_nodiscard g_inline const auto& GetObservers() const {
        return raster_draw_op_observers_;
    }

    g_inline void RemoveObserver(const Shared<RasterDrawOpObserver>& observer) {
        auto itr = std::find(raster_draw_op_observers_.begin(),
                             raster_draw_op_observers_.end(),
                             observer);
        if (itr != raster_draw_op_observers_.end())
            raster_draw_op_observers_.erase(itr);
    }

private:
    SkISize frame_size_;
    Shared<ContainerLayer> root_layer_;
    std::list<Shared<RasterDrawOpObserver>> raster_draw_op_observers_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_LAYERS_LAYERTREE_H
