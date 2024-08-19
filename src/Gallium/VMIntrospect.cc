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

#include <fstream>

#include "uv.h"

#include "Core/EventLoop.h"
#include "Core/Errors.h"
#include "Core/Journal.h"
#include "Core/Utils.h"
#include "Core/TraceEvent.h"
#include "Core/ApplicationInfo.h"
#include "Gallium/VMIntrospect.h"
#include "Gallium/TracingController.h"
#include "Glamor/SkEventTracerImpl.h"
#include "Gallium/Runtime.h"
#include "Gallium/ffi/DefineClass.h"
#include "Gallium/ffi/Module.h"
#include "Gallium/ffi/Class.h"

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Gallium.Introspect)

GALLIUM_NS_BEGIN

ffi::Ret<void> VMIntrospect::SetCallbackSlot(CallbackSlot slot, v8::Local<v8::Value> func)
{
    if (!func->IsFunction())
        return ffi::Fail(ffi::kTypeErr, "Argument `func` must be a function");

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    callback_map_[slot].Reset(isolate, func.As<v8::Function>());
    return {};
}

ffi::Ret<void> VMIntrospect::setUncaughtExceptionHandler(v8::Local<v8::Value> handler)
{
    return SetCallbackSlot(CallbackSlot::kUncaughtException, handler);
}

ffi::Ret<void> VMIntrospect::setUnhandledPromiseRejectionHandler(v8::Local<v8::Value> handler)
{
    return SetCallbackSlot(CallbackSlot::kUnhandledPromiseRejection, handler);
}

ffi::Ret<void> VMIntrospect::setPromiseMultipleResolveHandler(v8::Local<v8::Value> handler)
{
    return SetCallbackSlot(CallbackSlot::kPromiseMultipleResolve, handler);
}

ffi::Ret<void> VMIntrospect::setBeforeExitHandler(v8::Local<v8::Value> handler)
{
    return SetCallbackSlot(CallbackSlot::kBeforeExit, handler);
}

ffi::RetLocal<v8::Value> VMIntrospect::scheduleScriptEval(const std::string& source)
{
    ScheduledTask task{};
    task.type = ScheduledTask::Type::kEvalScript;
    task.param = source;

    auto ctx = isolate_->GetCurrentContext();
    auto resolver = v8::Promise::Resolver::New(ctx).ToLocalChecked();
    task.resolver.Reset(isolate_, resolver);

    scheduled_task_queue_.emplace(std::move(task));
    return resolver->GetPromise();
}

ffi::RetLocal<v8::Value> VMIntrospect::scheduleModuleUrlEval(const std::string& url)
{
    ScheduledTask task{};
    task.type = ScheduledTask::Type::kEvalModuleUrl;
    task.param = url;

    auto ctx = isolate_->GetCurrentContext();
    auto resolver = v8::Promise::Resolver::New(ctx).ToLocalChecked();
    task.resolver.Reset(isolate_, resolver);

    scheduled_task_queue_.emplace(std::move(task));
    return resolver->GetPromise();
}

ffi::Ret<void> VMIntrospect::print(const std::string& str)
{
    fmt::print("{}", str);
    return {};
}

ffi::Ret<bool> VMIntrospect::hasNativeModule(const std::string& name)
{
    auto *registry = ffi::ModuleRegistry::FromIsolate(isolate_);
    return (registry->SearchModule(name) != nullptr);
}

