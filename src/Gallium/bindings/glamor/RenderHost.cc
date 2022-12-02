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


#include "include/core/SkTypeface.h"

#include "Core/Journal.h"
#include "Gallium/bindings/glamor/Exports.h"
#include "Gallium/bindings/glamor/PromiseHelper.h"
#include "Gallium/bindings/glamor/CanvasKitTransferContext.h"
#include "Gallium/Runtime.h"

#include "Glamor/RenderHost.h"

#include <utility>
#include "Glamor/RenderClient.h"
#include "Glamor/RenderClientObject.h"
#include "Glamor/RenderHostCreator.h"
#include "Glamor/RenderHostTaskRunner.h"
#include "Glamor/MaybeGpuObject.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Gallium.bindings.Glamor)

namespace {

void check_gl_context_init()
{
    if (!gl::GlobalScope::Ref().HasInitialized())
    {
        g_throw(Error, "GL context (RenderHost) has not been initialized yet");
    }
}

} // namespace anonymous

void RenderHostWrap::Initialize(v8::Local<v8::Object> info)
{
    if (gl::GlobalScope::Ref().HasInitialized())
    {
        // Multiple initialization is not allowed, but the context
        // can be initialized again after calling `Dispose`.
        g_throw(Error, "Multiple initializations for GL context");
    }

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();

    for (const char *field : {"name", "major", "minor", "patch"})
    {
        if (!info->Has(ctx, binder::to_v8(isolate, field)).FromMaybe(false))
            g_throw(TypeError, fmt::format("Missing \"{}\" property in ApplicationInfo", field));
    }

    gl::GlobalScope::ApplicationInfo appInfo{};
    appInfo.name =
        binder::from_v8<std::string>(isolate, info->Get(ctx, binder::to_v8(isolate, "name")).ToLocalChecked());
    std::get<0>(appInfo.version_triple) =
        binder::from_v8<int32_t>(isolate, info->Get(ctx, binder::to_v8(isolate, "major")).ToLocalChecked());
    std::get<1>(appInfo.version_triple) =
            binder::from_v8<int32_t>(isolate, info->Get(ctx, binder::to_v8(isolate, "minor")).ToLocalChecked());
    std::get<2>(appInfo.version_triple) =
            binder::from_v8<int32_t>(isolate, info->Get(ctx, binder::to_v8(isolate, "patch")).ToLocalChecked());

    gl::GlobalScope::Ref().Initialize(appInfo);
    QLOG(LOG_INFO, "RenderHost is initialized, application name %fg<gr>\"{}\"%reset", appInfo.name);

    auto canvas_transfer_context = CanvasKitTransferContext::Create(isolate);
    if (!canvas_transfer_context)
        g_throw(Error, "Failed to create a CanvasKit transfer context");

    gl::GlobalScope::Ref().SetExternalDataPointer(canvas_transfer_context.release(),
                                                  [](void *ptr) {
        std::unique_ptr<CanvasKitTransferContext> adopted(
                reinterpret_cast<CanvasKitTransferContext*>(ptr));
    });
}

void RenderHostWrap::SetTypefaceTransferCallback(v8::Local<v8::Value> func)
{
    check_gl_context_init();

    if (!func->IsFunction())
        g_throw(TypeError, "Argument `func' must be a callback function");

    auto *transfer_context = reinterpret_cast<CanvasKitTransferContext*>(
            gl::GlobalScope::Ref().GetExternalDataPointer());
    CHECK(transfer_context);

    transfer_context->SetReadBackJSFunction(
            v8::Local<v8::Function>::Cast(func));
}

void RenderHostWrap::Dispose()
{
    check_gl_context_init();

    gl::GlobalScope::Ref().Dispose();
    QLOG(LOG_INFO, "RenderHost is disposed");
}

v8::Local<v8::Value> RenderHostWrap::Connect(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    check_gl_context_init();

    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    if (info.Length() > 1)
    {
        g_throw(Error, "Invalid number of arguments, expecting 0 or 1 argument");
    }

    std::string name;
    if (info.Length() == 1)
        name = binder::from_v8<std::string>(isolate, info[0]);

    auto creator = gl::GlobalScope::Ref().GetRenderHost()->GetRenderHostCreator();

    using T = gl::Shared<gl::RenderClientObject>;
    auto pack = PromiseClosure::New(isolate, [](v8::Isolate *isolate, gl::RenderHostCallbackInfo &info) {
        auto obj = binder::Class<DisplayWrap>::create_object(isolate,
                                                             info.GetReturnValue<T>());
        auto *ptr = binder::Class<DisplayWrap>::unwrap_object(isolate, obj);
        ptr->setGCObjectSelfHandle(obj);

        return obj;
    });

    creator->Invoke(GLOP_RENDERHOSTCREATOR_CREATE_DISPLAY,
                    pack,
                    PromiseClosure::HostCallback,
                    name);

    return pack->getPromise();
}

void RenderHostWrap::WaitForSyncBarrier(int64_t timeout)
{
    check_gl_context_init();

    gl::GlobalScope::Ref().GetRenderHost()->WaitForSyncBarrier(timeout);
}

