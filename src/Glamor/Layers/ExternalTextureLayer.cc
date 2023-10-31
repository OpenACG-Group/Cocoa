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

#include "include/core/SkImage.h"

#include "fmt/format.h"

#include "Glamor/Layers/ExternalTextureLayer.h"
GLAMOR_NAMESPACE_BEGIN

ExternalTextureLayer::ExternalTextureLayer(std::unique_ptr<Accessor> frame_accessor,
                                           const SkPoint& offset,
                                           const SkISize& size,
                                           const SkSamplingOptions& sampling)
        : Layer(Type::kExternalTexture)
        , frame_accessor_(std::move(frame_accessor))
        , offset_(offset)
        , scale_size_(size)
        , scale_sampling_(sampling)
{
}

void ExternalTextureLayer::DiffUpdate(const std::shared_ptr<Layer>& other)
{
    CHECK(other->GetType() == Type::kExternalTexture);
    auto layer = std::static_pointer_cast<ExternalTextureLayer>(other);

    frame_accessor_ = std::move(layer->frame_accessor_);
    offset_ = layer->offset_;
    scale_size_ = layer->scale_size_;
    scale_sampling_ = layer->scale_sampling_;

    // There is no way to know whether the external texture is
    // changed, so we should assume that the external texture
    // changes every frame.
    IncreaseGenerationId();
}

void ExternalTextureLayer::Preroll(PrerollContext *context, const SkMatrix& matrix)
{
    auto rect = SkRect::MakeWH(static_cast<SkScalar>(scale_size_.width()),
                               static_cast<SkScalar>(scale_size_.height()))
            .makeOffset(offset_);
    SetPaintBounds(rect);

    frame_accessor_->Prefetch();
}

void ExternalTextureLayer::Paint(PaintContext *context)
{
    sk_sp<SkImage> texture = frame_accessor_->Acquire(context->gr_context);
    if (!texture)
    {
        frame_accessor_->Release();
        // TODO(sora): Filling a solid-color background or draw a chessboard
        return;
    }

    SkImageInfo image_info = texture->imageInfo();
    if (image_info.width() == scale_size_.width()
        && image_info.height() == scale_size_.height())
    {
        // No rescaling are needed
        context->multiplexer_canvas->drawImage(texture,
                                               offset_.x(),
                                               offset_.y(),
                                               scale_sampling_,
                                               context->GetCurrentPaintPtr());
    }
    else
    {
        auto rect = SkRect::MakeWH(static_cast<SkScalar>(scale_size_.width()),
                                   static_cast<SkScalar>(scale_size_.height()));
        rect = rect.makeOffset(offset_);

        context->multiplexer_canvas->drawImageRect(texture,
                                                   rect,
                                                   scale_sampling_,
                                                   context->GetCurrentPaintPtr());
    }

    frame_accessor_->Release();

    context->resource_usage_flags |= PaintContext::kExternalTexture_ResourceUsage;
    if (frame_accessor_->IsGpuBackedTexture(context->gr_context))
        context->resource_usage_flags |= PaintContext::kGpu_ResourceUsage;
}

void ExternalTextureLayer::ToString(std::ostream& out)
{
    out << fmt::format("(external-texture#{}:{} '(size {} {}) '(offset {} {}))",
                       GetUniqueId(),
                       GetGenerationId(),
                       scale_size_.width(), scale_size_.height(),
                       offset_.x(), offset_.y());
}

GLAMOR_NAMESPACE_END
