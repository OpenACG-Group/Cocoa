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

#include "Glamor/Layers/RectClipLayer.h"
GLAMOR_NAMESPACE_BEGIN

RectClipLayer::RectClipLayer(const SkRect &rect, bool AA)
    : ClippingLayerBase(ContainerType::kRectClip, rect)
    , perform_anti_alias_(AA)
{
}

ContainerLayer::ContainerAttributeChanged
RectClipLayer::OnContainerDiffUpdateAttributes(const std::shared_ptr<ContainerLayer>& other)
{
    CHECK(other->GetContainerType() == ContainerType::kRectClip);
    auto layer = std::static_pointer_cast<RectClipLayer>(other);
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

void RectClipLayer::OnApplyClipShape(const SkRect& shape, PaintContext *ctx) const
{
    CHECK(ctx && ctx->multiplexer_canvas);
    ctx->multiplexer_canvas->clipRect(shape, perform_anti_alias_);
}

SkRect RectClipLayer::OnGetClipShapeBounds() const
{
    return GetClipShape();
}

void RectClipLayer::ToString(std::ostream& os)
{
    SkRect bounds = GetClipShape();

    os << fmt::format("(rect-clip#{}:{}", GetUniqueId(), GetGenerationId())
       << fmt::format(" '(bounds {} {} {} {})", bounds.x(), bounds.y(),
                                                bounds.width(), bounds.height())
       << fmt::format(" '(antialias {})", perform_anti_alias_)
       << ' ';

    ChildrenToString(os);
    os << ')';
}

GLAMOR_NAMESPACE_END
