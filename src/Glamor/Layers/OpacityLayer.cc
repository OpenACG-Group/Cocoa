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
#include "Glamor/Layers/LayerGenerationCache.h"
GLAMOR_NAMESPACE_BEGIN

OpacityLayer::OpacityLayer(SkAlpha alpha)
    : ContainerLayer(ContainerType::kOpacity)
    , alpha_(alpha)
{
}

ContainerLayer::ContainerAttributeChanged
OpacityLayer::OnContainerDiffUpdateAttributes(const std::shared_ptr<ContainerLayer>& other)
{
    CHECK(other->GetContainerType() == ContainerType::kOpacity);
    auto layer = std::static_pointer_cast<OpacityLayer>(other);
    return (layer->alpha_ == alpha_ ? ContainerAttributeChanged::kNo
                                    : ContainerAttributeChanged::kYes);
}

void OpacityLayer::Preroll(PrerollContext *context, const SkMatrix& matrix)
{
    SkRect child_paint_bounds = SkRect::MakeEmpty();
    PrerollChildren(context, matrix, &child_paint_bounds);

    SetPaintBounds(child_paint_bounds);
}

void OpacityLayer::Paint(PaintContext *context)
{
    SkCanvas *canvas = context->multiplexer_canvas;
    CHECK(canvas);

    if (context->cache->TryDrawCacheImageSnapshot(this, context))
        return;

    SkRect child_bounds = GetPaintBounds();

    SkAutoCanvasRestore scoped_restore(canvas, false);
    canvas->saveLayerAlpha(&child_bounds, alpha_);

    PaintChildren(context);
}

void OpacityLayer::ToString(std::ostream& out)
{
    out << fmt::format("(opacity#{}:{} '(alpha {})", GetUniqueId(), GetGenerationId(), alpha_);
    if (GetChildrenCount() > 0)
    {
        out << ' ';
        ChildrenToString(out);
    }
    out << ')';
}

GLAMOR_NAMESPACE_END
