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

#include "include/core/SkGraphics.h"
#include "include/core/SkTraceMemoryDump.h"
#include "Core/StandaloneThreadPool.h"
#include "Core/Journal.h"
#include "Glamor/Glamor.h"
#include "Glamor/RenderHost.h"
#include "Glamor/RenderClient.h"
#include "Glamor/MaybeGpuObject.h"
#include "Glamor/GProfiler.h"
GLAMOR_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Glamor)

ContextOptions::ContextOptions()
    : backend_(Backends::kDefault)
    , skia_jit_(GLAMOR_SKIA_JIT_DEFAULT)
    , profile_render_host_transfer_(false)
    , tile_width_(GLAMOR_TILE_WIDTH_DEFAULT)
    , tile_height_(GLAMOR_TILE_HEIGHT_DEFAULT)
    , render_workers_concurrency_count_(GLAMOR_WORKERS_CONCURRENCY)
    , show_tile_boundaries_(false)
    , enable_profiler_(false)
    , profiler_rb_threshold_(GLAMOR_PROFILER_RINGBUFFER_THRESHOLD_DEFAULT)
    , disable_hw_compose_(false)
    , enable_vkdbg_(false)
    , vkdbg_filter_severities_{"general", "performance", "validation"}
    , vkdbg_filter_levels_{"verbose", "info", "warning", "error"}
{
}

Backends ContextOptions::GetBackend() const
{
    return backend_;
}

bool ContextOptions::GetSkiaJIT() const
{
    return skia_jit_;
}

void ContextOptions::SetBackend(Backends backend)
{
    backend_ = backend;
}

void ContextOptions::SetSkiaJIT(bool allow)
{
    skia_jit_ = allow;
}

bool ContextOptions::GetProfileRenderHostTransfer() const
{
    return profile_render_host_transfer_;
}

void ContextOptions::SetProfileRenderHostTransfer(bool value)
{
    profile_render_host_transfer_ = value;
}

int32_t ContextOptions::GetTileHeight() const
{
    return tile_height_;
}

void ContextOptions::SetTileHeight(int32_t height)
{
    tile_height_ = height;
}

int32_t ContextOptions::GetTileWidth() const
{
    return tile_width_;
}

void ContextOptions::SetTileWidth(int32_t width)
{
    tile_width_ = width;
}

GlobalScope::GlobalScope(const ContextOptions& options, EventLoop *loop)
    : options_(options)
    , event_loop_(loop)
    , render_host_(nullptr)
    , render_client_(nullptr)
    , external_data_{nullptr, {}}
    , thread_shared_objs_collector_(
            std::make_unique<GpuThreadSharedObjectsCollector>())
{
    if (options_.GetSkiaJIT())
        SkGraphics::AllowJIT();
}

void GlobalScope::Initialize(const ApplicationInfo& info)
{
    if (render_host_ || render_client_)
    {
        QLOG(LOG_WARNING, "Initialize GlobalScope multiple times");
        return;
    }

    render_host_ = new RenderHost(event_loop_, info);
    render_client_ = new RenderClient(render_host_);
    render_host_->SetRenderClient(render_client_);
}

void GlobalScope::Dispose()
{
    // Destroy all the alive GPU objects which are referenced
    // by all other threads except GPU thread.
    if (thread_shared_objs_collector_)
        thread_shared_objs_collector_->Collect();
    thread_shared_objs_collector_.reset();

    if (external_data_.ptr && external_data_.deleter)
        external_data_.deleter(external_data_.ptr);
    external_data_.ptr = nullptr;
    external_data_.deleter = {};

    if (!render_client_ || !render_host_)
    {
        QLOG(LOG_ERROR, "Disposing GlobalScope without an initialization");
        return;
    }

    delete render_client_;
    render_client_ = nullptr;
    render_host_->OnDispose();
    /* This method may be called in a callback of host invocation,
     * so we should not delete @p render_host_ here. */
}

GlobalScope::~GlobalScope()
{
    if (render_client_)
    {
        QLOG(LOG_ERROR, "Destructing GlobalScope without disposing it is unexpected");
        delete render_client_;
    }
    delete render_host_;
}

ContextOptions& GlobalScope::GetOptions()
{
    return options_;
}

const Unique<StandaloneThreadPool>& GlobalScope::GetRenderWorkersThreadPool()
{
    if (!render_workers_)
    {
        uint32_t count = options_.GetRenderWorkersConcurrencyCount();
        render_workers_ = std::make_unique<StandaloneThreadPool>("TileWorker", count);
        CHECK(render_workers_);
    }

    return render_workers_;
}

std::optional<std::string> GlobalScope::TraceResourcesToJson()
{
    if (!render_client_ || !render_host_)
        return {};

    GraphicsResourcesTrackable::Tracer tracer;
    tracer.TraceRootObject("RenderClient", render_client_);
    tracer.TraceRootObject("ThreadSharedObjectsCollector",
                           thread_shared_objs_collector_.get());

    // TODO(sora): Also trace `RenderHost`
    return tracer.ToJsonString();
}

namespace {

class SkiaMemoryTracer : public SkTraceMemoryDump
{
public:
    void dumpNumericValue(const char* dumpName,
                          const char* valueName,
                          const char* units,
                          uint64_t value) override
    {
        fmt::print("{}: {} = {} {}\n", dumpName, valueName, value, units);
    }

    void dumpStringValue(const char* dumpName,
                         const char* valueName,
                         const char* value) override
    {
        fmt::print("{}: {} = \"{}\"\n", dumpName, valueName, value);
    }

    void setMemoryBacking(const char* dumpName,
                          const char* backingType,
                          const char* backingObjectId) override
    {
        //fmt::print("{}: [memory backing] type={}, object={}\n", dumpName, backingType,
        //           backingObjectId);
    }


    void setDiscardableMemoryBacking(const char* dumpName,
                                     const SkDiscardableMemory& discardableMemoryObject) override
    {
    }

    g_nodiscard LevelOfDetail getRequestedDetails() const override
    {
        return LevelOfDetail::kObjectsBreakdowns_LevelOfDetail;
    }
};

} // namespace anonymous

void GlobalScope::TraceSkiaMemoryResources()
{
    SkiaMemoryTracer tracer;
    fmt::print("===== Dump Begin ====\n");
    SkGraphics::DumpMemoryStatistics(&tracer);
    fmt::print("===== Dump End ====\n");
}

GLAMOR_NAMESPACE_END
