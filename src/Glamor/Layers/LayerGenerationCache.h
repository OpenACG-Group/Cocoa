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

#ifndef COCOA_GLAMOR_LAYER_LAYERGENERATIONCACHE_H
#define COCOA_GLAMOR_LAYER_LAYERGENERATIONCACHE_H

#include <unordered_map>

#include "include/core/SkImage.h"
#include "include/core/SkPicture.h"

#include "Glamor/Glamor.h"
#include "Glamor/SkiaGpuContextOwner.h"
#include "Glamor/GraphicsResourcesTrackable.h"
#include "Glamor/Layers/Layer.h"
GLAMOR_NAMESPACE_BEGIN

class LayerGenerationCache : public GraphicsResourcesTrackable
{
public:
    constexpr static uint32_t kMaxPictureGenerationStableCount = 32;
    constexpr static uint32_t kMaxImageFilterGenerationStableCount = 16;
    constexpr static uint32_t kMaxOpacityGenerationStableCount = 24;

    static uint32_t GetLayerGenerationStableCountThreshold(Layer *layer);

    explicit LayerGenerationCache(std::shared_ptr<SkiaGpuContextOwner> gpu_context);
    ~LayerGenerationCache() override = default;

    void BeginFrame();
    void EndFrame();

    void PurgeCacheResources(bool reset_recordings);

    enum class CacheState
    {
        kNotCachable,
        kRecording,
        kHasCached,
        kJustCached,
        kRenderError
    };

    /**
     * Update the cache recordings. Then if the corresponding cache exists and
     * is valid, draw the cached image-snapshot, using attributes set in `paint_context`.
     * In that case, returns true; otherwise, returns false and keeps the canvas untouched.
     */
    bool TryDrawCacheImageSnapshot(Layer *layer, Layer::PaintContext *paint_context);

    void PrintCacheStat(const std::function<void(std::string)>& line_printer);

    void Trace(Tracer *tracer) noexcept override;

private:
    CacheState UpdateCacheRecording(Layer *layer, Layer::PaintContext *paint_context);
    sk_sp<SkImage> TakeLayerImageSnapshot(Layer *layer, Layer::PaintContext *paint_context);

    using LayerUniqueID = uint64_t;
    using LayerGeneration = uint64_t;

    struct CacheRecordingEntry
    {
        Layer::Type     layer_type;
        const char     *layer_typename;
        LayerUniqueID   layer_id;
        LayerGeneration layer_generation;
        uint64_t        generation_stable_count;
        bool            evicted;
        sk_sp<SkImage>  image_snapshot;
    };
    using CacheRecordingMap = std::unordered_map<LayerUniqueID, CacheRecordingEntry>;

    std::shared_ptr<SkiaGpuContextOwner>    gpu_context_owner_;
    CacheRecordingMap                       cache_recording_map_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_LAYER_LAYERGENERATIONCACHE_H
