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

#include <iostream>
#include <memory>
#include <utility>
#include <thread>
#include <map>

#include "include/libplatform/libplatform.h"
#include "include/v8.h"

#include "Core/Exception.h"
#include "Core/Journal.h"
#include "Core/EventLoop.h"
#include "Core/QResource.h"
#include "Core/Data.h"
#include "Gallium/Gallium.h"
#include "Gallium/Runtime.h"
#include "Gallium/BindingManager.h"
#include "Gallium/ModuleImportURL.h"
#include "Gallium/Infrastructures.h"
#include "Gallium/Platform.h"
#include "Gallium/Inspector.h"

#include "Gallium/binder/Module.h"
#include "Gallium/binder/Class.h"
#include "Gallium/binder/CallV8.h"
#include "Gallium/bindings/Base.h"
GALLIUM_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Gallium.Runtime)

#define CASTJS(v) binder::to_v8(isolate, v)

Runtime::Options::Options()
    : v8_platform_thread_pool(0)
{
}

namespace {

v8::MaybeLocal<v8::Value>
create_synthetic_module_eval_step(v8::Local<v8::Context> context,
                                  v8::Local<v8::Module> module)
{
    v8::Isolate *isolate = context->GetIsolate();
    v8::EscapableHandleScope scope(isolate);
    auto *pRT = reinterpret_cast<Runtime*>(isolate->GetData(ISOLATE_DATA_SLOT_RUNTIME_PTR));

    bindings::BindingBase *binding = pRT->GetSyntheticModuleBinding(module);
    CHECK(binding);

    binding->onRegisterClasses(isolate);

    binder::Module bound_module(isolate);
    binding->onGetModule(bound_module);

    v8::Local<v8::Object> exports = bound_module.new_instance();
    exports->Set(context, CASTJS("__name__"), CASTJS(binding->name())).Check();
    exports->Set(context, CASTJS("__desc__"), CASTJS(binding->description())).Check();
    exports->Set(context, CASTJS("__unique_id__"), CASTJS(binding->onGetUniqueId())).Check();

    // Synthetic modules set their own specified dynamic properties here
    binding->onSetInstanceProperties(exports);

    // Store the `exports` object into the module cache entry
    Runtime *runtime = Runtime::GetBareFromIsolate(isolate);
    auto& module_cache_map = runtime->GetModuleCache();
    bool exports_object_stored = false;
    for (auto& cache : module_cache_map)
    {
        if (cache.second.module == module)
        {
            cache.second.setExportsObject(isolate, exports);
            exports_object_stored = true;
        }
    }
    CHECK(exports_object_stored);

    v8::Local<v8::Array> properties = CHECKED(exports->GetPropertyNames(context));
    for (uint32_t i = 0; i < properties->Length(); i++)
    {
        v8::Local<v8::Value> export_name_value = CHECKED(properties->Get(context, i));
        if (!export_name_value->IsString())
        {
            throw RuntimeException(__func__,
                fmt::format("Synthetic module {} has a non-string-named export", binding->name()));
        }

        v8::Local<v8::String> name = v8::Local<v8::String>::Cast(export_name_value);
        v8::Local<v8::Value> value = CHECKED(exports->Get(context, name));
        module->SetSyntheticModuleExport(isolate, name, value).Check();
    }

    return scope.EscapeMaybe(v8::MaybeLocal<v8::Value>(v8::True(isolate)));
}

v8::Local<v8::Module>
create_synthetic_module(v8::Isolate *isolate,
                        bindings::BindingBase *binding)
{
    v8::EscapableHandleScope scope(isolate);
    std::vector<v8::Local<v8::String>> exports{CASTJS("__name__"),
                                               CASTJS("__desc__"),
                                               CASTJS("__unique_id__")};
    for (const char **p = binding->onGetExports(); *p; p++)
        exports.emplace_back(CASTJS(*p));

    v8::Local<v8::Module> module = v8::Module::CreateSyntheticModule(isolate,
                                                                     CASTJS(binding->name()),
                                                                     exports,
                                                                     create_synthetic_module_eval_step);
    if (module.IsEmpty())
    {
        throw RuntimeException(__func__,
            fmt::format("Failed to create synthetic module \"{}\"", binding->name()));
    }
    return scope.Escape(module);
}

v8::StartupData *load_startup_snapshot()
{
    auto snapshot_data = QResource::Instance()->Lookup("org.cocoa.internal.v8",
                                                       "/snapshot_blob.bin");
    if (!snapshot_data)
    {
        QLOG(LOG_ERROR, "Failed to load snapshot data from package org.cocoa.internal.v8");
        return nullptr;
    }

    size_t size = snapshot_data->size();
    auto *startup_data = new v8::StartupData{
            .data = new char[size],
            .raw_size = static_cast<int>(size)
    };

    snapshot_data->read(const_cast<char*>(startup_data->data), size);
    v8::V8::SetSnapshotDataBlob(startup_data);

    return startup_data;
}

} // namespace anonymous

