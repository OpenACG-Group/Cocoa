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

#include "Core/Errors.h"
#include "Glamor/Glamor.h"
#include "Glamor/Layers/TextureLayer.h"
#include "Glamor/Texture.h"
#include "Glamor/TextureManager.h"
GLAMOR_NAMESPACE_BEGIN

TextureLayer::TextureLayer(int64_t texture_id,
                           const SkPoint& offset,
                           const SkISize& size,
                           const SkSamplingOptions& sampling)
    : texture_id_(texture_id)
    , offset_(offset)
    , size_(size)
    , sampling_options_(sampling)
{
}

void TextureLayer::Preroll(PrerollContext *context, const SkMatrix& matrix)
{
    auto rect = SkRect::MakeWH(static_cast<SkScalar>(size_.width()),
                               static_cast<SkScalar>(size_.height()))
                .makeOffset(offset_);
    SetPaintBounds(rect);
}

void TextureLayer::Paint(PaintContext *context) const
{
    SkAutoCanvasRestore restore(context->multiplexer_canvas, true);
    context->multiplexer_canvas->translate(offset_.x(), offset_.y());

    TextureManager::ScopedTextureAcquire acquire(*context->texture_manager, texture_id_);
    Texture *texture = acquire.Get();
    CHECK(texture);

    SkImageInfo image_info = texture->GetImageInfo();
    if (image_info.width() == size_.width() && image_info.height() == size_.height())
    {
        // No rescales are needed
        context->multiplexer_canvas->drawImage(texture->GetImage(),
                                               offset_.x(),
                                               offset_.y(),
                                               sampling_options_,
                                               context->GetCurrentPaintPtr());
    }
    else
    {
        auto rect = SkRect::MakeWH(static_cast<SkScalar>(size_.width()),
                                   static_cast<SkScalar>(size_.height()));
        rect = rect.makeOffset(offset_);

        context->multiplexer_canvas->drawImageRect(texture->GetImage(),
                                                   rect,
                                                   sampling_options_,
                                                   context->GetCurrentPaintPtr());
    }

    context->has_gpu_retained_resource = texture->IsHWComposeTexture();
}

GLAMOR_NAMESPACE_END
