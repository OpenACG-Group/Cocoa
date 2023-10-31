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

#include "include/core/SkSurface.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"

#include "Glamor/Layers/LayerGenerationCache.h"
#include "Glamor/Layers/ContainerLayer.h"
GLAMOR_NAMESPACE_BEGIN

LayerGenerationCache::LayerGenerationCache(std::shared_ptr<SkiaGpuContextOwner> gpu_context)
    : gpu_context_owner_(std::move(gpu_context))
{
}

void LayerGenerationCache::BeginFrame()
{
    // To begin a new frame, all the tracked layers should be marked
    // with EVICTED state. When the ContentAggregator is visiting the
    // new layers, the EVICTED mark will be cleared. Finally, the layers
    // that still remain in the EVICTED state are dead layers.
    for (auto& [layer_id, layer_record_entry] : cache_recording_map_)
    {
        layer_record_entry.evicted = true;
    }
}

void LayerGenerationCache::EndFrame()
{
    // Sweep stage. The layers that still remain in the EVICTED state
    // are dead layers.
    auto itr = cache_recording_map_.begin();
    while (itr != cache_recording_map_.end())
    {
        if (itr->second.evicted)
        {
            // Destruct the cached resources
            itr->second.image_snapshot.reset();
            itr = cache_recording_map_.erase(itr);
        }
        else
        {
            itr++;
        }
    }
}

LayerGenerationCache::CacheState
LayerGenerationCache::UpdateCacheRecording(Layer *layer, Layer::PaintContext *paint_context)
{
    // To avoid nested cache generating.
    if (paint_context->is_generating_cache)
        return CacheState::kNotCachable;

    uint32_t stable_count_threshold = GetLayerGenerationStableCountThreshold(layer);
    if (stable_count_threshold == UINT32_MAX)
        return CacheState::kNotCachable;

    // If the layer has not been tracked yet, we just need to add it to
    // the tracking list.
    LayerUniqueID layer_id = layer->GetUniqueId();
    if (cache_recording_map_.count(layer_id) == 0)
    {
        cache_recording_map_[layer_id] = {
            .layer_type = layer->GetType(),
            .layer_typename = layer->GetLayerTypeName(),
            .layer_id = layer_id,
            .layer_generation = layer->GetGenerationId(),
            .generation_stable_count = 1,
            .evicted = false,
            .image_snapshot = nullptr
        };
        return CacheState::kRecording;
    }

    // If the layer has been tracked, update its tracking state.
    CacheRecordingEntry& record_entry = cache_recording_map_[layer_id];
    LayerGeneration layer_generation = layer->GetGenerationId();
    record_entry.evicted = false;
    if (record_entry.layer_generation == layer_generation)
        record_entry.generation_stable_count++;
    else
    {
        // When the generation of a layer changes, the cached image-snapshot
        // is invalidated, then it should be destructed as soon as possible.
        record_entry.image_snapshot.reset();

        record_entry.generation_stable_count = 0;
        record_entry.layer_generation = layer_generation;
        return CacheState::kRecording;
    }

    // The stable count has not exceeded the corresponding threshold yet,
    // which means the layer has not been cached and still cannot be cached.
    if (record_entry.generation_stable_count < stable_count_threshold)
        return CacheState::kRecording;

    // Cache is available, return it. Otherwise, generate a new cache.
    if (record_entry.image_snapshot)
        return CacheState::kHasCached;

    record_entry.image_snapshot = TakeLayerImageSnapshot(layer, paint_context);
    if (!record_entry.image_snapshot)
        return CacheState::kRenderError;

    return CacheState::kJustCached;
}

uint32_t LayerGenerationCache::GetLayerGenerationStableCountThreshold(Layer *layer)
{
    Layer::Type type = layer->GetType();
    if (type == Layer::Type::kPicture)
        return kMaxPictureGenerationStableCount;

    if (type == Layer::Type::kContainer)
    {
        ContainerLayer *container = static_cast<ContainerLayer*>(layer);
        ContainerLayer::ContainerType container_type = container->GetContainerType();
        if (container_type == ContainerLayer::ContainerType::kImageFilter)
            return kMaxImageFilterGenerationStableCount;
        if (container_type == ContainerLayer::ContainerType::kOpacity)
            return kMaxOpacityGenerationStableCount;
        else
            return UINT32_MAX;
    }

    return UINT32_MAX;
}

