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

#include "include/core/SkPicture.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "fmt/format.h"

#include "Glamor/Layers/RasterCacheKey.h"
#include "Glamor/Layers/RasterCache.h"
#include "Glamor/Layers/Layer.h"
GLAMOR_NAMESPACE_BEGIN

namespace {

// boost::hash_combine to combine to hash values
size_t hash_combine(size_t lhs, size_t rhs)
{
    lhs ^= rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2);
    return lhs;
}

} // namespace anonymous

uint64_t RasterCacheLayerId::ComputeHashValue() const
{
    constexpr static std::size_t kHashCombineSeed = 0x66ccff;

    if (id_type_ == Type::kPicture)
        return picture_unique_id_;

    uint64_t result = kHashCombineSeed;
    for (const RasterCacheLayerId& id : child_ids_)
        result = hash_combine(result, id.GetHash());

    return result;
}

void RasterCache::Trace(Tracer *tracer) noexcept
{
    for (const auto& pair : cache_map_)
    {
        std::string annotation;
        std::optional<size_t> size;

        RasterCacheLayerId::Type type = pair.first.GetLayerId().GetType();
        if (type == RasterCacheLayerId::Type::kPicture)
        {
            annotation = fmt::format("RasterCache[Picture#{}]",
                                     pair.first.GetLayerId().GetPictureUniqueId());

            size = pair.second.GetImageSnapshot()->imageInfo().computeMinByteSize();
        }
        else if (type == RasterCacheLayerId::Type::kContainer)
        {
            annotation = fmt::format("RasterCache[Container#{}]",
                                     pair.first.GetLayerId().GetHash());

            // TODO(sora): cache for container layers
        }

        tracer->TraceResource(annotation,
                              TRACKABLE_TYPE_TEXTURE,
                              HasDirectContext() ? TRACKABLE_DEVICE_GPU : TRACKABLE_DEVICE_CPU,
                              TRACKABLE_OWNERSHIP_STRICT_OWNED,
                              pair.first.GetLayerId().GetHash(),
                              size);
    }
}

void RasterCache::PurgeAllCaches()
{
    picture_use_tracing_.clear();
    cache_map_.clear();
}

void RasterCache::IncreaseFrameCount()
{
    frame_counter_++;
}

sk_sp<SkSurface> RasterCache::CreateSurface(SkISize size,
                                            SkSurface *format_hint_surface)
{
    CHECK(format_hint_surface);
    SkColorInfo color_info = format_hint_surface->imageInfo().colorInfo();
    SkImageInfo image_info = SkImageInfo::Make(size, color_info);

    sk_sp<SkSurface> surface;
    if (direct_context_)
    {
        surface = SkSurfaces::RenderTarget(direct_context_, skgpu::Budgeted::kNo, image_info);
    }
    else
    {
        surface = SkSurfaces::Raster(image_info);
    }

    return surface;
}

bool RasterCache::GeneratePictureCache(const sk_sp<SkPicture>& picture,
                                       const SkMatrix& matrix,
                                       SkSurface *format_hint_surface)
{
    PurgeOverduePictureCaches();

    SkRect cull = picture->cullRect();
    SkIRect bounds = picture->cullRect().roundOut();
    if (picture->cullRect() == kGiantRect || !format_hint_surface)
        return false;

    sk_sp<SkSurface> surface = CreateSurface(bounds.size(), format_hint_surface);
    if (!surface)
        return false;
    SkCanvas *canvas = surface->getCanvas();
    CHECK(canvas);

    canvas->clipRect(SkRect::MakeWH(cull.width(), cull.height()));
    canvas->translate(-cull.x(), -cull.y());

    canvas->drawPicture(picture, nullptr, nullptr);

    sk_sp<SkImage> image_snapshot = surface->makeImageSnapshot();
    RasterCacheKey cache_key(RasterCacheLayerId(picture->uniqueID()), matrix);

    auto insert = cache_map_.try_emplace(cache_key, image_snapshot);
    return insert.second;
}

std::optional<RasterCacheItem> RasterCache::FindCacheItem(const RasterCacheKey& key)
{
    if (cache_map_.count(key) == 0)
        return std::nullopt;
    return cache_map_[key];
}

void RasterCache::PurgeOverduePictureTracingInfo()
{
    for (auto itr = picture_use_tracing_.begin();
         itr != picture_use_tracing_.end(); itr++)
    {
        PictureTraceInfo& info = itr->second;
        if (info.last_frame - frame_counter_ >= kPictureTraceInfoOverdue)
        {
            itr = picture_use_tracing_.erase(itr);
            if (itr == picture_use_tracing_.end())
                break;
        }
    }
}

void RasterCache::PurgeOverduePictureCaches()
{
    for (auto itr = cache_map_.begin(); itr != cache_map_.end(); itr++)
    {
        if (itr->first.GetLayerId().GetType() != RasterCacheLayerId::Type::kPicture)
            continue;

        uint64_t pict_id = itr->first.GetLayerId().GetPictureUniqueId();
        if (picture_use_tracing_.count(pict_id) == 0)
        {
            itr = cache_map_.erase(itr);
            if (itr == cache_map_.end())
                break;
        }
    }
}

bool RasterCache::MarkPictureUsedInCurrentFrame(const sk_sp<SkPicture>& picture)
{
    CHECK(picture);

    uint64_t unique_id = picture->uniqueID();
    if (picture_use_tracing_.count(unique_id) == 0)
    {
        picture_use_tracing_[unique_id] = {
            .first_frame = frame_counter_,
            .last_frame = frame_counter_,
            .use_count = 1
        };
    }
    else
    {
        picture_use_tracing_[unique_id].last_frame = frame_counter_;
        picture_use_tracing_[unique_id].use_count++;
    }

    PurgeOverduePictureTracingInfo();

    PictureTraceInfo& info = picture_use_tracing_[unique_id];
    return (info.use_count >= kPictureCacheThreshold);
}

GLAMOR_NAMESPACE_END
