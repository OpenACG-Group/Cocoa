#include "Glamor/RenderHost.h"

#include <fstream>
#include <iostream>
#include <utility>
#include "fmt/format.h"
#include "json/writer.h"

#include "Core/Project.h"
#include "Core/Journal.h"
#include "Glamor/RenderHostInvocation.h"
#include "Glamor/RenderClient.h"
#include "Glamor/RenderHostCreator.h"
#include "Glamor/RenderClientEmitterInfo.h"
GLAMOR_NAMESPACE_BEGIN

#define TRANSFER_PROFILE_CACHE_SIZE     10
#define TRANSFER_PROFILE_TYPE           "transfer-profile"

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Glamor.RenderHost)

RenderHost::RenderHost(EventLoop *hostLoop, ApplicationInfo applicationInfo)
    : AsyncSource(hostLoop)
    , render_client_(nullptr)
    , host_creator_(std::make_shared<RenderHostCreator>())
    , host_task_runner_(std::make_shared<RenderHostTaskRunner>())
    , application_info_(std::move(applicationInfo))
    , samples_time_base_(std::chrono::steady_clock::now())
    , profile_json_root_(nullptr)
{
    if (GlobalScope::Ref().GetOptions().GetProfileRenderHostTransfer())
    {
        profile_json_root_ = std::shared_ptr<Json::Value>(new Json::Value(), [](Json::Value *v) {
            Json::StreamWriterBuilder builder;
            builder["indentation"] = "  ";
            builder["commentStyle"] = "None";
            builder["enableYAMLCompatibility"] = true;

            std::string path = fmt::format("transfer-profile-{}.json", uv_os_getpid());
            std::ofstream fs(path);
            if (!fs.is_open())
                QLOG(LOG_ERROR, "Failed to open {} to write profiling results", path);
            else
            {
                Unique<Json::StreamWriter> writer(builder.newStreamWriter());
                writer->write(*v, &fs);
            }

            delete v;
        });
        CHECK(profile_json_root_);

        Json::Value& v = *profile_json_root_;
        v["Type"] = TRANSFER_PROFILE_TYPE;
        v["Module"] = "org.OpenACG.Cocoa.Glamor";
        Json::Value& version = v["Version"];
        version[0] = COCOA_MAJOR;
        version[1] = COCOA_MINOR;
        version[2] = COCOA_PATCH;
        v["Sampler"] = "RenderHost";
    }
}

RenderHost::~RenderHost()
{
    CHECK(host_creator_.unique());
    if (profile_json_root_)
        FlushProfileSamplesAsync();
}

void RenderHost::OnDispose()
{
    AsyncSource::disableAsync();
    render_client_ = nullptr;
}

Shared<RenderClientObject> RenderHost::GetRenderHostCreator()
{
    return host_creator_;
}

void RenderHost::asyncDispatch()
{
    OnResponseFromClient();
}

void RenderHost::OnResponseFromClient()
{
    std::shared_ptr<RenderClientTransfer> transfer;

    client_transfer_queue_lock_.lock();
    while (!client_transfer_queue_.empty())
    {
        transfer = client_transfer_queue_.front();
        client_transfer_queue_.pop();
        client_transfer_queue_lock_.unlock();

        transfer->MarkProfileMilestone(ITCProfileMilestone::kHostReceived);
        if (transfer->IsInvocationResponse())
        {
            auto invocation = std::dynamic_pointer_cast<RenderHostInvocation>(transfer);
            invocation->MarkProfileMilestone(ITCProfileMilestone::kHostReceived);
            RenderHostCallbackInfo callbackInfo(invocation.get());
            invocation->GetHostCallback()(callbackInfo);
        }
        else if (transfer->IsSignalEmit())
        {
            auto emit = std::dynamic_pointer_cast<RenderClientSignalEmit>(transfer);
            Shared<RenderClientObject> emitter = emit->GetEmitter();
            emitter->EmitterTrampoline(emit, false);
        }

        if (GlobalScope::Ref().GetOptions().GetProfileRenderHostTransfer())
            CollectTransferProfileSample(transfer.get());

        client_transfer_queue_lock_.lock();
    }
    client_transfer_queue_lock_.unlock();
}

