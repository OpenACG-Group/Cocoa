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

#include "Core/Journal.h"
#include "Glamor/Layers/TransformLayer.h"
GLAMOR_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Glamor.Layers.TransformLayer)

TransformLayer::TransformLayer(const SkMatrix& transform)
    : ContainerLayer(ContainerType::kTransform)
    , transform_(transform)
{
    // If a transformation matrix is invalid, we print a message to indicate
    // that and use identity matrix as a fallback.
    if (!transform_.isFinite())
    {
        QLOG(LOG_ERROR, "TransformLayer is constructed with an invalid transformation matrix");
        transform_.setIdentity();
    }
}

ContainerLayer::ContainerAttributeChanged
TransformLayer::OnContainerDiffUpdateAttributes(const std::shared_ptr<ContainerLayer>& other)
{
    CHECK(other->GetContainerType() == ContainerType::kTransform);
    auto layer = std::static_pointer_cast<TransformLayer>(other);
    if (layer->transform_ == transform_)
        return ContainerAttributeChanged::kNo;
    else
    {
        transform_ = layer->transform_;
        return ContainerAttributeChanged::kYes;
    }
}

void TransformLayer::Preroll(PrerollContext *context, const SkMatrix& matrix)
{
    SkMatrix child_matrix;
    child_matrix.setConcat(matrix, transform_);

    SkRect previous_cull_rect = context->cull_rect;
    SkMatrix inverse_transform;

    // Perspective projections don't produce rectangles that are useful for
    // culling for some reason.
    if (!transform_.hasPerspective() && transform_.invert(&inverse_transform))
    {
        inverse_transform.mapRect(&context->cull_rect);
    }
    else
    {
        context->cull_rect = kGiantRect;
    }

    SkRect child_paint_bounds = SkRect::MakeEmpty();
    PrerollChildren(context, child_matrix, &child_paint_bounds);

    // Get transformed clipping rectangle
    transform_.mapRect(&child_paint_bounds);
    SetPaintBounds(child_paint_bounds);

    // Restore cull rectangle
    context->cull_rect = previous_cull_rect;
}

void TransformLayer::Paint(PaintContext *context)
{
    SkAutoCanvasRestore scopedRestore(context->multiplexer_canvas, true);
    context->multiplexer_canvas->concat(transform_);

    PaintChildren(context);
}

void TransformLayer::ToString(std::ostream& out)
{
#define M(v) transform_[SkMatrix::kM##v]
    out << fmt::format("(transform#{}:{}", GetUniqueId(), GetGenerationId())
        << fmt::format(" '(mat3x3 [[{}, {}, {}], [{}, {}, {}], [{}, {}, {}]]))",
                       M(ScaleX), M(SkewX), M(TransX),
                       M(SkewY), M(ScaleY), M(TransY),
                       M(Persp0), M(Persp1), M(Persp2));

    if (GetChildrenCount() > 0)
    {
        out << ' ';
        ChildrenToString(out);
    }
    out << ')';
#undef M
}

GLAMOR_NAMESPACE_END
