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

#include "Glamor/Layers/PictureLayer.h"
#include "Glamor/Layers/RasterCache.h"
GLAMOR_NAMESPACE_BEGIN

PictureLayer::PictureLayer(const SkPoint& offset,
                           bool auto_fast_clip,
                           const sk_sp<SkPicture>& picture)
    : sk_picture_(picture)
    , offset_(offset)
    , auto_fast_clip_(auto_fast_clip)
{
}

PictureLayer::~PictureLayer() = default;

void PictureLayer::Preroll(PrerollContext *context, const SkMatrix& matrix)
{
    SkRect bounds = sk_picture_->cullRect().makeOffset(offset_.x(), offset_.y());
    SetPaintBounds(bounds);
}

void PictureLayer::Paint(PaintContext *context) const
{
    SkCanvas *canvas = context->multiplexer_canvas;
    CHECK(canvas);

    SkAutoCanvasRestore canvas_restore(canvas, true);
    canvas->translate(offset_.x(), offset_.y());
    if (auto_fast_clip_)
        canvas->clipRect(sk_picture_->cullRect());

    // We do not care about the transformations applied on the picture
    // when querying cache, so we always use an identity matrix.
    auto key = RasterCacheKey(RasterCacheLayerId(sk_picture_->uniqueID()), SkMatrix::I());

    // Pictures which are not specified `auto_fast_clip` have an infinite boundary,
    // and they cannot be cached.
    sk_sp<SkImage> cached_image = nullptr;

    if (context->raster_cache && auto_fast_clip_)
    {
        bool should_cache = context->raster_cache->MarkPictureUsedInCurrentFrame(sk_picture_);

        auto maybe_item = context->raster_cache->FindCacheItem(key);
        if (!maybe_item && should_cache)
        {
            bool cached =
                context->raster_cache->GeneratePictureCache(sk_picture_,
                                                            SkMatrix::I(),
                                                            context->frame_surface);

            // TODO(sora): Report an error instead of assertion.
            CHECK(cached);
        }

        // Find the cache again because the cache item may be newly generated.
        if (!maybe_item)
            maybe_item = context->raster_cache->FindCacheItem(key);

        if (maybe_item)
        {
            CHECK(maybe_item->GetType() == RasterCacheItem::kImageSnapshot);
            cached_image = maybe_item->GetImageSnapshot();
        }
    }

    if (cached_image)
    {
        // If the picture is found in caches, it must have an `auto_fast_clip` attribute.
        SkRect cull_rect = sk_picture_->cullRect();
        canvas->drawImage(cached_image, cull_rect.x(), cull_rect.y(), {},
                          context->GetCurrentPaintPtr());
    }
    else
    {
        canvas->drawPicture(sk_picture_, nullptr, context->GetCurrentPaintPtr());
    }
}

void PictureLayer::ToString(std::ostream& out)
{
    SkRect bounds = sk_picture_->cullRect();
    out << fmt::format("(picture '(auto-fast-clipping {}) '(bounds {} {} {} {}) '(id {}) '(offset {} {}))",
                       auto_fast_clip_ ? 1 : 0,
                       bounds.x(), bounds.y(), bounds.width(), bounds.height(),
                       sk_picture_->uniqueID(), offset_.x(), offset_.y());
}

GLAMOR_NAMESPACE_END
