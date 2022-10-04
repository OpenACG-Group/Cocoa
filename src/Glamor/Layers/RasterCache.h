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

#ifndef COCOA_GLAMOR_LAYERS_RASTERCACHE_H
#define COCOA_GLAMOR_LAYERS_RASTERCACHE_H

#include <unordered_map>

#include "include/gpu/GrDirectContext.h"

#include "Core/Errors.h"
#include "Glamor/Glamor.h"
#include "Glamor/Layers/RasterCacheKey.h"
#include "Glamor/GraphicsResourcesTrackable.h"
GLAMOR_NAMESPACE_BEGIN

class RasterCacheItem
{
public:
    enum Type
    {
        kEmpty,
        kImageSnapshot
    };

    RasterCacheItem()
        : type_(Type::kEmpty) {}

    explicit RasterCacheItem(const sk_sp<SkImage>& image)
        : type_(Type::kImageSnapshot), image_snapshot_(image) {}

    g_nodiscard g_inline Type GetType() const {
        return type_;
    }

    g_nodiscard g_inline sk_sp<SkImage> GetImageSnapshot() const {
        CHECK(type_ == Type::kImageSnapshot);
        return image_snapshot_;
    }

private:
    Type                type_;
    sk_sp<SkImage>      image_snapshot_;
};

class RasterCache : public GraphicsResourcesTrackable
{
public:
    constexpr static int kPictureCacheThreshold = 15;
    constexpr static uint64_t kPictureTraceInfoOverdue = 40;

    explicit RasterCache(GrDirectContext *direct_context = nullptr)
        : direct_context_(direct_context)
        , frame_counter_(0) {}

    g_nodiscard g_inline bool HasDirectContext() const {
        return direct_context_;
    }

    g_nodiscard g_inline GrDirectContext *GetDirectContext() const {
        CHECK(direct_context_);
        return direct_context_;
    }

    g_private_api void IncreaseFrameCount();

    /**
     * Delete all the tracing infos and cached images to relieve the graphics
     * memory pressure.
     */
    void PurgeAllCaches();

    /**
     * Mark that the `picture` is not cached and will be rasterized in current frame.
     * This method also purges the overdue tracing infos automatically.
     *
     * @return  Ture if the `picture` can be cached;
     *          otherwise, return false.
     */
    bool MarkPictureUsedInCurrentFrame(const sk_sp<SkPicture>& picture);

    std::optional<RasterCacheItem> FindCacheItem(const RasterCacheKey& key);

    /**
     * Explicitly generate a cache for the specified picture.
     * Offscreen rasterize will be performed to generate the cache item, and the
     * color format of the generated cache image is up to `format_hint_surface`.
     *
     * A cache is overdue when its corresponding picture ID cannot be found in
     * the tracing infos of pictures anymore. This method also purges the overdue
     * caches automatically.
     */
    bool GeneratePictureCache(const sk_sp<SkPicture>& picture,
                              const SkMatrix& matrix,
                              SkSurface *format_hint_surface);

    void Trace(Tracer *tracer) noexcept override;

private:
    sk_sp<SkSurface> CreateSurface(SkISize size, SkSurface *format_hint_surface);
    void PurgeOverduePictureTracingInfo();
    void PurgeOverduePictureCaches();

    GrDirectContext *direct_context_;
    RasterCacheKey::Map<RasterCacheItem> cache_map_;
    uint64_t frame_counter_;

    struct PictureTraceInfo
    {
        uint64_t first_frame;
        uint64_t last_frame;
        uint64_t use_count;
    };
    std::unordered_map<uint64_t, PictureTraceInfo> picture_use_tracing_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_LAYERS_RASTERCACHE_H