sk_sp<SkImage> LayerGenerationCache::TakeLayerImageSnapshot(Layer *layer, Layer::PaintContext *paint_context)
{
    GrDirectContext *direct_ctx = gpu_context_owner_
                                  ? gpu_context_owner_->GetSkiaGpuContext()
                                  : nullptr;
    CHECK(direct_ctx == paint_context->gr_context);

    SkRect layer_paint_bounds = layer->GetPaintBounds();
    SkImageInfo image_info = paint_context->frame_surface->imageInfo()
                            .makeDimensions(layer_paint_bounds.roundOut().size());

    sk_sp<SkSurface> surface;
    if (direct_ctx)
        surface = SkSurfaces::RenderTarget(direct_ctx, skgpu::Budgeted::kNo, image_info);
    else
        surface = SkSurfaces::Raster(image_info);

    if (!surface)
        return nullptr;

    SkCanvas *canvas = surface->getCanvas();
    canvas->translate(-layer_paint_bounds.left(), -layer_paint_bounds.top());

    // Create a subcontext for cache rendering. Only the cached subtree will be rendered.
    // Field `paint_context->paints_stack` is ignored when we perform the cache rendering,
    // because the paint effect will be applied when the cached image-snapshot is drawn.
    Layer::PaintContext sub_paint_context{
        .gr_context = direct_ctx,
        .is_generating_cache = true,
        .root_surface_transformation = SkMatrix::I(),
        .frame_surface = surface.get(),
        .frame_canvas = canvas,
        .multiplexer_canvas = canvas,
        .cull_rect = paint_context->cull_rect,
        .resource_usage_flags = Layer::PaintContext::kNone_ResourceUsage,
        .cache = this,
        .gpu_finished_semaphores = {}
    };

    layer->Paint(&sub_paint_context);

    auto& signal_semaphores = sub_paint_context.gpu_finished_semaphores;
    if (direct_ctx && !signal_semaphores.empty())
    {
        // The subtree requires us to signal some semaphores when the
        // rendering task is finished on GPU. This should have been done by
        // `ContentAggregator`, but as a result of creating a new `PaintContext`,
        // those semaphores will not be handled by `ContentAggregator`.
        GrFlushInfo info;
        info.fNumSemaphores = signal_semaphores.size();
        info.fSignalSemaphores = signal_semaphores.data();

        // Just flush the surface. `GrDirectContext::submit()` will be done
        // when the paint stage is finished by `ContentAggregator`.
        direct_ctx->flush(surface.get(), info, nullptr);
    }

    return surface->makeImageSnapshot();
}

bool LayerGenerationCache::TryDrawCacheImageSnapshot(Layer *layer, Layer::PaintContext *paint_context)
{
    CacheState cache_state = UpdateCacheRecording(layer, paint_context);
    if (cache_state != CacheState::kHasCached && cache_state != CacheState::kJustCached)
        return false;

    LayerUniqueID layer_id = layer->GetUniqueId();
    CHECK(cache_recording_map_.count(layer_id) > 0);
    sk_sp<SkImage> image_snapshot = cache_recording_map_[layer_id].image_snapshot;
    CHECK(image_snapshot);

    SkRect paint_bounds = layer->GetPaintBounds();
    SkSamplingOptions sampling_opt(SkFilterMode::kLinear);

    // `paint_context` specifies the SkPaint, which contains SkBlendMode, SkColorFilter,
    // SkImageFilter, and so on.
    paint_context->multiplexer_canvas->drawImage(
            image_snapshot, paint_bounds.left(), paint_bounds.top(),
            sampling_opt, paint_context->GetCurrentPaintPtr());

    if (image_snapshot->isTextureBacked())
        paint_context->resource_usage_flags |= Layer::PaintContext::kGpu_ResourceUsage;

    return true;
}

void LayerGenerationCache::PrintCacheStat(const std::function<void(std::string)> &line_printer)
{
    for (const auto& [layer_id, record] : cache_recording_map_)
    {
        std::string line = fmt::format(
                "Layer #{}:{} [typename: {}, stable_count: {}] ",
                record.layer_id,
                record.layer_generation,
                record.layer_typename,
                record.generation_stable_count);

        if (record.image_snapshot)
        {
            const char *image_snapshot_type = record.image_snapshot->isTextureBacked()
                    ? "GPU texture" : "Raster bitmap";
            size_t image_size;
            if (record.image_snapshot->isTextureBacked())
                image_size = record.image_snapshot->textureSize();
            else
            {
                SkPixmap pixmap;
                CHECK(record.image_snapshot->peekPixels(&pixmap));
                image_size = pixmap.computeByteSize();
            }
            line += fmt::format("<SkImage> {} [{} {:.2f}KiB]\n",
                                fmt::ptr(record.image_snapshot.get()), image_snapshot_type,
                                double(image_size) / 1024.0);
        }
        else
        {
            line += "<Recording>\n";
        }

        line_printer(std::move(line));
    }
}

void LayerGenerationCache::PurgeCacheResources(bool reset_recordings)
{
    for (auto& [layer_id, record] : cache_recording_map_)
        record.image_snapshot.reset();
    if (reset_recordings)
        cache_recording_map_.clear();
}

void LayerGenerationCache::Trace(Tracer *tracer) noexcept
{
    for (const auto& [layer_id, record] : cache_recording_map_)
    {
        if (!record.image_snapshot)
            continue;

        size_t texture_size;
        if (record.image_snapshot->isTextureBacked())
            texture_size = record.image_snapshot->textureSize();
        else
        {
            SkPixmap pixmap;
            CHECK(record.image_snapshot->peekPixels(&pixmap));
            texture_size = pixmap.computeByteSize();
        }

        tracer->TraceResource(
            fmt::format("Cache[Layer#{}:{}]", record.layer_id, record.layer_generation),
            TRACKABLE_TYPE_TEXTURE,
            record.image_snapshot ? TRACKABLE_DEVICE_GPU : TRACKABLE_DEVICE_CPU,
            TRACKABLE_OWNERSHIP_SHARED,
            TraceIdFromPointer(record.image_snapshot.get()),
            texture_size
        );
    }
}

GLAMOR_NAMESPACE_END
