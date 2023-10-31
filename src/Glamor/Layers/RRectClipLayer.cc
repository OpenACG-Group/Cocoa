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

#include "Glamor/Layers/RRectClipLayer.h"
GLAMOR_NAMESPACE_BEGIN

RRectClipLayer::RRectClipLayer(const SkRRect& rrect, bool AA)
    : ClippingLayerBase(ContainerType::kRRectClip, rrect)
    , perform_anti_alias_(AA)
{
}

ContainerLayer::ContainerAttributeChanged
RRectClipLayer::OnContainerDiffUpdateAttributes(const std::shared_ptr<ContainerLayer>& other)
{
    CHECK(other->GetContainerType() == ContainerType::kRRectClip);
    auto layer = std::static_pointer_cast<RRectClipLayer>(other);
    if (layer->perform_anti_alias_ == perform_anti_alias_ &&
        layer->GetClipShape() == GetClipShape())
    {
        return ContainerAttributeChanged::kNo;
    }
    else
    {
        return ContainerAttributeChanged::kYes;
    }
}

SkRect RRectClipLayer::OnGetClipShapeBounds() const
{
    return GetClipShape().rect();
}

void RRectClipLayer::OnApplyClipShape(const SkRRect& shape, PaintContext *ctx) const
{
    CHECK(ctx && ctx->multiplexer_canvas);
    ctx->multiplexer_canvas->clipRRect(GetClipShape(), perform_anti_alias_);
}

void RRectClipLayer::ToString(std::ostream& os)
{
    SkRRect shape = GetClipShape();
    SkRect bounds = shape.rect();

#define FLATTEN(v)                           \
    shape.radii(SkRRect::k##v##_Corner).x(), \
    shape.radii(SkRRect::k##v##_Corner).y()

    os << fmt::format("(round-rect-clip#{}:{}", GetUniqueId(), GetGenerationId())
       << fmt::format(" '(bounds {} {} {} {})", bounds.x(), bounds.y(),
                      bounds.width(), bounds.height())
       << fmt::format(" '(radii {} {} {} {} {} {} {} {})",
                      FLATTEN(UpperLeft), FLATTEN(UpperRight),
                      FLATTEN(LowerRight), FLATTEN(LowerLeft))
       << fmt::format(" '(antialias {})", perform_anti_alias_)
       << ' ';
#undef FLATTEN

    ChildrenToString(os);
    os << ')';
}

GLAMOR_NAMESPACE_END