void Runtime::AdoptV8CommandOptions(const Options& options)
{
    for (const auto& arg : options.v8_options)
        v8::V8::SetFlagsFromString(arg.c_str(), arg.length());
}

std::shared_ptr<Runtime> Runtime::Make(EventLoop *loop, const Options& options)
{
    v8::StartupData *startup_data = load_startup_snapshot();
    if (!startup_data)
        return nullptr;

    ScopeExitAutoInvoker startup_data_releaser([startup_data] {
        delete[] startup_data->data;
        delete startup_data;
    });

    Options dump_options(options);
    if (dump_options.v8_platform_thread_pool <= 0)
    {
        dump_options.v8_platform_thread_pool = static_cast<int32_t>(
                std::thread::hardware_concurrency());
    }

    // std::unique_ptr<v8::Platform> platform =
    //        v8::platform::NewDefaultPlatform(static_cast<int>(options.v8_platform_thread_pool),
    //                                         v8::platform::IdleTaskSupport::kEnabled);
    auto tracing_controller = std::make_unique<v8::TracingController>();
    std::unique_ptr<Platform> platform = Platform::Make(loop,
                                                        dump_options.v8_platform_thread_pool,
                                                        tracing_controller.get());

    v8::V8::InitializePlatform(platform.get());
    v8::V8::Initialize();

    v8::Isolate::CreateParams params;
    // TODO: Implement a more efficient allocator
    params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();


    v8::Isolate *isolate = v8::Isolate::Allocate();
    platform->RegisterIsolate(isolate);

    v8::Isolate::Initialize(isolate, params);
    isolate->SetMicrotasksPolicy(v8::MicrotasksPolicy::kExplicit);
    std::shared_ptr<Runtime> runtime;

    {
        v8::Isolate::Scope isolate_scope(isolate);
        v8::HandleScope scope(isolate);

        v8::Local<v8::Context> context = v8::Context::New(isolate);
        v8::Context::Scope context_scope(context);

        // Create inspector if required
        std::unique_ptr<Inspector> inspector;
        if (options.start_with_inspector)
        {
            inspector = std::make_unique<Inspector>(loop->handle(),
                                                    isolate,
                                                    context,
                                                    options.inspector_port);
        }

        runtime = std::make_shared<Runtime>(loop,
                                            nullptr,
                                            std::move(tracing_controller),
                                            std::move(platform),
                                            params.array_buffer_allocator,
                                            isolate,
                                            v8::Global<v8::Context>(isolate, context),
                                            std::move(inspector),
                                            options);
        startup_data_releaser.cancel();

        auto isolateGuard = std::make_unique<GlobalIsolateGuard>(runtime);
        runtime->SetGlobalIsolateGuard(std::move(isolateGuard));

        BindingManager::NotifyIsolateHasCreated(isolate);
    }

    return runtime;
}

Runtime *Runtime::GetBareFromIsolate(v8::Isolate *isolate)
{
    auto *bare = reinterpret_cast<Runtime*>(isolate->GetData(ISOLATE_DATA_SLOT_RUNTIME_PTR));
    CHECK(bare != nullptr);
    return bare;
}

