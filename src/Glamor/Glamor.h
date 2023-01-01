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

#include "Core/Project.h"
#include "Core/UniquePersistent.h"
#include "Core/EventLoop.h"

#include "Glamor/Types.h"

namespace cocoa {
class StandaloneThreadPool;
} // namespace cocoa

GLAMOR_NAMESPACE_BEGIN

template<typename T>
using Shared = std::shared_ptr<T>;

template<typename T>
using Unique = std::unique_ptr<T>;

template<typename T>
using Weak = std::weak_ptr<T>;

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

class RenderHost;
class RenderClient;
class GpuThreadSharedObjectsCollector;
class GProfiler;

class ContextOptions
{
public:
    ContextOptions();
    ContextOptions(const ContextOptions&) = default;
    ~ContextOptions() = default;

    g_nodiscard Backends GetBackend() const;
    g_nodiscard bool GetSkiaJIT() const;
    g_nodiscard bool GetProfileRenderHostTransfer() const;
    g_nodiscard int32_t GetTileWidth() const;
    g_nodiscard int32_t GetTileHeight() const;

    g_nodiscard g_inline uint32_t GetRenderWorkersConcurrencyCount() const {
        return render_workers_concurrency_count_;
    }

    g_nodiscard g_inline bool GetShowTileBoundaries() const {
        return show_tile_boundaries_;
    }

    // Use a specific window system integration backend
    void SetBackend(Backends backend);

    // Whether allow Skia to use JIT compilation to acceleration CPU-bound operations
    void SetSkiaJIT(bool allow);

    // Whether to collect profiling samples for RenderHost's message queue
    void SetProfileRenderHostTransfer(bool value = true);

    // Set tile dimensions if tile-based rendering is available
    void SetTileWidth(int32_t width);
    void SetTileHeight(int32_t height);

    // Set the number of RenderWorker threads to run in parallel.
    // RenderWorker threads are usually used to perform tile-based rendering.
    g_inline void SetRenderWorkersConcurrencyCount(uint32_t count) {
        render_workers_concurrency_count_ = count;
    }

    // Whether draw a grid of tiles if tile-based rendering is available
    g_inline void SetShowTileBoundaries(bool v) {
        show_tile_boundaries_ = v;
    }

    g_inline void SetEnableProfiler(bool v) {
        enable_profiler_ = v;
    }

    g_nodiscard g_inline bool GetEnableProfiler() const {
        return enable_profiler_;
    }

    g_inline void SetProfilerRingBufferThreshold(size_t v) {
        profiler_rb_threshold_ = v;
    }

    g_nodiscard g_inline size_t GetProfilerRingBufferThreshold() const {
        return profiler_rb_threshold_;
    }

    g_nodiscard g_inline bool GetDisableHWCompose() const {
        return disable_hw_compose_;
    }

    g_inline void SetDisableHWCompose(bool v) {
        disable_hw_compose_ = v;
    }

    g_nodiscard g_inline bool GetEnableVkDBG() const {
        return enable_vkdbg_;
    }

    g_inline void SetEnableVkDBG(bool v) {
        enable_vkdbg_ = v;
    }

    g_nodiscard g_inline std::vector<std::string>& GetVkDBGFilterSeverities() {
        return vkdbg_filter_severities_;
    }

    g_nodiscard g_inline std::vector<std::string>& GetVkDBGFilterLevels() {
        return vkdbg_filter_levels_;
    }

    g_inline void SetVkDBGFilterSeverities(const std::vector<std::string>& v) {
        vkdbg_filter_severities_ = v;
    }

    g_inline void SetVkDBGFilterLevels(const std::vector<std::string>& v) {
        vkdbg_filter_levels_ = v;
    }

private:
    Backends    backend_;
    bool        skia_jit_;
    bool        profile_render_host_transfer_;
    int32_t     tile_width_;
    int32_t     tile_height_;
    uint32_t    render_workers_concurrency_count_;
    bool        show_tile_boundaries_;
    bool        enable_profiler_;
    size_t      profiler_rb_threshold_;
    bool        disable_hw_compose_;

    bool        enable_vkdbg_;
    std::vector<std::string> vkdbg_filter_severities_;
    std::vector<std::string> vkdbg_filter_levels_;
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
        ApplicationInfo(const ApplicationInfo&) = default;
        ApplicationInfo(ApplicationInfo&& rhs) noexcept
                : name(std::move(rhs.name))
                , version_triple(std::move(rhs.version_triple)) {}

        std::string         name;
        VersionTriple       version_triple;
    };

    struct ExternalData
    {
        using Deleter = std::function<void(void*)>;
        void *ptr;
        Deleter deleter;
    };

    GlobalScope(const ContextOptions& options, EventLoop *loop);
    ~GlobalScope();

    g_nodiscard ContextOptions& GetOptions();

    void Initialize(const ApplicationInfo& info);
    void Dispose();

    g_nodiscard g_inline bool HasInitialized() const {
        return (render_host_ && render_client_);
    }

    g_nodiscard g_inline RenderHost *GetRenderHost() {
        return render_host_;
    }

    g_nodiscard g_inline RenderClient *GetRenderClient() {
        return render_client_;
    }

    const Unique<StandaloneThreadPool>& GetRenderWorkersThreadPool();

    /**
     * Get a using report of graphics resources in JSON format.
     * The method traces alive graphics objects and their members recursively,
     * collects information about graphics memory, GPU handles, etc. and
     * finally gives a JSON report.
     * The method should be called in the rendering thread to avoid synchronous
     * problems. If it is necessary to trace resources from other threads,
     * use `RenderHostTaskRunner` to call it asynchronously.
     */
    std::optional<std::string> TraceResourcesToJson();

    void TraceSkiaMemoryResources();

    g_inline void SetExternalDataPointer(void *ptr, ExternalData::Deleter deleter) {
        if (external_data_.ptr && external_data_.deleter)
            external_data_.deleter(external_data_.ptr);
        external_data_.ptr = ptr;
        external_data_.deleter = std::move(deleter);
    }

    g_nodiscard g_inline void *GetExternalDataPointer() const {
        return external_data_.ptr;
    }

    g_nodiscard g_inline auto *GetGpuThreadSharedObjectsCollector() const {
        return thread_shared_objs_collector_.get();
    }

private:
    ContextOptions  options_;
    EventLoop      *event_loop_;
    RenderHost     *render_host_;
    RenderClient   *render_client_;
    Unique<StandaloneThreadPool>  render_workers_;
    ExternalData    external_data_;
    Unique<GpuThreadSharedObjectsCollector>
                    thread_shared_objs_collector_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_GLAMOR_H
