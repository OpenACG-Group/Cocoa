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

#include "fmt/format.h"

#include "Glamor/Layers/OpacityLayer.h"
GLAMOR_NAMESPACE_BEGIN

void OpacityLayer::Preroll(PrerollContext *context, const SkMatrix& matrix)
{
    SkRect child_paint_bounds = SkRect::MakeEmpty();
    PrerollChildren(context, matrix, &child_paint_bounds);

    SetPaintBounds(child_paint_bounds);
}

void OpacityLayer::Paint(PaintContext *context) const
{
    SkCanvas *canvas = context->multiplexer_canvas;
    CHECK(canvas);

    SkRect child_bounds = GetPaintBounds();

    SkAutoCanvasRestore scoped_restore(canvas, false);
    canvas->saveLayerAlpha(&child_bounds, alpha_);

    PaintChildren(context);
}

void OpacityLayer::ToString(std::ostream& out)
{
    out << fmt::format("(opacity '(alpha {})", alpha_);

    if (GetChildrenCount() > 0)
    {
        out << ' ';
        ChildrenToString(out);
    }
    out << ')';
}

GLAMOR_NAMESPACE_END