v8::Local<v8::Value> RenderHostWrap::SleepRendererFor(int64_t timeout)
{
    check_gl_context_init();

    if (timeout < 0)
        g_throw(Error, fmt::format("Invalid time for argument \'timeout\': {}", timeout));

    gl::RenderHost *host = gl::GlobalScope::Ref().GetRenderHost();
    auto runner = host->GetRenderHostTaskRunner();

    gl::RenderHostTaskRunner::Task task = [timeout]() -> std::any {
        std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
        return {};
    };

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto closure = PromiseClosure::New(isolate, nullptr);

    runner->Invoke(GLOP_TASKRUNNER_RUN, closure, PromiseClosure::HostCallback, task);

    return closure->getPromise();
}

namespace {

struct TraceResult
{
    std::string str;
};

} // namespace anonymous

v8::Local<v8::Value> RenderHostWrap::TraceGraphicsResources()
{
    check_gl_context_init();

    gl::RenderHost *host = gl::GlobalScope::Ref().GetRenderHost();

    auto trace_result = std::make_shared<TraceResult>();
    gl::RenderHostTaskRunner::Task task = [trace_result]() -> std::any {
        auto maybe = gl::GlobalScope::Ref().TraceResourcesToJson();
        if (!maybe)
        {
            // TaskRunner will catch and handle the thrown exception
            // appropriately by returning a wrong state in the asynchronous
            // invocation. Then the `Promise::HostCallback` handler will
            // detect that and reject the promise object automatically.
            throw std::runtime_error("Failed to trace graphics resources");
        }
        trace_result->str = *maybe;
        return {};
    };

    auto acceptor = [trace_result](v8::Isolate *isolate,
                                   gl::RenderHostCallbackInfo& info) {
        return binder::to_v8(isolate, trace_result->str);
    };

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto closure = PromiseClosure::New(isolate, acceptor);

    auto runner = host->GetRenderHostTaskRunner();
    runner->Invoke(GLOP_TASKRUNNER_RUN, closure, PromiseClosure::HostCallback, task);

    return closure->getPromise();
}

void RenderHostWrap::CollectCriticalSharedResources()
{
    check_gl_context_init();

    auto collector = gl::GlobalScope::Ref().GetGpuThreadSharedObjectsCollector();
    CHECK(collector);
    collector->Collect();
}

// ============================
// RenderClientObjectWrap
// ============================

RenderClientObjectWrap::RenderClientObjectWrap(gl::Shared<gl::RenderClientObject> object)
    : object_(std::move(object))
{
}

RenderClientObjectWrap::~RenderClientObjectWrap()
{
    slot_closures_map_.clear();
}

void RenderClientObjectWrap::defineSignal(const char *name, int32_t code, InfoAcceptor acceptor)
{
    CHECK(signal_name_map_.count(name) == 0 || acceptors_map_.count(code) == 0);
    signal_name_map_[name] = code;
    acceptors_map_[code] = std::move(acceptor);
}

int32_t RenderClientObjectWrap::getSignalCodeByName(const std::string& name)
{
    if (signal_name_map_.count(name) == 0)
        return -1;
    return signal_name_map_[name];
}

uint32_t RenderClientObjectWrap::connect(const std::string& name, v8::Local<v8::Function> callback)
{
    int32_t code = getSignalCodeByName(name);
    if (code < 0)
    {
        g_throw(Error, fmt::format("\'{}\' is not a valid signal name for slot to connect to", name));
    }

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto closure = SlotClosure::New(isolate, code, getObject(), callback, acceptors_map_[code]);
    uint32_t slot_id = closure->slot_id_;
    slot_closures_map_[closure->slot_id_] = std::move(closure);

    return slot_id;
}

void RenderClientObjectWrap::disconnect(uint32_t id)
{
    if (slot_closures_map_.count(id) == 0)
    {
        g_throw(Error, fmt::format("{} is not a valid slot ID", id));
    }
    slot_closures_map_.erase(id);
}

v8::Local<v8::Value> RenderClientObjectWrap::inspectObject()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    using RCO = gl::RenderClientObject;

    std::vector<v8::Local<v8::Object>> signals_array;
    for (const auto& signal : signal_name_map_)
    {
        std::vector<v8::Local<v8::Value>> callbacks;
        for (const auto& slot : slot_closures_map_)
        {
            if (slot.second->signal_code_ != signal.second)
                continue;
            callbacks.emplace_back(slot.second->callback_.Get(isolate));
        }

        std::map<std::string_view, v8::Local<v8::Value>> item{
            { "name", binder::to_v8(isolate, signal.first) },
            { "code", binder::to_v8(isolate, signal.second) },
            { "connectedCallbacks", binder::to_v8(isolate, callbacks) }
        };

        signals_array.emplace_back(binder::to_v8(isolate, item));
    }

    std::map<std::string_view, v8::Local<v8::Value>> inspect_result{
        { "objectType", binder::to_v8(isolate, RCO::GetTypeName(object_->GetRealType())) },
        { "signals", binder::to_v8(isolate, signals_array) }
    };

    return binder::to_v8(isolate, inspect_result);
}

GALLIUM_BINDINGS_GLAMOR_NS_END
