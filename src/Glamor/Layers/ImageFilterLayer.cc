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

#include "include/core/SkImageFilter.h"
#include "fmt/format.h"

#include "Core/Errors.h"
#include "Glamor/Layers/ImageFilterLayer.h"
#include "Glamor/Layers/LayerGenerationCache.h"
GLAMOR_NAMESPACE_BEGIN

ImageFilterLayer::ImageFilterLayer(const sk_sp<SkImageFilter>& filter)
    : ContainerLayer(ContainerType::kImageFilter)
    , filter_(filter)
{
}

ContainerLayer::ContainerAttributeChanged
ImageFilterLayer::OnContainerDiffUpdateAttributes(const std::shared_ptr<ContainerLayer>& other)
{
    CHECK(other->GetContainerType() == ContainerType::kImageFilter);
    auto layer = std::static_pointer_cast<ImageFilterLayer>(other);
    if (layer->filter_ == filter_)
        return ContainerAttributeChanged::kNo;
    else
    {
        filter_ = layer->filter_;
        return ContainerAttributeChanged::kYes;
    }
}

void ImageFilterLayer::Preroll(PrerollContext *context, const SkMatrix& matrix)
{
    CHECK(filter_);

    SkRect child_paint_bounds = SkRect::MakeEmpty();
    PrerollChildren(context, matrix, &child_paint_bounds);

    SkIRect filter_bounds = filter_->filterBounds(child_paint_bounds.roundOut(),
                                                  SkMatrix::I(),
                                                  SkImageFilter::kForward_MapDirection);

    SetPaintBounds(SkRect::Make(filter_bounds));
}

void ImageFilterLayer::Paint(PaintContext *context)
{
    SkCanvas *canvas = context->multiplexer_canvas;
    CHECK(canvas);

    if (context->cache->TryDrawCacheImageSnapshot(this, context))
        return;

    // Graphics state stored in `PaintContext::paints_stack` can be overwritten
    // (consider there are multiple container layers linked serially, and each of
    // them will change the graphics states stored in the paints stack),
    // so we must use `SkCanvas::saveLayer` to apply the image filter settings.
    {
        ScopedPaintMutator mutator(context, [this](SkPaint& paint) {
            paint.setImageFilter(this->filter_);
        });
        canvas->saveLayer(GetPaintBounds(), context->GetCurrentPaintPtr());
    }

    PaintChildren(context);

    canvas->restore();
}

void ImageFilterLayer::ToString(std::ostream& out)
{
    out << fmt::format("(imagefilter#{}:{} '(typename \"{}\")",
                       GetUniqueId(), GetGenerationId(), filter_->getTypeName());
    if (GetChildrenCount() > 0)
    {
        out << ' ';
        ChildrenToString(out);
    }
    out << ')';
}

GLAMOR_NAMESPACE_END
