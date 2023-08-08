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

#include "fmt/format.h"

#include "Gallium/bindings/workers/WorkerRuntime.h"
#include "Gallium/bindings/workers/MessagePort.h"
#include "Gallium/Infrastructures.h"
GALLIUM_BINDINGS_WORKERS_NS_BEGIN

WorkerRuntime::WorkerRuntime(uint32_t thread_id,
                             uv_loop_t *event_loop,
                             std::shared_ptr<Platform> platform,
                             std::shared_ptr<MessagePort> port)
    : RuntimeBase(event_loop, std::move(platform), fmt::format("Runtime@Worker#{}", thread_id))
    , message_port_(std::move(port))
{
}

void WorkerRuntime::OnInitialize(v8::Isolate *isolate, v8::Local<v8::Context> context)
{
    infra::InstallOnGlobalContext(isolate, context, true);

    isolate->SetCaptureStackTraceForUncaughtExceptions(true, 50, v8::StackTrace::kDetailed);

    // Import `workers` synthetic module in worker runtime.
    // Exportable classes are registered after this call so that `binder::NewObject`
    // can be used to create a `MessagePortWrap` object.
    CHECK(!GetAndCacheSyntheticModule(ModuleImportURL::Resolve(
            nullptr, "workers", ModuleImportURL::ResolvedAs::kSysImport)).IsEmpty());

    auto global = context->Global();
    global->Set(context,
                binder::to_v8(isolate, "port"),
                binder::NewObject<MessagePortWrap>(isolate, message_port_))
                .Check();
}

void WorkerRuntime::OnReportUncaughtExceptionInCallback(const v8::TryCatch& try_catch)
{
    infra::ReportUncaughtException(GetIsolate(),
                                   try_catch.Message(),
                                   try_catch.Exception());
}

GALLIUM_BINDINGS_WORKERS_NS_END
