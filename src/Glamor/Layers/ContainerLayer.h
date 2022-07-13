#ifndef COCOA_GLAMOR_LAYERS_CONTAINERLAYER_H
#define COCOA_GLAMOR_LAYERS_CONTAINERLAYER_H

#include <list>
#include <algorithm>

#include "Glamor/Layers/Layer.h"
GLAMOR_NAMESPACE_BEGIN

class ContainerLayer : public Layer
{
public:
    ContainerLayer() = default;
    ~ContainerLayer() override = default;

    g_inline void AppendChildLayer(const Shared<Layer>& layer) {
        auto itr = std::find(child_layers_.begin(), child_layers_.end(), layer);
        if (itr == child_layers_.end())
            child_layers_.push_back(layer);
    }

    g_inline void RemoveChildLayer(const Shared<Layer>& layer) {
        auto itr = std::find(child_layers_.begin(), child_layers_.end(), layer);
        if (itr != child_layers_.end())
            child_layers_.erase(itr);
    }

    void Preroll(PrerollContext *context, const SkMatrix &matrix) override;
    void Paint(PaintContext *context) const override;

protected:
    void PrerollChildren(PrerollContext *context, const SkMatrix& matrix, SkRect *child_paint_bounds);
    void PaintChildren(PaintContext *context) const;

private:
    std::list<Shared<Layer>>    child_layers_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_LAYERS_CONTAINERLAYER_H