ffi::RetLocal<v8::Value> VMIntrospect::rewind(int frame_limit)
{
    if (frame_limit <= 0)
    {
        // Use default settings
        frame_limit = Runtime::GetBareFromIsolate(isolate_)
                ->getOptions().introspect_stacktrace_frame_limit;
    }

    auto trace = v8::StackTrace::CurrentStackTrace(isolate_, frame_limit);
    if (trace.IsEmpty())
        return ffi::Fail(ffi::kErr, "Failed to capture stacktrace");

    auto result = v8::Array::New(isolate_, trace->GetFrameCount());
    auto ctx = isolate_->GetCurrentContext();

    constexpr auto kNL = v8::Message::kNoLineNumberInfo;
    constexpr auto kNC = v8::Message::kNoColumnInfo;

    for (int32_t i = 0; i < trace->GetFrameCount(); i++)
    {
        v8::Local<v8::StackFrame> frame = trace->GetFrame(isolate_, i);

        std::unordered_map<std::string, v8::Local<v8::Value>> cur;
        int32_t line = frame->GetLineNumber() != kNL ? frame->GetLineNumber() : -1;
        int32_t column = frame->GetColumn() != kNC ? frame->GetColumn() : -1;

        cur["line"] = ffi::Cast<int32_t>::To(isolate_, line);
        cur["column"] = ffi::Cast<int32_t>::To(isolate_, column);
        cur["isEval"] = ffi::Cast<bool>::To(isolate_, frame->IsEval());
        cur["isWasm"] = ffi::Cast<bool>::To(isolate_, frame->IsWasm());
        cur["isUserJavaScript"] = ffi::Cast<bool>::To(isolate_, frame->IsUserJavaScript());
        cur["isConstructor"] = ffi::Cast<bool>::To(isolate_, frame->IsConstructor());

        auto script_name = frame->GetScriptName();
        if (!script_name.IsEmpty())
            cur["scriptName"] = script_name;

        auto func_name = frame->GetFunctionName();
        if (!func_name.IsEmpty())
            cur["functionName"] = func_name;

        result->Set(ctx, i, ffi::Cast<decltype(cur)>::ToChecked(isolate_, cur)).Check();
    }

    return result;
}

ffi::Ret<void> VMIntrospect::startProcessTracing(const ffi::IFace<TracingConfig>& config)
{
    if (!ApplicationInfo::Instance()->enable_tracing)
        return ffi::Fail(ffi::kErr, "Tracing session is disabled by default, passing `--enable-tracing` to enable it.");

    if (current_tracing_session_)
        return ffi::Fail(ffi::kErr, "A tracing session has already started");

    perfetto::protos::gen::TrackEventConfig track_event_cfg;
    perfetto::TraceConfig perfetto_cfg;
    perfetto_cfg.add_buffers()->set_size_kb(config->buffer_size_kb);

    if (config->large_trace)
    {
        // Enable continuous file writing or "streaming mode" to output trace data throughout
        // the program instead of one large dump at the end.
        perfetto_cfg.set_write_into_file(true);
        // If set to a value other than the default, set how often trace data gets written to
        // the output file.
        perfetto_cfg.set_file_write_period_ms(5000);
        // Force periodic commitment of shared memory buffer pages to the central buffer.
        // Helps prevent out-of-order event slices with long traces.
        perfetto_cfg.set_flush_timeout_ms(10000);
    }

    // Set enabled track event categories
    track_event_cfg.add_disabled_categories("*");
    for (const std::string& category : config->enables)
        track_event_cfg.add_enabled_categories(category);

    auto* ds_cfg = perfetto_cfg.add_data_sources()->mutable_config();
    ds_cfg->set_name("track_event");
    ds_cfg->set_track_event_config_raw(track_event_cfg.SerializeAsString());

    // Begin a tracing session
    auto tracing_session = std::make_unique<TracingSession>();
    tracing_session->session = perfetto::Tracing::NewTrace();
    tracing_session->is_large_trace = config->large_trace;
    tracing_session->write_to_file = config->write_to_file;
    tracing_session->large_trace_fd = -1;

    if (config->large_trace)
    {
        int fd = ::open(config->write_to_file.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0600);
        if (fd < 0)
            return ffi::Fail(ffi::kErr, "failed to open output file");
        tracing_session->large_trace_fd = fd;
        tracing_session->session->Setup(perfetto_cfg, fd);
    }
    else
    {
        tracing_session->session->Setup(perfetto_cfg);
    }
    tracing_session->session->StartBlocking();

    // Notify related modules that a tracing has been started
    Runtime *runtime = Runtime::GetBareFromIsolate(v8::Isolate::GetCurrent());
    runtime->GetTracingController()->StartTracing();
    gl::GlobalScope::Ref().GetSkEventTracerImpl()->StartTracing();

    current_tracing_session_ = std::move(tracing_session);
    return {};
}

