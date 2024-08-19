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

#ifndef COCOA_GLAMOR_GLAMOR_H
#define COCOA_GLAMOR_GLAMOR_H

#include <memory>
#include <vector>
#include <optional>
#include <mutex>

#include "Core/Project.h"
#include "Core/UniquePersistent.h"
#include "Core/EventLoop.h"

#include "Glamor/Types.h"

namespace cocoa {
class StandaloneThreadPool;
} // namespace cocoa

GLAMOR_NAMESPACE_BEGIN

class SkEventTracerImpl;

#define GLAMOR_BACKEND_WAYLAND      "wayland"

#define GLAMOR_SKIA_JIT_DEFAULT     true
#define GLAMOR_TILE_WIDTH_DEFAULT   200
#define GLAMOR_TILE_HEIGHT_DEFAULT  200
#define GLAMOR_WORKERS_CONCURRENCY  4
#define GLAMOR_PROFILER_RINGBUFFER_THRESHOLD_DEFAULT 32

enum class Backends
{
    kWayland,
    kDefault = kWayland
};

struct PresentGpuContextOptions
{
    bool video_decode_compatible = false;
};

class GProfiler;
class HWComposeContext;
class PresentThread;

class ContextOptions
{
public:
    ContextOptions();
    ContextOptions(const ContextOptions&) = default;
    ~ContextOptions() = default;

    g_nodiscard Backends GetBackend() const;
    g_nodiscard bool GetSkiaJIT() const;
    g_nodiscard int32_t GetTileWidth() const;
    g_nodiscard int32_t GetTileHeight() const;

    g_nodiscard uint32_t GetRenderWorkersConcurrencyCount() const {
        return render_workers_concurrency_count_;
    }

    g_nodiscard bool GetShowTileBoundaries() const {
        return show_tile_boundaries_;
    }

    g_nodiscard bool GetDisableHWComposePresent() const {
        return disable_hw_compose_present_;
    }

    // Use a specific window system integration backend
    void SetBackend(Backends backend);

    // Whether allow Skia to use JIT compilation to acceleration CPU-bound operations
    void SetSkiaJIT(bool allow);

    // Set tile dimensions if tile-based rendering is available
    void SetTileWidth(int32_t width);
    void SetTileHeight(int32_t height);

    // Set the number of RenderWorker threads to run in parallel.
    // RenderWorker threads are usually used to perform tile-based rendering.
    void SetRenderWorkersConcurrencyCount(uint32_t count) {
        render_workers_concurrency_count_ = count;
    }

    // Whether draw a grid of tiles if tile-based rendering is available
    void SetShowTileBoundaries(bool v) {
        show_tile_boundaries_ = v;
    }

    g_nodiscard bool GetDisableHWCompose() const {
        return disable_hw_compose_;
    }

    void SetDisableHWCompose(bool v) {
        disable_hw_compose_ = v;
    }

    g_nodiscard bool GetEnableVkDBG() const {
        return enable_vkdbg_;
    }

    void SetEnableVkDBG(bool v) {
        enable_vkdbg_ = v;
    }

    g_nodiscard std::vector<std::string>& GetVkDBGFilterSeverities() {
        return vkdbg_filter_severities_;
    }

    g_nodiscard std::vector<std::string>& GetVkDBGFilterLevels() {
        return vkdbg_filter_levels_;
    }

    void SetVkDBGFilterSeverities(const std::vector<std::string>& v) {
        vkdbg_filter_severities_ = v;
    }

    void SetVkDBGFilterLevels(const std::vector<std::string>& v) {
        vkdbg_filter_levels_ = v;
    }

    /**
     * Whether to disable onscreen rendering support of HWCompose.
     *
     * If HWCompose present is enabled, onscreen rendering is available
     * and the related Vulkan extensions are required for initiating
     * HWCompose context. No matter whether the created HWCompose context
     * will be used for onscreen or offscreen rendering, initiation will
     * fail if the required extensions are not available.
     *
     * But if HWCompose present is disabled, presentation extensions are
     * not required for initiating HWCompose context, and the created
     * HWCompose context can only be used for offscreen rendering.
     * That is, `HWComposeSwapchain` (for presentation) cannot be created,
     * and only `HWComposeOffscreen` can be created.
     */
    void SetDisableHWComposePresent(bool v) {
        disable_hw_compose_present_ = v;
    }

    void SetVkDeviceNameHint(const std::string& hint) {
        vk_devicename_hint_ = hint;
    }

    g_nodiscard const std::string& GetVkDeviceNameHint() {
        return vk_devicename_hint_;
    }

private:
    Backends    backend_;
    bool        skia_jit_;
    int32_t     tile_width_;
    int32_t     tile_height_;
    uint32_t    render_workers_concurrency_count_;
    bool        show_tile_boundaries_;
    bool        disable_hw_compose_;
    bool        disable_hw_compose_present_;

    bool        enable_vkdbg_;
    std::vector<std::string> vkdbg_filter_severities_;
    std::vector<std::string> vkdbg_filter_levels_;

    std::string vk_devicename_hint_;
};

class GlobalScope : public UniquePersistent<GlobalScope>
{
public:
    struct ApplicationInfo
    {
        using VersionTriple = std::tuple<int32_t, int32_t, int32_t>;

        ApplicationInfo() = default;
        ApplicationInfo(const std::string_view& name, VersionTriple triple)
                : name(name), version_triple(std::move(triple)) {}

        std::string         name;
        VersionTriple       version_triple;
    };

    GlobalScope(const ContextOptions& options, EventLoop *loop);
    ~GlobalScope();

    g_nodiscard ContextOptions& GetOptions();

    void SetApplicationInfo(const ApplicationInfo& info) {
        application_info_ = info;
    }

    bool StartPresentThread();
    void DisposePresentThread();

    g_nodiscard PresentThread *GetPresentThread() {
        return present_thread_.get();
    }

    const std::unique_ptr<StandaloneThreadPool>& GetRenderWorkersThreadPool();

    g_nodiscard SkEventTracerImpl *GetSkEventTracerImpl() {
        return skia_event_tracer_impl_;
    }

    g_nodiscard std::shared_ptr<HWComposeContext> GetHWComposeContext();

private:
    ContextOptions                          options_;
    ApplicationInfo                         application_info_;
    EventLoop                              *event_loop_;
    std::unique_ptr<StandaloneThreadPool>   render_workers_;
    SkEventTracerImpl                      *skia_event_tracer_impl_;
    std::mutex                              hw_compose_creation_lock_;
    bool                                    hw_compose_context_creation_failed_;
    bool                                    hw_compose_disabled_;
    std::shared_ptr<HWComposeContext>       hw_compose_context_;
    std::unique_ptr<PresentThread>          present_thread_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_GLAMOR_H
