#define VK_USE_PLATFORM_WAYLAND_KHR

#include "Core/Errors.h"
#include "Core/Journal.h"
#include "Core/Properties.h"
#include "Cobalt/RenderClient.h"
#include "Cobalt/RenderHostInvocation.h"
#include "Cobalt/RenderClientCallInfo.h"
#include "Cobalt/RenderClientObject.h"
#include "Cobalt/RenderHost.h"

#include "Cobalt/HWComposeContext.h"
COBALT_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Cobalt.RenderClient)

RenderClient::RenderClient(RenderHost *renderHost)
    : render_host_(renderHost)
    , client_event_loop_(nullptr)
    , host_call_async_{}
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

void RenderClient::OnInvocationFromHost()
{
    if (disposed_)
    {
        uv_close((uv_handle_t*) &host_call_async_, nullptr);
        return;
    }

    RenderHostInvocation *pInvocation;
    host_invocation_queue_lock_.lock();
    while (!host_invocation_queue_.empty())
    {
        pInvocation = host_invocation_queue_.front();
        host_invocation_queue_.pop();
        host_invocation_queue_lock_.unlock();

        pInvocation->MarkProfileMilestone(ITCProfileMilestone::kClientReceived);

        co_sp<RenderClientObject> pReceiver = pInvocation->GetReceiver();
        pReceiver->CallFromHostTrampoline(pInvocation->GetClientCallInfo());

        pInvocation->MarkProfileMilestone(ITCProfileMilestone::kClientProcessed);

        render_host_->WakeupHost(pInvocation);
        pInvocation->MarkProfileMilestone(ITCProfileMilestone::kClientFeedback);

        host_invocation_queue_lock_.lock();
    }
    host_invocation_queue_lock_.unlock();
}

void RenderClient::EnqueueHostInvocation(RenderHostInvocation *invocation)
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

co_sp<HWComposeContext> RenderClient::GetHWComposeContext()
{
    if (hw_compose_context_creation_failed_ || hw_compose_disabled_)
        return nullptr;

    using DN = PropertyDataNode;
    auto hwComposeNode = prop::Get()
                         ->next("Graphics")
                         ->next("HWCompose")
                         ->as<PropertyObjectNode>();
    if (hwComposeNode->hasMember("Disabled"))
    {
        if (hwComposeNode->getMember("Disabled")->as<DN>()->extract<bool>())
        {
            QLOG(LOG_INFO, "HWCompose is disabled for current environment");
            hw_compose_disabled_ = true;
            return nullptr;
        }
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

    if (hwComposeNode->hasMember("EnableVkDBG"))
    {
        if (hwComposeNode->getMember("EnableVkDBG")->as<DN>()->extract<bool>())
        {
            QLOG(LOG_INFO, "Enabled VkDBG feature for HWCompose context");
            options.use_vkdbg = true;
        }
    }

    if (options.use_vkdbg)
    {
        options.vkdbg_type_filter |= HWComposeContext::Options::VkDBGTypeFilter::kGeneral;
        options.vkdbg_level_filter |= HWComposeContext::Options::VkDBGLevelFilter::kWarning;
        options.vkdbg_level_filter |= HWComposeContext::Options::VkDBGLevelFilter::kError;

        if (hwComposeNode->hasMember("VkDBGFilterSeverities"))
        {
            options.vkdbg_type_filter.clear();
            auto array = hwComposeNode->getMember("VkDBGFilterSeverities")->as<PropertyArrayNode>();
            for (const std::shared_ptr<PropertyNode>& node : *array)
            {
                auto str = node->as<DN>()->extract<std::string>();
                if (g_vkdbg_type_filters_name.count(str) == 0)
                {
                    QLOG(LOG_WARNING, "Unrecognized severity name of VkDBG filter: {}", str);
                    continue;
                }
                options.vkdbg_type_filter |= g_vkdbg_type_filters_name[str];
            }
        }

        if (hwComposeNode->hasMember("VkDBGFilterLevels"))
        {
            options.vkdbg_level_filter.clear();
            auto array = hwComposeNode->getMember("VkDBGFilterLevels")->as<PropertyArrayNode>();
            for (const std::shared_ptr<PropertyNode>& node : *array)
            {
                auto str = node->as<DN>()->extract<std::string>();
                if (g_vkdbg_level_filters_name.count(str) == 0)
                {
                    QLOG(LOG_WARNING, "Unrecognized information level of VkDBG filter: {}", str);
                    continue;
                }
                options.vkdbg_level_filter |= g_vkdbg_level_filters_name[str];
            }
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

COBALT_NAMESPACE_END