// NOLINTNEXTLINE
struct ReadTraceClosure
{
    v8::Isolate *isolate;
    v8::Global<v8::Promise::Resolver> resolver;
    size_t total_protobuf_size;
    uv_async_t async;
    std::string filepath;
    std::ofstream file_stream;
    std::unique_ptr<perfetto::TracingSession> tracing_session;
};

ffi::Ret<void> VMIntrospect::finishProcessTracing()
{
    if (!current_tracing_session_)
        return ffi::Fail(ffi::kErr, "tracing session has not been started yet");

    // Notify related modules that the tracing has been stopped
    Runtime *runtime = Runtime::GetBareFromIsolate(isolate_);
    runtime->GetTracingController()->StopTracing();
    gl::GlobalScope::Ref().GetSkEventTracerImpl()->StopTracing();

    // Stop tracing session
    auto session = std::move(current_tracing_session_);
    perfetto::TrackEvent::Flush();
    session->session->StopBlocking();

    // Write tracing recordings to the specified file
    if (!session->is_large_trace)
    {
        std::vector<char> trace_data(session->session->ReadTraceBlocking());
        std::ofstream output;
        output.open(session->write_to_file, std::ios::out | std::ios::binary);
        if (!output.is_open())
            return ffi::Fail(ffi::kErr, fmt::format("failed to open output file `{}`", session->write_to_file));
        output.write(&trace_data[0], static_cast<std::streamsize>(trace_data.size()));
        output.close();
    }
    else
    {
        // Perfetto has written the file already, and just close it.
        ::close(session->large_trace_fd);
    }

    return {};
}

std::unique_ptr<VMIntrospect> VMIntrospect::InstallGlobal(v8::Isolate *isolate)
{
    CHECK(isolate->InContext());
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    v8::Local<v8::Object> g = ctx->Global();

    ffi::DefineInterface<TracingConfig>(isolate, "TracingConfig")
        .Field("bufferSizeKb", &TracingConfig::buffer_size_kb)
        .Field("enables", &TracingConfig::enables)
        .Field("largeTrace", &TracingConfig::large_trace)
        .Field("writeToFile", &TracingConfig::write_to_file)
        .Finalize();

    ffi::DefineClass<VMIntrospect>(isolate)
        // The lifecycle of `VMIntrospect` is managed by runtime directly
        .OverrideDeleter(nullptr)
        .Method("setUncaughtExceptionHandler", &VMIntrospect::setUncaughtExceptionHandler)
        .Method("setBeforeExitHandler", &VMIntrospect::setBeforeExitHandler)
        .Method("setUnhandledPromiseRejectionHandler", &VMIntrospect::setUnhandledPromiseRejectionHandler)
        .Method("setPromiseMultipleResolveHandler", &VMIntrospect::setPromiseMultipleResolveHandler)
        .Method("scheduleScriptEval", &VMIntrospect::scheduleScriptEval)
        .Method("scheduleModuleUrlEval", &VMIntrospect::scheduleModuleUrlEval)
        .Method("print", &VMIntrospect::print)
        .Method("hasNativeModule", &VMIntrospect::hasNativeModule)
        .Method("rewind", &VMIntrospect::rewind)
        .Method("startProcessTracing", &VMIntrospect::startProcessTracing)
        .Method("finishProcessTracing", &VMIntrospect::finishProcessTracing)
        .Finalize();

    auto introspect = std::make_unique<VMIntrospect>(isolate);
    CHECK(introspect);

    g->Set(ctx, v8::String::NewFromUtf8Literal(isolate, "introspect"),
           ffi::JSObject::Wrap<VMIntrospect>(isolate, introspect.get())).Check();
    return introspect;
}

