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

#define VK_USE_PLATFORM_WAYLAND_KHR

#include "Core/Errors.h"
#include "Core/Journal.h"
#include "Glamor/RenderClient.h"
#include "Glamor/RenderHostInvocation.h"
#include "Glamor/RenderClientCallInfo.h"
#include "Glamor/RenderClientObject.h"
#include "Glamor/RenderHost.h"
#include "Glamor/Display.h"

#include "Glamor/HWComposeContext.h"
GLAMOR_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Glamor.RenderClient)

RenderClient::RenderClient(RenderHost *renderHost)
    : render_host_(renderHost)
    , client_event_loop_(nullptr)
    , host_call_async_{}
    , dltsi_idle_handle_{}
    , disposed_(false)
    , thread_stopped_(false)
    , hw_compose_context_creation_failed_(false)
    , hw_compose_disabled_(false)
{
    client_event_loop_ = new uv_loop_t;
    CHECK(client_event_loop_);
    uv_loop_init(client_event_loop_);

    uv_async_init(client_event_loop_, &host_call_async_, InvocationFromHostTrampoline);
    uv_handle_set_data((uv_handle_t*)&host_call_async_, this);

    uv_idle_init(client_event_loop_, &dltsi_idle_handle_);
    dltsi_idle_handle_.data = this;

    thread_ = std::thread(&RenderClient::RenderThread, this);
}

RenderClient::~RenderClient()
{
    Dispose();
}

void RenderClient::Dispose()
{
    if (!disposed_)
    {
        disposed_ = true;
        while (!thread_stopped_)
            uv_async_send(&host_call_async_);

        thread_.join();

        uv_loop_close(client_event_loop_);
        delete client_event_loop_;
    }
}

void RenderClient::InvocationFromHostTrampoline(uv_async_t *handle)
{
    auto *pClient = reinterpret_cast<RenderClient*>(uv_handle_get_data((uv_handle_t*) handle));
    CHECK(pClient);
    pClient->OnInvocationFromHost();
}

void RenderClient::RenderThread()
{
    pthread_setname_np(pthread_self(), "RenderThread");
    QLOG(LOG_INFO, "Render thread has started, RenderClient:{}", fmt::ptr(this));

    uv_run(client_event_loop_, UV_RUN_DEFAULT);
    thread_stopped_ = true;

    QLOG(LOG_INFO, "Render thread has stopped, RenderClient:{}", fmt::ptr(this));
}

void RenderClient::ScheduleDeferredLocalThreadSlotsInvocation(const Shared<RenderClientSignalEmit>& emit,
                                                              const Shared<RenderClientObject>& emitter)
{
    if (dltsi_closures_queue_.empty())
        uv_idle_start(&dltsi_idle_handle_, RenderClient::DLTSICallback);
    dltsi_closures_queue_.emplace(DLTSIClosure{ emit, emitter });
}

void RenderClient::DLTSICallback(uv_idle_t *handle)
{
    auto *client = reinterpret_cast<RenderClient*>(handle->data);
    CHECK(client);

    while (!client->dltsi_closures_queue_.empty())
    {
        DLTSIClosure& closure = client->dltsi_closures_queue_.front();
        closure.emitter->EmitterTrampoline(closure.emit, true);

        client->dltsi_closures_queue_.pop();
    }

    uv_idle_stop(handle);
}

void RenderClient::OnInvocationFromHost()
{
    if (disposed_)
    {
        uv_close((uv_handle_t*) &host_call_async_, nullptr);
        return;
    }

    Shared<RenderHostInvocation> pInvocation;
    host_invocation_queue_lock_.lock();
    while (!host_invocation_queue_.empty())
    {
        pInvocation = host_invocation_queue_.front();
        host_invocation_queue_.pop();
        host_invocation_queue_lock_.unlock();

        pInvocation->MarkProfileMilestone(ITCProfileMilestone::kClientReceived);

        Shared<RenderClientObject> pReceiver = pInvocation->GetReceiver();
        pReceiver->CallFromHostTrampoline(pInvocation->GetClientCallInfo());

        pInvocation->MarkProfileMilestone(ITCProfileMilestone::kClientProcessed);

        render_host_->WakeupHost(pInvocation);
        pInvocation->MarkProfileMilestone(ITCProfileMilestone::kClientFeedback);

        host_invocation_queue_lock_.lock();
    }
    host_invocation_queue_lock_.unlock();
}

void RenderClient::EnqueueHostInvocation(const Shared<RenderHostInvocation>& invocation)
{
    {
        std::scoped_lock scope(host_invocation_queue_lock_);
        host_invocation_queue_.push(invocation);
    }
    uv_async_send(&host_call_async_);
}

namespace {

// NOLINTNEXTLINE
std::map<std::string, HWComposeContext::Options::VkDBGTypeFilter> g_vkdbg_type_filters_name = {
    { "general",     HWComposeContext::Options::VkDBGTypeFilter::kGeneral     },
    { "performance", HWComposeContext::Options::VkDBGTypeFilter::kPerformance },
    { "validation",  HWComposeContext::Options::VkDBGTypeFilter::kValidation  }
};

// NOLINTNEXTLINE
std::map<std::string, HWComposeContext::Options::VkDBGLevelFilter> g_vkdbg_level_filters_name = {
    { "verbose",  HWComposeContext::Options::VkDBGLevelFilter::kVerbose },
    { "info",     HWComposeContext::Options::VkDBGLevelFilter::kInfo    },
    { "warning",  HWComposeContext::Options::VkDBGLevelFilter::kWarning },
    { "error",    HWComposeContext::Options::VkDBGLevelFilter::kError   }
};

} // namespace anonymous

Shared<HWComposeContext> RenderClient::GetHWComposeContext()
{
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

    auto& hostApplicationInfo = render_host_->GetApplicationInfo();
    HWComposeContext::Options options{};

    options.application_name = hostApplicationInfo.name;
    options.application_version_major = std::get<0>(hostApplicationInfo.version_triple);
    options.application_version_minor = std::get<1>(hostApplicationInfo.version_triple);
    options.application_version_patch = std::get<2>(hostApplicationInfo.version_triple);
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

    switch (GlobalScope::Ref().GetOptions().GetBackend())
    {
    case Backends::kWayland:
        options.instance_extensions.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);
        options.instance_extensions.emplace_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
        break;
    }

    hw_compose_context_ = HWComposeContext::MakeVulkan(options);

    if (!hw_compose_context_)
        hw_compose_context_creation_failed_ = true;
    return hw_compose_context_;
}

void RenderClient::RegisterDisplay(Display *display)
{
    display_registry_.push_back(display);
}

void RenderClient::UnregisterDisplay(Display *display)
{
    auto itr = std::find(display_registry_.begin(),
                         display_registry_.end(),
                         display);
    if (itr != display_registry_.end())
    {
        display_registry_.erase(itr);
    }
}

void RenderClient::Trace(GraphicsResourcesTrackable::Tracer *tracer) noexcept
{
    if (hw_compose_context_)
        tracer->TraceMember("HWComposeContext", hw_compose_context_.get());

    int32_t index = 0;
    for (Display *display : display_registry_)
    {
        tracer->TraceMember(fmt::format("Display#{}", index), display);
        index++;
    }
}

GLAMOR_NAMESPACE_END
