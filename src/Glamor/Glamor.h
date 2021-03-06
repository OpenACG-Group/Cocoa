#ifndef COCOA_GLAMOR_GLAMOR_H
#define COCOA_GLAMOR_GLAMOR_H

#include <memory>
#include <vector>

#include "Core/Project.h"
#include "Core/UniquePersistent.h"
#include "Core/EventLoop.h"

#define GLAMOR_NAMESPACE_BEGIN  namespace cocoa::glamor {
#define GLAMOR_NAMESPACE_END    }

namespace cocoa {
class OutOfLoopThreadPool;
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

enum class Backends
{
    kWayland,
    kDefault = kWayland
};

class RenderHost;
class RenderClient;
class MoeJITContext;

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

private:
    Backends    backend_;
    bool        skia_jit_;
    bool        profile_render_host_transfer_;
    int32_t     tile_width_;
    int32_t     tile_height_;
    uint32_t    render_workers_concurrency_count_;
    bool        show_tile_boundaries_;
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

    GlobalScope(const ContextOptions& options, EventLoop *loop);
    ~GlobalScope();

    g_nodiscard ContextOptions& GetOptions();

    void Initialize(const ApplicationInfo& info);
    void Dispose();

    g_nodiscard g_inline RenderHost *GetRenderHost() {
        return render_host_;
    }

    g_nodiscard g_inline RenderClient *GetRenderClient() {
        return render_client_;
    }

    const Unique<OutOfLoopThreadPool>& GetRenderWorkersThreadPool();
    const Unique<MoeJITContext>& GetJITContext();

private:
    ContextOptions  options_;
    EventLoop      *event_loop_;
    RenderHost     *render_host_;
    RenderClient   *render_client_;
    Unique<OutOfLoopThreadPool>  render_workers_;
    Unique<MoeJITContext>        jit_context_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_GLAMOR_H