Runtime::Runtime(EventLoop *loop,
                 v8::StartupData *startupData,
                 std::unique_ptr<v8::TracingController> tracing_controller,
                 std::unique_ptr<Platform> platform,
                 v8::ArrayBuffer::Allocator *allocator,
                 v8::Isolate *isolate,
                 v8::Global<v8::Context> context,
                 std::unique_ptr<Inspector> inspector,
                 Options opts)
    : PrepareSource(loop)
    , CheckSource(loop)
    , options_(std::move(opts))
    , startup_data_(startupData)
    , tracing_controller_(std::move(tracing_controller))
    , platform_(std::move(platform))
    , array_buffer_allocator_(allocator)
    , isolate_(isolate)
    , inspector_(std::move(inspector))
    , context_(std::move(context))
    , resolved_promises_(0)
    , idle_{}
{
    v8::Isolate::Scope isolateScope(isolate_);
    v8::HandleScope handleScope(isolate_);
    v8::Context::Scope ctxScope(this->GetContext());

    isolate_->SetData(ISOLATE_DATA_SLOT_RUNTIME_PTR, this);
    isolate_->SetPromiseHook(Runtime::PromiseHookCallback);
    isolate_->SetHostImportModuleDynamicallyCallback(Runtime::DynamicImportHostCallback);

    PrepareSource::startPrepare();
    PrepareSource::unrefEventSource();
    CheckSource::startCheck();
    CheckSource::unrefEventSource();

    infra::InstallOnGlobalContext(isolate_, this->GetContext());
    if (options_.rt_expose_introspect)
        introspect_ = VMIntrospect::InstallGlobal(isolate_);

    uv_idle_init(loop->handle(), &idle_);
}

Runtime::~Runtime()
{
    uv_close((uv_handle_t *)&idle_, nullptr);

    QLOG(LOG_DEBUG, "Imported modules (URL):");
    for (auto& module : module_cache_)
    {
        QLOG(LOG_DEBUG, "  %fg<cyan,hl>{}%reset", module.first->toString());
        module.second.reset();
    }

    ModuleImportURL::FreeInternalCaches();
    binder::Cleanup(isolate_);

    introspect_.reset();

    context_.Reset();
    isolate_guard_.reset();

    platform_->UnregisterIsolate(isolate_);
    isolate_->Dispose();

    v8::V8::Dispose();
    v8::V8::DisposePlatform();

    delete array_buffer_allocator_;

    platform_.reset();

    if (startup_data_)
    {
        delete[] startup_data_->data;
        delete startup_data_;
    }
}

bindings::BindingBase *Runtime::GetSyntheticModuleBinding(v8::Local<v8::Module> module)
{
    for (auto& pair : module_cache_)
    {
        if (pair.second.module == module)
            return pair.second.binding;
    }
    return nullptr;
}

Runtime::ModuleCacheMap& Runtime::GetModuleCache()
{
    return module_cache_;
}

v8::MaybeLocal<v8::Module> Runtime::GetAndCacheSyntheticModule(const ModuleImportURL::SharedPtr& url)
{
    v8::EscapableHandleScope scope(isolate_);
    if (url->getProtocol() != ModuleImportURL::Protocol::kSynthetic)
        return {};
    for (auto& cache : module_cache_)
    {
        if (*cache.first == *url)
            return scope.Escape(cache.second.module.Get(isolate_));
    }
    bindings::BindingBase *binding = url->getSyntheticBinding();
    v8::Local<v8::Module> module = create_synthetic_module(isolate_, binding);
    module_cache_[url] = ESModuleCache(isolate_, module, binding);
    return scope.Escape(module);
}

v8::Local<v8::Object> Runtime::getSyntheticModuleExportObject(v8::Local<v8::Module> module)
{
    for (auto& cache : module_cache_)
    {
        if (cache.second.module == module)
            return cache.second.exports.IsEmpty()
                    ? v8::Local<v8::Object>()
                    : cache.second.exports.Get(isolate_);
    }
    return {};
}

