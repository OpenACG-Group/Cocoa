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
    for (const Shared<Layer>& layer : child_layers_)
    {
        // ContainerLayer doesn't have any transformations, so applying `matrix` directly
        // to child layer is reasonable.
        layer->Preroll(context, matrix);

        // The dirty boundary of a ContainerLayer is just the union of all its
        // children's dirty boundaries.
        child_paint_bounds->join(layer->GetPaintBounds());
    }
}

void ContainerLayer::Paint(PaintContext *context) const
{
    PaintChildren(context);
}

void ContainerLayer::PaintChildren(PaintContext *context) const
{
    // Iterate each child layer and paint them respectively
    for (const Shared<Layer>& layer : child_layers_)
    {
        if (layer->NeedsPainting(context))
            layer->Paint(context);
    }
}

void ContainerLayer::ChildrenToString(std::ostream& out)
{
    bool is_first_child = true;
    for (const Shared<Layer>& layer : child_layers_)
    {
        if (is_first_child)
            is_first_child = false;
        else
            out << ' ';
        layer->ToString(out);
    }
}

GLAMOR_NAMESPACE_END