VMIntrospect::VMIntrospect(v8::Isolate *isolate)
    : isolate_(isolate)
{
}

VMIntrospect::~VMIntrospect()
{
    for (auto& item : callback_map_)
        item.second.Reset();
}

v8::MaybeLocal<v8::Function> VMIntrospect::GetCallbackFromSlot(CallbackSlot slot)
{
    if (!utils::MapContains(callback_map_, slot))
        return {};
    return callback_map_[slot].Get(isolate_);
}

namespace {

template<VMIntrospect::CallbackSlot kSlot, typename...ArgsT>
bool introspect_invoke_callback(VMIntrospect *this_, ArgsT&&...args)
{
    v8::Isolate *isolate = this_->GetIsolate();
    v8::HandleScope scope(isolate);

    v8::Local<v8::Function> cb;
    if (!this_->GetCallbackFromSlot(kSlot).ToLocal(&cb))
        return false;

    constexpr static int kLength = sizeof...(ArgsT);
    v8::Local<v8::Value> argv[] = {
            ffi::Cast<std::remove_reference_t<ArgsT>>::ToChecked(isolate, args)... };
    auto ctx = isolate->GetCurrentContext();
    (void) cb->Call(ctx, ctx->Global(),  kLength, argv);
    return true;
}

} // namespace anonymous

bool VMIntrospect::notifyUncaughtException(v8::Local<v8::Value> except)
{
    return introspect_invoke_callback<CallbackSlot::kUncaughtException>(this, except);
}

bool VMIntrospect::notifyBeforeExit()
{
    return introspect_invoke_callback<CallbackSlot::kBeforeExit>(this);
}

bool VMIntrospect::notifyUnhandledPromiseRejection(v8::Local<v8::Promise> promise, v8::Local<v8::Value> value)
{
    return introspect_invoke_callback<CallbackSlot::kUnhandledPromiseRejection>(this, promise, value);
}

bool VMIntrospect::notifyPromiseMultipleResolve(v8::Local<v8::Promise> promise, MultipleResolveAction action)
{
    std::string strAction;
    switch (action)
    {
    case MultipleResolveAction::kResolve:
        strAction = "resolve";
        break;
    case MultipleResolveAction::kReject:
        strAction = "reject";
        break;
    }
    return introspect_invoke_callback<CallbackSlot::kPromiseMultipleResolve>(this, promise, strAction);
}

VMIntrospect::PerformCheckpointResult VMIntrospect::performScheduledTasksCheckpoint()
{
    Runtime *rt = Runtime::GetBareFromIsolate(isolate_);
    v8::HandleScope scope(isolate_);

    v8::Local<v8::Context> ctx = isolate_->GetCurrentContext();
    while (!scheduled_task_queue_.empty())
    {
        ScheduledTask task(std::move(scheduled_task_queue_.front()));
        scheduled_task_queue_.pop();
        CHECK(task.type != ScheduledTask::Type::kInvalid);

        auto resolver = task.resolver.Get(isolate_);

        v8::MaybeLocal<v8::Value> maybe_value;
        v8::TryCatch catch_block(isolate_);

        if (task.type == ScheduledTask::Type::kEvalScript)
            maybe_value = rt->ExecuteScript("<anonymous@scheduled>", task.param.c_str());
        else if (task.type == ScheduledTask::Type::kEvalModuleUrl)
            maybe_value = rt->EvaluateModule(task.param);

        if (catch_block.HasCaught())
        {
            resolver->Reject(ctx, catch_block.Exception()).Check();
            continue;
        }

        resolver->Resolve(ctx, maybe_value.ToLocalChecked()).Check();
    }
    return PerformCheckpointResult::kOk;
}

GALLIUM_NS_END
