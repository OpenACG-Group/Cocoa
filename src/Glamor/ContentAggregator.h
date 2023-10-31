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

#ifndef COCOA_GLAMOR_BLENDER_H
#define COCOA_GLAMOR_BLENDER_H

#include <optional>
#include <vulkan/vulkan_core.h>

#include "include/core/SkImage.h"

#include "Core/Data.h"
#include "Glamor/Glamor.h"
#include "Glamor/PresentRemoteHandle.h"
#include "Glamor/RenderTarget.h"
#include "Glamor/GraphicsResourcesTrackable.h"
#include "Glamor/Layers/LayerGenerationCache.h"

class SkSurface;

GLAMOR_NAMESPACE_BEGIN

class Surface;

class ContentAggregator;
class LayerTree;
class GProfiler;

#define GLOP_CONTENTAGGREGATOR_DISPOSE                            1
#define GLOP_CONTENTAGGREGATOR_UPDATE                             2
#define GLOP_CONTENTAGGREGATOR_CAPTURE_NEXT_FRAME_AS_PICTURE      8
#define GLOP_CONTENTAGGREGATOR_PURGE_RASTER_CACHE_RESOURCES       9
#define GLOP_CONTENTAGGREGATOR_IMPORT_GPU_SEMAPHORE_FROM_FD       10
#define GLOP_CONTENTAGGREGATOR_DELETE_IMPORTED_GPU_SEMAPHORE      11
#define GLOP_CONTENTAGGREGATOR_IMPORT_GPU_SKSURFACE               12
#define GLOP_CONTENTAGGREGATOR_DELETE_IMPORTED_GPU_SKSURFACE      13

#define GLSI_CONTENTAGGREGATOR_PICTURE_CAPTURED                   8

class ContentAggregator : public PresentRemoteHandle,
                          public GraphicsResourcesTrackable
{
public:
    enum class FrameScheduleState
    {
        // Completely idle and ready to schedule a new frame.
        kIdle,

        // There is a frame which has been begun and waiting to be submitted.
        // A new frame request has been sent to WSI layer, and we wait for WSI layer to notify
        // us when it is a good time to present a new frame.
        // The only way to change into this state is to call `Update` method.
        // If the scheduler has already been in `kPendingFrame` state, invocations of `Update`
        // have no effect.
        kPendingFrame,

        kPresented,
        kDisposed
    };

    enum class UpdateResult
    {
        kSuccess,
        kFrameDropped,
        kError
    };

    static std::shared_ptr<ContentAggregator> Make(
            const std::shared_ptr<Surface>& surface);

    explicit ContentAggregator(const std::shared_ptr<Surface>& surface);
    ~ContentAggregator() override;

    g_nodiscard std::shared_ptr<Surface> GetOutputSurface() const {
        return GetSurfaceChecked();
    }

    g_nodiscard const std::shared_ptr<LayerTree>& GetLayerTree() const {
        return layer_tree_;
    }

    /**
     * The profiler is associated with the blender uniquely when the blender
     * is created. It will NOT be removed or changed during the lifetime
     * of blender.
     * It is always safe to use the profiler after the corresponding blender
     * has been destroyed.
     */
    g_sync_api g_nodiscard const std::shared_ptr<GProfiler>&
    GetAttachedProfiler() const {
        return gfx_profiler_;
    }

    g_nodiscard RenderTarget::RenderDevice GetRenderDeviceType() const;
    g_nodiscard int32_t GetWidth() const;
    g_nodiscard int32_t GetHeight() const;
    g_nodiscard SkColorInfo GetOutputColorInfo() const;

    g_async_api UpdateResult Update(const std::shared_ptr<LayerTree>& layer_tree);
    g_async_api int32_t CaptureNextFrameAsPicture();
    g_async_api void Dispose();

    g_async_api void PurgeRasterCacheResources();

    using ImportedResourcesId = int64_t;

    g_async_api ImportedResourcesId ImportGpuSemaphoreFromFd(int32_t fd, bool auto_close);
    g_async_api void DeleteImportedGpuSemaphore(ImportedResourcesId id);

    g_async_api ImportedResourcesId ImportGpuSkSurface(const SkiaGpuContextOwner::ExportedSkSurfaceInfo& info);
    g_async_api void DeleteImportedGpuSkSurface(ImportedResourcesId id);

    g_private_api VkSemaphore GetImportedGpuSemaphore(ImportedResourcesId id);
    g_private_api SkSurface *GetImportedSkSurface(ImportedResourcesId id);

    void Trace(GraphicsResourcesTrackable::Tracer *tracer) noexcept override;

private:
    void SurfaceResizeSlot(int32_t width, int32_t height);
    void SurfaceFrameSlot();

    std::shared_ptr<HWComposeSwapchain> TryGetSwapchain();

    std::shared_ptr<Surface> GetSurfaceChecked() const;

    bool                           disposed_;
    uint32_t                       surface_resize_slot_id_;
    uint32_t                       surface_frame_slot_id_;
    std::weak_ptr<Surface>         weak_surface_;
    std::shared_ptr<LayerTree>     layer_tree_;
    SkIRect                        current_dirty_rect_;
    FrameScheduleState             frame_schedule_state_;
    std::unique_ptr<LayerGenerationCache>
                                   layer_generation_cache_;
    std::shared_ptr<GProfiler>     gfx_profiler_;

    bool                           should_capture_next_frame_;
    int32_t                        capture_next_frame_serial_;

    struct ImportedResourceEntry
    {
        enum Type
        {
            kSemaphore,
            kSkSurface
        };
        Type type;
        VkSemaphore semaphore;
        sk_sp<SkSurface> surface;
    };

    using ImportedResourcesIdMap =
            std::unordered_map<ImportedResourcesId, ImportedResourceEntry>;
    ImportedResourcesIdMap         imported_resources_ids_;
    int64_t                        imported_resources_ids_cnt_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_BLENDER_H