v8::Local<v8::Module> Runtime::CompileModule(const ModuleImportURL::SharedPtr& referer,
                                             const std::string& url,
                                             bool isImport,
                                             bool isSysInvoke)
{
    v8::Isolate::Scope isolateScope(isolate_);
    v8::EscapableHandleScope handleScope(isolate_);
    v8::Context::Scope contextScope(this->GetContext());

    ModuleImportURL::ResolvedAs resolvedAs;
    if (isImport)
    {
        if (isSysInvoke)
            resolvedAs = ModuleImportURL::ResolvedAs::kSysImport;
        else
            resolvedAs = ModuleImportURL::ResolvedAs::kUserImport;
    }
    else
    {
        if (isSysInvoke)
            resolvedAs = ModuleImportURL::ResolvedAs::kSysExecute;
        else
            resolvedAs = ModuleImportURL::ResolvedAs::kUserExecute;
    }

    ModuleImportURL::SharedPtr resolved = ModuleImportURL::Resolve(referer, url, resolvedAs);
    if (!resolved)
        throw RuntimeException(__func__, "Failed to resolve module path \"" + url + "\"");
    for (auto& cache : module_cache_)
    {
        if (*cache.first == *resolved)
            return handleScope.Escape(cache.second.module.Get(isolate_));
    }

    // Synthetic modules don't need to be compiled
    if (resolved->getProtocol() == ModuleImportURL::Protocol::kSynthetic)
    {
        v8::Local<v8::Module> module = GetAndCacheSyntheticModule(resolved).ToLocalChecked();
        return handleScope.Escape(module);
    }

    v8::ScriptOrigin scriptOrigin(isolate_,
                                  binder::to_v8(isolate_, resolved->toString()),
                                  0,
                                  0,
                                  false,
                                  -1,
                                  v8::Local<v8::Value>(),
                                  false,
                                  false,
                                  true);

    v8::ScriptCompiler::Source source(binder::to_v8(isolate_, *resolved->loadResourceText()),
                                      scriptOrigin);
    v8::Local<v8::Module> module;
    v8::TryCatch tryCatch(isolate_);
    if (!v8::ScriptCompiler::CompileModule(isolate_, &source).ToLocal(&module))
    {
        if (tryCatch.HasCaught())
        {
            auto message = binder::from_v8<std::string>(isolate_, tryCatch.Message()->Get());
            QLOG(LOG_ERROR, "Failed to compile ES6 module {}: {}", resolved->toString(), message);
        }
        throw RuntimeException(__func__, "Failed to compile ES6 module \"" + resolved->toString() + "\"");
    }

    module_cache_[resolved] = ESModuleCache(isolate_, module);
    return handleScope.Escape(module);
}

namespace {

v8::MaybeLocal<v8::Module>
instantiate_module_callback(v8::Local<v8::Context> context,
                            v8::Local<v8::String> specifier,
                            g_maybe_unused v8::Local<v8::FixedArray> assertions,
                            v8::Local<v8::Module> referer)
{
    auto *pRT = reinterpret_cast<Runtime*>(context->GetIsolate()->GetData(ISOLATE_DATA_SLOT_RUNTIME_PTR));
    for (auto& cache : pRT->GetModuleCache())
    {
        if (cache.second.module != referer)
            continue;
        auto url = binder::from_v8<std::string>(pRT->GetIsolate(), specifier);

        // FIXME(sora): propagate `isSysInvoke`
        v8::MaybeLocal<v8::Module> maybe_module = pRT->CompileModule(cache.first, url, true, false);
        QLOG(LOG_DEBUG, "Resolved ES module {} (from {})", url, cache.first->toString());

        return maybe_module;
    }
    return {};
}
} // namespace anonymous

v8::MaybeLocal<v8::Value> Runtime::EvaluateModule(const std::string& url,
                                                  v8::Local<v8::Module> *outModule,
                                                  const std::shared_ptr<ModuleImportURL> &referer,
                                                  bool isImport, bool isSysInvoke)
{
    v8::Local<v8::Module> module = this->CompileModule(referer, url, isImport, isSysInvoke);
    if (outModule)
        *outModule = module;
    if (module->GetStatus() == v8::Module::Status::kInstantiated
        || module->GetStatus() == v8::Module::Status::kEvaluated
        || module->GetStatus() == v8::Module::Status::kErrored)
    {
        return module->Evaluate(this->GetContext());
    }

    v8::TryCatch caught(isolate_);
    v8::Maybe<bool> instantiate = module->InstantiateModule(this->GetContext(),
                                                            instantiate_module_callback);

    if (instantiate.IsNothing())
    {
        if (caught.HasCaught())
        {
            auto what = binder::from_v8<std::string>(
                    isolate_, caught.Exception()->ToString(GetContext()).ToLocalChecked());
            QLOG(LOG_ERROR, "%fg<re>Evaluation: {}%reset", what);
        }
        throw RuntimeException(__func__, "Could not instantiate ES6 module " + url);
    }

    v8::MaybeLocal<v8::Value> result = module->Evaluate(this->GetContext());
    UpdateIdleRequirementByPromiseCounter();

    return result;
}

