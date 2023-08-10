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
#include "Gallium/Runtime.h"

#include "Glamor/RenderHost.h"

#include "Glamor/RenderClient.h"
#include "Glamor/RenderClientObject.h"
#include "Glamor/RenderHostCreator.h"
#include "Glamor/RenderHostTaskRunner.h"
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

    using Sp = std::shared_ptr<gl::RenderClientObject>;
    return PromisifiedRemoteCall::Call(
            isolate, creator,
            [](v8::Isolate *isolate, gl::RenderHostCallbackInfo &info) {
                auto obj = binder::NewObject<DisplayWrap>(isolate, info.GetReturnValue<Sp>());
                auto *ptr = binder::UnwrapObject<DisplayWrap>(isolate, obj);
                ptr->setGCObjectSelfHandle(obj);
                return obj;
            },
            GLOP_RENDERHOSTCREATOR_CREATE_DISPLAY,
            name
    );
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
    return PromisifiedRemoteCall::Call(isolate, runner, {}, GLOP_TASKRUNNER_RUN, task);
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
            // invocation. Then the `PromisifiedRemoteCall::ResultCallback`
            // handler will detect that and reject the promise object automatically.
            throw std::runtime_error("Failed to trace graphics resources");
        }
        trace_result->str = *maybe;
        return {};
    };

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto runner = host->GetRenderHostTaskRunner();
    CHECK(runner);

    return PromisifiedRemoteCall::Call(
            isolate, runner,
            [trace_result](v8::Isolate *i, gl::RenderHostCallbackInfo& info) {
                return binder::to_v8(i, trace_result->str);
            },
            GLOP_TASKRUNNER_RUN,
            task
    );
}

void RenderHostWrap::CollectCriticalSharedResources()
{
    check_gl_context_init();

    auto collector = gl::GlobalScope::Ref().GetGpuThreadSharedObjectsCollector();
    CHECK(collector);
    collector->Collect();
}

GALLIUM_BINDINGS_GLAMOR_NS_END
