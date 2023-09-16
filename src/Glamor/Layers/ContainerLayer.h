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

    g_inline void AppendChildLayer(const std::shared_ptr<Layer>& layer) {
        auto itr = std::find(child_layers_.begin(), child_layers_.end(), layer);
        if (itr == child_layers_.end())
            child_layers_.push_back(layer);
    }

    g_inline void RemoveChildLayer(const std::shared_ptr<Layer>& layer) {
        auto itr = std::find(child_layers_.begin(), child_layers_.end(), layer);
        if (itr != child_layers_.end())
            child_layers_.erase(itr);
    }

    g_nodiscard g_inline size_t GetChildrenCount() const {
        return child_layers_.size();
    }

    void Preroll(PrerollContext *context, const SkMatrix &matrix) override;
    void Paint(PaintContext *context) const override;

protected:
    void PrerollChildren(PrerollContext *context, const SkMatrix& matrix, SkRect *child_paint_bounds);
    void PaintChildren(PaintContext *context) const;
    void ChildrenToString(std::ostream& out);

private:
    std::list<std::shared_ptr<Layer>> child_layers_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_LAYERS_CONTAINERLAYER_H