namespace {

std::shared_ptr<ModuleImportURL> search_referrer_url_info_in_cache(v8::Isolate *isolate,
                                                                   const std::string& referrer_url)
{
    Runtime *runtime = Runtime::GetBareFromIsolate(isolate);

    auto& module_cache = runtime->GetModuleCache();
    auto referrer_module_cache_itr = std::find_if(module_cache.begin(), module_cache.end(),
        [&referrer_url](Runtime::ModuleCacheMap::value_type& pair) {
            return (pair.first->toString() == referrer_url);
        });

    if (referrer_module_cache_itr == module_cache.end())
        return nullptr;

    return referrer_module_cache_itr->first;
}

} // namespace anonymous

v8::MaybeLocal<v8::Promise>
Runtime::DynamicImportHostCallback(v8::Local<v8::Context> context,
                                   g_maybe_unused v8::Local<v8::Data> host_defined_options,
                                   v8::Local<v8::Value> resource_name,
                                   v8::Local<v8::String> specifier,
                                   g_maybe_unused v8::Local<v8::FixedArray> import_assertions)
{
    v8::Isolate *isolate = context->GetIsolate();
    v8::EscapableHandleScope scope(isolate);

    v8::Local<v8::Promise::Resolver> resolver =
            v8::Promise::Resolver::New(context).ToLocalChecked();
    v8::MaybeLocal<v8::Promise> promise(resolver->GetPromise());

    if (resource_name->IsNullOrUndefined())
    {
        resolver->Reject(context,
                         binder::to_v8(isolate, "Dynamic import: resource name of referrer is undefined"))
                         .Check();
        return scope.EscapeMaybe(promise);
    }

    Runtime *runtime = Runtime::GetBareFromIsolate(isolate);

    // `referrer_url_info` may be nullptr if V8 gives us a non-cached referrer module URL.
    // (e.g. import(...) from a normal script or REPL statements from inspector).
    // In that case, we resolve the specifier URL without relative path.
    std::shared_ptr<ModuleImportURL> referrer_url_info = search_referrer_url_info_in_cache(isolate,
        binder::from_v8<std::string>(isolate, resource_name));

    auto specifier_url = binder::from_v8<std::string>(isolate, specifier);
    v8::Local<v8::Module> module;
    try
    {
        v8::Local<v8::Value> value;

        if (!runtime->EvaluateModule(specifier_url, &module, referrer_url_info, true).ToLocal(&value))
            throw RuntimeException(__func__, fmt::format("Error evaluating module {}", specifier_url));
    }
    catch (const std::exception& e)
    {
        v8::Local<v8::Value> message = binder::to_v8(isolate,
                fmt::format("dynamic import: {}", e.what()));
        resolver->Reject(context, message).Check();
        return scope.EscapeMaybe(promise);
    }

    QLOG(LOG_DEBUG, "Resolved ES module {} (from {}, dynamically)",
         specifier_url, referrer_url_info ? referrer_url_info->toString() : "<unknown>");

    resolver->Resolve(context, module->GetModuleNamespace()).Check();
    return scope.EscapeMaybe(promise);
}

v8::MaybeLocal<v8::Value> Runtime::ExecuteScript(const char *scriptName, const char *str)
{
    v8::Isolate::Scope isolateScope(isolate_);
    v8::EscapableHandleScope handleScope(isolate_);
    v8::Context::Scope contextScope(this->GetContext());

    v8::Local<v8::String> sourceCode = v8::String::NewFromUtf8(isolate_, str).ToLocalChecked();
    v8::Local<v8::String> scriptNameS = v8::String::NewFromUtf8(isolate_, scriptName).ToLocalChecked();
    v8::ScriptOrigin origin(isolate_,
                            scriptNameS,
                            0,
                            0,
                            false,
                            -1,
                            v8::Local<v8::Value>(),
                            false,
                            false,
                            false);
    v8::ScriptCompiler::Source source(sourceCode, origin);
    v8::Local<v8::Script> script;

    if (!v8::ScriptCompiler::Compile(this->GetContext(), &source).ToLocal(&script))
        return {};

    v8::MaybeLocal<v8::Value> result = script->Run(this->GetContext());
    UpdateIdleRequirementByPromiseCounter();

    return handleScope.EscapeMaybe(result);
}

