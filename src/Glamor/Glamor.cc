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
#include "Core/StandaloneThreadPool.h"
#include "Core/Journal.h"
#include "Glamor/Glamor.h"
#include "Glamor/SkEventTracerImpl.h"
#include "Glamor/HWComposeContext.h"
#include "Glamor/PresentThread.h"
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
    , disable_hw_compose_present_(false)
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
    , application_info_("Cocoa", std::make_tuple(0, 0, 0))
    , event_loop_(loop)
    , skia_event_tracer_impl_(new SkEventTracerImpl())
    , hw_compose_context_creation_failed_(false)
    , hw_compose_disabled_(false)
{
    SkEventTracer::SetInstance(skia_event_tracer_impl_, false);

    /*
    if (options_.GetSkiaJIT())
        SkGraphics::AllowJIT();
    */
}

GlobalScope::~GlobalScope() = default;

ContextOptions& GlobalScope::GetOptions()
{
    return options_;
}

const std::unique_ptr<StandaloneThreadPool>& GlobalScope::GetRenderWorkersThreadPool()
{
    if (!render_workers_)
    {
        uint32_t count = options_.GetRenderWorkersConcurrencyCount();
        render_workers_ = std::make_unique<StandaloneThreadPool>("TileWorker", count);
        CHECK(render_workers_);
    }

    return render_workers_;
}

bool GlobalScope::StartPresentThread()
{
    present_thread_ = PresentThread::Start(event_loop_->handle());
    if (!present_thread_)
    {
        QLOG(LOG_ERROR, "Failed to start present thread");
        return false;
    }
    return true;
}

void GlobalScope::DisposePresentThread()
{
    if (!present_thread_)
        return;
    present_thread_->Dispose();
    present_thread_.reset();
}

namespace {

using VkDBGTypeFilter = HWComposeContext::Options::VkDBGTypeFilter;
using VkDBGLevelFilter = HWComposeContext::Options::VkDBGLevelFilter;

// NOLINTNEXTLINE
std::map<std::string, VkDBGTypeFilter> g_vkdbg_type_filters_name = {
    { "general",     VkDBGTypeFilter::kGeneral     },
    { "performance", VkDBGTypeFilter::kPerformance },
    { "validation",  VkDBGTypeFilter::kValidation  }
};

// NOLINTNEXTLINE
std::map<std::string, VkDBGLevelFilter> g_vkdbg_level_filters_name = {
    { "verbose",  VkDBGLevelFilter::kVerbose },
    { "info",     VkDBGLevelFilter::kInfo    },
    { "warning",  VkDBGLevelFilter::kWarning },
    { "error",    VkDBGLevelFilter::kError   }
};

} // namespace anonymous

std::shared_ptr<HWComposeContext> GlobalScope::GetHWComposeContext()
{
    // Only one thread can create a `HWComposeContext` at the same time.
    // Once a context is created, it will be used by all the threads.
    std::scoped_lock<std::mutex> lock(hw_compose_creation_lock_);

    if (hw_compose_context_creation_failed_ || hw_compose_disabled_)
        return nullptr;

    if (GlobalScope::Ref().GetOptions().GetDisableHWCompose())
    {
        QLOG(LOG_INFO, "HWCompose is disabled for current environment");
        hw_compose_disabled_ = true;
        return nullptr;
    }

    if (hw_compose_context_)
        return hw_compose_context_;

    HWComposeContext::Options options{};

    options.application_name = application_info_.name;
    options.application_version_major = std::get<0>(application_info_.version_triple);
    options.application_version_minor = std::get<1>(application_info_.version_triple);
    options.application_version_patch = std::get<2>(application_info_.version_triple);
    options.use_vkdbg = false;

    ContextOptions& gl_options = GlobalScope::Ref().GetOptions();

    if (gl_options.GetEnableVkDBG())
    {
        QLOG(LOG_INFO, "Enabled VkDBG feature for HWCompose context");
        options.use_vkdbg = true;

        for (const std::string& name : gl_options.GetVkDBGFilterSeverities())
        {
            if (g_vkdbg_type_filters_name.count(name) == 0)
            {
                QLOG(LOG_WARNING, "Unrecognized severity name of VkDBG filter: {}", name);
                continue;
            }
            options.vkdbg_type_filter |= g_vkdbg_type_filters_name[name];
        }

        for (const std::string& name : gl_options.GetVkDBGFilterLevels())
        {
            if (g_vkdbg_level_filters_name.count(name) == 0)
            {
                QLOG(LOG_WARNING, "Unrecognized information level of VkDBG filter: {}", name);
                continue;
            }
            options.vkdbg_level_filter |= g_vkdbg_level_filters_name[name];
        }
    }

    options.device_extensions.emplace_back("VK_KHR_external_memory");
    options.device_extensions.emplace_back("VK_KHR_external_memory_fd");
    options.device_extensions.emplace_back("VK_KHR_external_semaphore");
    options.device_extensions.emplace_back("VK_KHR_external_semaphore_fd");

    if (!gl_options.GetDisableHWComposePresent())
    {
        switch (gl_options.GetBackend())
        {
        case Backends::kWayland:
            options.instance_extensions.emplace_back("VK_KHR_surface");
            options.instance_extensions.emplace_back("VK_KHR_wayland_surface");
            break;
        }
    }

    hw_compose_context_ = HWComposeContext::MakeVulkan(options);

    if (!hw_compose_context_)
        hw_compose_context_creation_failed_ = true;

    return hw_compose_context_;
}

GLAMOR_NAMESPACE_END
