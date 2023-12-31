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

#include <memory>
#include <utility>
#include <thread>

#include "include/libplatform/libplatform.h"
#include "include/v8.h"

#include "Core/Journal.h"
#include "Core/EventLoop.h"
#include "Gallium/Gallium.h"
#include "Gallium/Runtime.h"
#include "Gallium/ModuleImportURL.h"
#include "Gallium/Infrastructures.h"
#include "Gallium/Platform.h"
#include "Gallium/Inspector.h"
#include "Gallium/TracingController.h"

#include "Gallium/binder/Module.h"
#include "Gallium/binder/Class.h"
#include "Gallium/bindings/Base.h"
GALLIUM_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Gallium.Runtime)

Runtime::Options::Options()
    : v8_platform_thread_pool(0)
{
}

void Runtime::AdoptV8CommandOptions(const Options& options)
{
    for (const auto& arg : options.v8_options)
        v8::V8::SetFlagsFromString(arg.c_str(), arg.length());
}

std::shared_ptr<Runtime> Runtime::Make(EventLoop *loop, const Options& options)
{
    Options dump_options(options);
    if (dump_options.v8_platform_thread_pool <= 0)
    {
        dump_options.v8_platform_thread_pool = static_cast<int32_t>(
                std::thread::hardware_concurrency());
    }

    // std::unique_ptr<v8::Platform> platform =
    //        v8::platform::NewDefaultPlatform(static_cast<int>(options.v8_platform_thread_pool),
    //                                         v8::platform::IdleTaskSupport::kEnabled);
    auto tracing_controller = std::make_unique<TracingController>();
    std::shared_ptr<Platform> platform = Platform::Make(loop,
                                                        dump_options.v8_platform_thread_pool,
                                                        std::move(tracing_controller));

    v8::V8::InitializePlatform(platform.get());
    v8::V8::Initialize();

    auto runtime = std::make_shared<Runtime>(loop, std::move(platform), options);
    runtime->Initialize();

    return runtime;
}

Runtime *Runtime::GetBareFromIsolate(v8::Isolate *isolate)
{
    // NOLINTNEXTLINE
    return static_cast<Runtime*>(RuntimeBase::FromIsolate(isolate));
}

Runtime::Runtime(EventLoop *loop, std::shared_ptr<Platform> platform, Options opts)
    : RuntimeBase(loop->handle(), std::move(platform), "Runtime@Main")
    , options_(std::move(opts))
{
}

Runtime::~Runtime() = default;

void Runtime::OnInitialize(v8::Isolate *isolate, v8::Local<v8::Context> context)
{
    infra::InstallOnGlobalContext(isolate, context, false);
    if (options_.rt_expose_introspect)
        introspect_ = VMIntrospect::InstallGlobal(isolate);

    isolate_guard_ = std::make_unique<GlobalIsolateGuard>(this);

    if (options_.start_with_inspector)
    {
        inspector_ = std::make_unique<Inspector>(GetEventLoop(),
                                                 isolate,
                                                 context,
                                                 options_.inspector_port);
    }
}

void Runtime::OnPreDispose()
{
    inspector_.reset();
    introspect_.reset();
    isolate_guard_.reset();
}

void Runtime::OnPostDispose()
{
    ModuleImportURL::FreeInternalCaches();
    v8::V8::Dispose();
    v8::V8::DisposePlatform();
}

void Runtime::OnPostPerformTasksCheckpoint()
{
    isolate_guard_->performUnhandledRejectPromiseCheck();

    // TODO(Important): Where should it be placed?
    if (introspect_)
        introspect_->performScheduledTasksCheckpoint();
}

void Runtime::OnReportUncaughtExceptionInCallback(const v8::TryCatch& catch_block)
{
    isolate_guard_->reportUncaughtExceptionFromCallback(catch_block);
}

void Runtime::NotifyRuntimeWillExit()
{
    if (introspect_)
        introspect_->notifyBeforeExit();
}

void Runtime::RunWithMainLoop()
{
    v8::HandleScope handle_scope(GetIsolate());

    EvaluateModule("internal:///bootstrap.js", nullptr, nullptr, kSysInvoke_ScriptSourceFlag);

    if (!options_.start_with_inspector ||
        (options_.start_with_inspector && !options_.inspector_no_script))
    {
        auto eval_main_script = [url = options_.startup, this]() {
            v8::Isolate *isolate = GetIsolate();
            v8::HandleScope handle_scope(isolate);
            v8::TryCatch cache_block(isolate);
            EvaluateModule(url);
            if (cache_block.HasCaught())
                ReportUncaughtExceptionInCallback(cache_block);
        };

        if (options_.start_with_inspector)
            inspector_->ScheduleModuleEvalOnNextConnect(std::move(eval_main_script));
        else
            eval_main_script();
    }

    SpinRun();
}

GALLIUM_NS_END