KeepInLoop Runtime::prepareDispatch()
{
    UpdateIdleRequirementByPromiseCounter();
    return KeepInLoop::kYes;
}

KeepInLoop Runtime::checkDispatch()
{
    PerformTasksCheckpoint();
    UpdateIdleRequirementByPromiseCounter();
    return KeepInLoop::kYes;
}

void Runtime::PerformTasksCheckpoint()
{
    /**
     * We must set `resolved_promises_` counter to zero before `PerformMicrotaskCheckpoint`
     * instead of setting it after that function, because `PerformMicrotaskCheckpoint`
     * may resolve some pending promises.
     */
    resolved_promises_ = 0;
    isolate_->PerformMicrotaskCheckpoint();
    isolate_guard_->performUnhandledRejectPromiseCheck();

    // TODO(Important): Where should it be placed?
    if (introspect_)
        introspect_->performScheduledTasksCheckpoint();
}

void Runtime::PromiseHookCallback(v8::PromiseHookType type,
                                  v8::Local<v8::Promise> promise,
                                  g_maybe_unused v8::Local<v8::Value> parent)
{
    Runtime *runtime = Runtime::GetBareFromIsolate(promise->GetIsolate());
    if (type == v8::PromiseHookType::kResolve)
    {
        runtime->resolved_promises_++;
    }
}

void Runtime::UpdateIdleRequirementByPromiseCounter()
{
    if (resolved_promises_ > 0)
        uv_idle_start(&idle_, [](uv_idle_t *) {});
    else
        uv_idle_stop(&idle_);
}

void Runtime::ReportUncaughtExceptionInCallback(const v8::TryCatch& catchBlock)
{
    isolate_guard_->reportUncaughtExceptionFromCallback(catchBlock);
}

void Runtime::NotifyRuntimeWillExit()
{
    if (introspect_)
        introspect_->notifyBeforeExit();
}

void Runtime::DrainPlatformTasks()
{
    platform_->DrainTasks(isolate_);
    PerformTasksCheckpoint();
}

void Runtime::RunWithMainLoop()
{
    v8::HandleScope handle_scope(isolate_);

    EvaluateModule("internal:///bootstrap.js", nullptr, nullptr, false, true);

    if (!options_.start_with_inspector)
    {
        // Inspector is not enabled, and we can evaluate the startup script
        // immediately.
        v8::Local<v8::Value> result;
        if (!EvaluateModule(options_.startup).ToLocal(&result))
            return;
    }
    else
    {
        inspector_->WaitForConnection();
        if (!options_.inspector_no_script)
        {
            // Startup script should not be executed here immediately.
            // Instead, it should be executed when the inspector frontend
            // notifies us to do that.
            inspector_->ScheduleModuleEvaluation(options_.startup);
        }
    }

    // Inspector messages from frontend will also be handled
    // in the event loop.
    EventLoop::Ref().spin([this] { this->DrainPlatformTasks(); });
}

void Runtime::RegisterExternalValueHolder(BinderExtValueHolderBase *value)
{
    CHECK(value);

    auto itr = std::find(binder_external_value_holders_.begin(),
                         binder_external_value_holders_.end(),
                         value);
    if (itr != binder_external_value_holders_.end())
        return;

    binder_external_value_holders_.emplace_back(value);
}

void Runtime::UnregisterExternalValueHolder(BinderExtValueHolderBase *value)
{
    CHECK(value);

    auto itr = std::find(binder_external_value_holders_.begin(),
                         binder_external_value_holders_.end(),
                         value);
    CHECK(itr != binder_external_value_holders_.end());
    binder_external_value_holders_.erase(itr);
}

void Runtime::DeleteExternalValueHolders()
{
    // The value holder removes itself when it is destructed (see binder/Function.h)
    // through `UnregisterExternalValueHolder`. It is essential to copy a list
    // for iteration.
    auto list = binder_external_value_holders_;

    for (BinderExtValueHolderBase *value_holder : list)
    {
        CHECK(value_holder);
        delete value_holder;
    }
}

GALLIUM_NS_END
