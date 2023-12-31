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

#include "Glamor/Layers/ContainerLayer.h"
GLAMOR_NAMESPACE_BEGIN

ContainerLayer::ContainerLayer(ContainerType container_type)
    : Layer(Type::kContainer)
    , container_type_(container_type)
{
}

bool ContainerLayer::IsComparableWith(Layer *other) const
{
    if (other->GetType() != Type::kContainer)
        return false;
    return static_cast<ContainerLayer*>(other)->container_type_ == container_type_;
}

void ContainerLayer::Preroll(PrerollContext *context, const SkMatrix& matrix)
{
    SkRect paint_bound = SkRect::MakeEmpty();
    PrerollChildren(context, matrix, &paint_bound);
    SetPaintBounds(paint_bound);
}

void ContainerLayer::PrerollChildren(PrerollContext *context,
                                     const SkMatrix& matrix,
                                     SkRect *child_paint_bounds)
{
    // Iterate each child layer and reroll them respectively
    for (const std::shared_ptr<Layer>& layer : child_layers_)
    {
        // ContainerLayer doesn't have any transformations, so applying `matrix` directly
        // to child layer is reasonable.
        layer->Preroll(context, matrix);

        // The dirty boundary of a ContainerLayer is just the union of all its
        // children's dirty boundaries.
        child_paint_bounds->join(layer->GetPaintBounds());
    }
}

void ContainerLayer::Paint(PaintContext *context)
{
    PaintChildren(context);
}

void ContainerLayer::PaintChildren(PaintContext *context) const
{
    // Iterate each child layer and paint them respectively
    for (const std::shared_ptr<Layer>& layer : child_layers_)
    {
        if (layer->NeedsPainting(context))
            layer->Paint(context);
    }
}

void ContainerLayer::DiffUpdate(const std::shared_ptr<Layer>& other)
{
    auto new_container = std::static_pointer_cast<ContainerLayer>(other);
    std::list<std::shared_ptr<Layer>>& new_children = new_container->child_layers_;
    std::list<std::shared_ptr<Layer>>& old_children = child_layers_;

    std::list<std::shared_ptr<Layer>> replace_children;

    // TODO(sora): Improve the comparison algorithm
    bool subtree_dirty = false;
    for (auto itr = new_children.begin(); itr != new_children.end(); itr++)
    {
        auto reusable_old_itr = std::find_if(old_children.begin(), old_children.end(),
                                             [&itr](const std::shared_ptr<Layer>& layer) {
            return layer->IsComparableWith(itr->get());
        });

        if (reusable_old_itr == old_children.end())
        {
            // No reusable child node is found.
            replace_children.emplace_back(*itr);
            continue;
        }

        std::shared_ptr<Layer> reusable_old_layer = *reusable_old_itr;
        uint64_t old_gen_id = reusable_old_layer->GetGenerationId();
        reusable_old_layer->DiffUpdate(*itr);
        subtree_dirty = subtree_dirty || (old_gen_id != reusable_old_layer->GetGenerationId());

        // Reuse the found node
        old_children.erase(reusable_old_itr);
        replace_children.emplace_back(std::move(reusable_old_layer));
    }

    child_layers_ = std::move(replace_children);

    auto attrs_changed = OnContainerDiffUpdateAttributes(new_container);
    if (subtree_dirty || attrs_changed == ContainerAttributeChanged::kYes)
        IncreaseGenerationId();
}

void ContainerLayer::ChildrenToString(std::ostream& out)
{
    bool is_first_child = true;
    for (const std::shared_ptr<Layer>& layer : child_layers_)
    {
        if (is_first_child)
            is_first_child = false;
        else
            out << ' ';
        layer->ToString(out);
    }
}

GLAMOR_NAMESPACE_END