void RenderHost::Send(const Shared<RenderClientObject>& receiver, RenderClientCallInfo info,
                      const RenderHostCallback& pCallback)
{
    CHECK(receiver);
    CHECK(render_client_);

    auto invocation = std::make_shared<RenderHostInvocation>(receiver, std::move(info), pCallback);
    invocation->MarkProfileMilestone(ITCProfileMilestone::kHostConstruction);

    render_client_->EnqueueHostInvocation(invocation);
    invocation->MarkProfileMilestone(ITCProfileMilestone::kHostEnqueued);
}

void RenderHost::WakeupHost(const std::shared_ptr<RenderClientTransfer>& transfer)
{
    {
        std::scoped_lock scope(client_transfer_queue_lock_);
        client_transfer_queue_.push(transfer);
    }
    AsyncSource::wakeupAsync();
}

namespace {

#define E2(t, p) { ITCProfileMilestone::k##t, #t, RenderClientTransfer::Type::k##p }
#define E1(t)    { ITCProfileMilestone::k##t, #t }
struct {
    ITCProfileMilestone tag{};
    const char *name{};
    std::optional<RenderClientTransfer::Type> type_selector;
} g_milestone_tagged_names[] = {
    E2(HostConstruction, InvocationResponse),
    E2(HostEnqueued, InvocationResponse),
    E2(ClientReceived, InvocationResponse),
    E2(ClientProcessed, InvocationResponse),
    E2(ClientFeedback, InvocationResponse),
    E1(HostReceived),
    E2(ClientEmitted, SignalEmit)
};
#undef E1
#undef E2

#define E(t)   { RenderClientObject::ReturnStatus::k##t, #t }
struct {
    RenderClientObject::ReturnStatus st;
    const char *name;
} g_return_status_names[] = {
    E(Pending),
    E(OpCodeInvalid),
    E(ArgsInvalid),
    E(Caught),
    E(OpSuccess),
    E(OpFailed)
};
#undef E

#define E(t)   { RenderClientObject::RealType::k##t, #t }
struct {
    RenderClientObject::RealType rt;
    const char *name;
} g_rco_realtype_names[] = {
    E(RenderHostCreator),
    E(Display),
    E(Surface),
    E(Blender),
    E(RenderHostTaskRunner),
    E(Monitor)
};

} // namespace anonymous

struct RenderHost::TransferProfileSample
{
    constexpr static size_t kSize = static_cast<uint32_t>(ITCProfileMilestone::kLast);
    using SampleType = RenderClientTransfer::Type;
    using SampleTimepoint = RenderClientTransfer::Timepoint;

    explicit TransferProfileSample(RenderClientTransfer *transfer)
        : type(SampleType::kInvocationResponse)
        , opcode(0)
        , return_status(RenderClientObject::ReturnStatus::kPending)
        , obj(nullptr)
        , obj_type()
        , sigcode(0)
        , milestones{}
    {
        if (transfer->IsInvocationResponse())
        {
            type = SampleType::kInvocationResponse;
            auto *ptr = dynamic_cast<RenderHostInvocation *>(transfer);
            opcode = ptr->GetClientCallInfo().GetOpCode();
            obj = ptr->GetReceiver().get();
            obj_type = ptr->GetReceiver()->GetRealType();
            return_status = ptr->GetClientCallInfo().GetReturnStatus();
        }
        else if (transfer->IsSignalEmit())
        {
            type = SampleType::kSignalEmit;
            auto *ptr = dynamic_cast<RenderClientSignalEmit *>(transfer);
            sigcode = ptr->GetSignalCode();
            obj = ptr->GetEmitter().get();
            obj_type = ptr->GetEmitter()->GetRealType();
        }

        for (uint32_t i = 0; i < kSize; i++)
            milestones[i] = transfer->GetProfileMilestone(static_cast<ITCProfileMilestone>(i));
    }
    ~TransferProfileSample() = default;

