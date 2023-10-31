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

#include "include/core/SkPathMeasure.h"
#include "fmt/format.h"

#include "Glamor/Layers/PathClipLayer.h"
GLAMOR_NAMESPACE_BEGIN

PathClipLayer::PathClipLayer(const SkPath &path, SkClipOp op, bool AA)
    : ClippingLayerBase(ContainerType::kPathClip, path)
    , clip_op_(op)
    , perform_anti_alias_(AA)
{
}

ContainerLayer::ContainerAttributeChanged
PathClipLayer::OnContainerDiffUpdateAttributes(const std::shared_ptr<ContainerLayer>& other)
{
    CHECK(other->GetContainerType() == ContainerType::kPathClip);
    auto layer = std::static_pointer_cast<PathClipLayer>(other);
    if (layer->clip_op_ == clip_op_ &&
        layer->perform_anti_alias_ == perform_anti_alias_ &&
        layer->GetClipShape() == GetClipShape())
    {
        return ContainerAttributeChanged::kNo;
    }
    else
    {
        return ContainerAttributeChanged::kYes;
    }
}

void PathClipLayer::OnApplyClipShape(const SkPath &shape, PaintContext *ctx) const
{
    CHECK(ctx && ctx->multiplexer_canvas);
    ctx->multiplexer_canvas->clipPath(shape, clip_op_, perform_anti_alias_);
}

SkRect PathClipLayer::OnGetClipShapeBounds() const
{
    return GetClipShape().computeTightBounds();
}

void PathClipLayer::ToString(std::ostream &out)
{
    SkPath shape = GetClipShape();

    out << fmt::format("(path-clip#{}:{}", GetUniqueId(), GetGenerationId())
        << fmt::format(" '(op {})", clip_op_ == SkClipOp::kIntersect
                                    ? "Intersect" : "Difference")
        << fmt::format(" '(antialias {})", perform_anti_alias_)
        << ' ';

    ChildrenToString(out);
    out << ')';
}

GLAMOR_NAMESPACE_END