    void Serialize(SampleTimepoint baseTime, Json::Value& value)
    {
        const char *objectTypeName = "Unknown";
        for (const auto& pair : g_rco_realtype_names)
        {
            if (pair.rt == obj_type)
                objectTypeName = pair.name;
        }

        if (type == SampleType::kInvocationResponse)
        {
            const char *returnStatus = "Unknown";
            for (const auto& pair : g_return_status_names)
            {
                if (pair.st == return_status)
                    returnStatus = pair.name;
            }

            value["Type"] = "Invocation";
            value["Opcode"] = opcode;
            value["Receiver"] = fmt::format("{}", obj);
            value["ReturnStatus"] = returnStatus;
            value["ReceiverType"] = objectTypeName;
        }
        else
        {
            value["Type"] = "Signal";
            value["SignalCode"] = sigcode;
            value["Emitter"] = fmt::format("{}", obj);
            value["EmitterType"] = objectTypeName;
        }

        Json::Value& datas = value["Milestones"];

        for (const auto& match : g_milestone_tagged_names)
        {
            if (match.type_selector.has_value() && *match.type_selector != type)
                continue;
            auto dur = milestones[static_cast<uint32_t>(match.tag)] - baseTime;
            uint64_t us = std::chrono::duration_cast<std::chrono::microseconds>(dur).count();
            datas[match.name] = us;
        }
    }

    SampleType          type;
    RenderClientObject::OpCode opcode;
    RenderClientObject::ReturnStatus return_status;
    void               *obj;
    RenderClientObject::RealType obj_type;
    RenderClientObject::SignalCode sigcode;
    SampleTimepoint     milestones[kSize];
};

namespace {

struct SamplerSerializeTaskClosure
{
    SamplerSerializeTaskClosure(std::vector<Shared<RenderHost::TransferProfileSample>>& sv,
                                RenderHost::TransferProfileSample::SampleTimepoint baseTime,
                                Shared<Json::Value> valueRoot)
        : samples(sv.size()), base_time(baseTime), values(sv.size()), json_root(std::move(valueRoot))
    {
        for (size_t i = 0; i < sv.size(); i++)
            samples[i] = std::move(sv[i]);
        sv.clear();
    }

    std::vector<Shared<RenderHost::TransferProfileSample>> samples;
    RenderHost::TransferProfileSample::SampleTimepoint base_time;
    std::vector<Json::Value> values;
    Shared<Json::Value> json_root;
};

void threadpool_perform_sampler_serialize(uv_work_t *work)
{
    auto *closure = reinterpret_cast<SamplerSerializeTaskClosure *>(
            uv_handle_get_data(reinterpret_cast<uv_handle_t *>(work)));
    CHECK(closure);

    for (size_t i = 0; i < closure->samples.size(); i++)
        closure->samples[i]->Serialize(closure->base_time, closure->values[i]);
    closure->samples.clear();
}

void threadpool_after_perform_sampler_serialize(uv_work_t *work, g_maybe_unused int status)
{
    auto *closure = reinterpret_cast<SamplerSerializeTaskClosure *>(
            uv_handle_get_data(reinterpret_cast<uv_handle_t *>(work)));
    CHECK(closure);

    Json::Value& samples = (*closure->json_root)["Samples"];
    for (Json::Value& v : closure->values)
        samples.append(std::move(v));

    delete closure;
    ::free(work);
}

} // namespace anonymous

void RenderHost::CollectTransferProfileSample(RenderClientTransfer *transfer)
{
    auto sample = std::make_shared<TransferProfileSample>(transfer);
    transfer_profile_samples_.push_back(sample);

    if (transfer_profile_samples_.size() >= TRANSFER_PROFILE_CACHE_SIZE)
        FlushProfileSamplesAsync();
}

void RenderHost::FlushProfileSamplesAsync()
{
    auto *closure = new SamplerSerializeTaskClosure(transfer_profile_samples_,
                                                    samples_time_base_, profile_json_root_);
    CHECK(closure);

    auto *work = reinterpret_cast<uv_work_t *>(::malloc(sizeof(uv_work_t)));
    CHECK(work);

    uv_handle_set_data(reinterpret_cast<uv_handle_t *>(work), closure);
    uv_queue_work(EventLoop::Ref().handle(), work, threadpool_perform_sampler_serialize,
                  threadpool_after_perform_sampler_serialize);
}

GLAMOR_NAMESPACE_END
