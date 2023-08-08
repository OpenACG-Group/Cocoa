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

#include <utility>

#include "Core/Journal.h"
#include "Core/TraceEvent.h"

#include "Gallium/binder/Class.h"
#include "Gallium/binder/Convert.h"
#include "Gallium/bindings/Base.h"
#include "Gallium/BindingManager.h"
#include "Gallium/RuntimeBase.h"
GALLIUM_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Gallium.RuntimeBase)

RuntimeBase::RuntimeBase(uv_loop_t *event_loop,
                         std::shared_ptr<Platform> platform,
                         std::string runtime_id)
    : runtime_id_(std::move(runtime_id))
    , disposed_(false)
    , event_loop_(event_loop)
    , platform_(std::move(platform))
    , isolate_(nullptr)
    , context_()
    , event_check_(event_loop)
    , event_prepare_(event_loop)
    , event_idle_(event_loop)
    , nb_pending_resolved_promises_(0)
{
    event_check_.Unref();
    event_prepare_.Unref();
}

RuntimeBase::~RuntimeBase()
{
    CHECK(disposed_ && "Runtime must be disposed before destruction");
}

namespace {

std::shared_ptr<ModuleImportURL>
search_referrer_url_info_in_cache(v8::Isolate *isolate,
                                  const std::string& referrer_url)
{
    auto *runtime_base = RuntimeBase::FromIsolate(isolate);

    auto& module_cache = runtime_base->GetModuleCache();
    auto referrer_module_cache_itr = std::find_if(module_cache.begin(), module_cache.end(),
                                                  [&referrer_url](Runtime::ModuleCacheMap::value_type& pair) {
                                                      return (pair.first->toString() == referrer_url);
                                                  });

    if (referrer_module_cache_itr == module_cache.end())
        return nullptr;

    return referrer_module_cache_itr->first;
}

v8::MaybeLocal<v8::Promise>
dynamic_import_handler(v8::Local<v8::Context> context,
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

    auto *runtime_base = RuntimeBase::FromIsolate(isolate);

    // `referrer_url_info` may be nullptr if V8 gives us a non-cached referrer module URL.
    // (e.g. import(...) from a normal script or REPL statements from inspector).
    // In that case, we resolve the specifier URL without relative path.
    std::shared_ptr<ModuleImportURL> referrer_url_info = search_referrer_url_info_in_cache(
            isolate,
            binder::from_v8<std::string>(isolate, resource_name));

    auto specifier_url = binder::from_v8<std::string>(isolate, specifier);
    v8::Local<v8::Module> module;
    try
    {
        v8::Local<v8::Value> value;

        constexpr int FLAG = RuntimeBase::kFromImport_ScriptSourceFlag;
        if (!runtime_base->EvaluateModule(specifier_url, &module, referrer_url_info, FLAG).ToLocal(&value))
            throw std::runtime_error(fmt::format("Error evaluating module {}", specifier_url));
    }
    catch (const std::exception& e)
    {
        v8::Local<v8::Value> message = binder::to_v8(isolate,
                                                     fmt::format("dynamic import: {}", e.what()));
        resolver->Reject(context, message).Check();
        return scope.EscapeMaybe(promise);
    }

    QLOG(LOG_DEBUG, "Resolved JavaScript module {} (from {}, dynamically)",
         specifier_url, referrer_url_info ? referrer_url_info->toString() : "<unknown>");

    resolver->Resolve(context, module->GetModuleNamespace()).Check();
    return scope.EscapeMaybe(promise);
}

void import_meta_resolve_impl(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    v8::HandleScope scope(info.GetIsolate());
    v8::Isolate *isolate = info.GetIsolate();

    if (info.Length() != 1)
    {
        // `g_throw()` must not be used here as the function is not called
        // from binder (it is called from V8 directly).
        isolate->ThrowError("Invalid number of arguments, requires 2 arguments");
        return;
    }

    if (!info[0]->IsString())
    {
        isolate->ThrowError("Argument `url` is not a string");
        return;
    }

    auto url = binder::from_v8<std::string>(isolate, info[0].As<v8::String>());

    auto external = info.Data().As<v8::External>();
    auto *import_url = reinterpret_cast<ModuleImportURL*>(external->Value());

    std::shared_ptr<ModuleImportURL> resolved;
    try
    {
        // `Resolve` may throw an exception when an internal script
        // is not found.
        resolved = ModuleImportURL::Resolve(
                import_url, url, ModuleImportURL::ResolvedAs::kUserImport);
    }
    catch (const std::exception& except)
    {
        info.GetReturnValue().SetNull();
        return;
    }

    if (!resolved)
    {
        info.GetReturnValue().SetNull();
        return;
    }

    info.GetReturnValue().Set(v8::String::NewFromUtf8(
            isolate, resolved->toString().c_str()).ToLocalChecked());
}

void on_init_import_meta_object(v8::Local<v8::Context> context,
                                v8::Local<v8::Module> module,
                                v8::Local<v8::Object> meta)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    RuntimeBase *runtime = RuntimeBase::FromIsolate(isolate);

    v8::HandleScope scope(isolate);

    Runtime::ModuleCacheMap& cache_map = runtime->GetModuleCache();
    std::shared_ptr<ModuleImportURL> import_url;
    for (const auto& cache_pair : cache_map)
    {
        if (cache_pair.second.module == module)
        {
            import_url = cache_pair.first;
            break;
        }
    }

    if (!import_url)
    {
        QLOG(LOG_ERROR, "Failed to set `import.meta`: module not found in the cache");
        return;
    }

    auto url_prop_name = v8::String::NewFromUtf8Literal(isolate, "url");
    auto url_str = v8::String::NewFromUtf8(isolate, import_url->toString().c_str()).ToLocalChecked();

    if (!meta->CreateDataProperty(context, url_prop_name, url_str).FromMaybe(false))
    {
        QLOG(LOG_ERROR, "Failed to set property `url` on `import.meta` object");
        return;
    }

    // The module cache keeps valid during the whole lifetime of JavaScript
    // execution, so it is safe to expose a bare pointer to `v8::External`.
    auto import_url_data = v8::External::New(isolate, import_url.get());
    auto resolve_cb = v8::Function::New(context,
                                        import_meta_resolve_impl,
                                        import_url_data)
            .ToLocalChecked();

    auto resolve_prop_name = v8::String::NewFromUtf8Literal(isolate, "resolve");
    if (!meta->CreateDataProperty(context, resolve_prop_name, resolve_cb).FromMaybe(false))
        QLOG(LOG_ERROR, "Failed to set property `resolve` on `import.meta` object");
}

} // namespace anonymous

void RuntimeBase::Initialize()
{
    isolate_ = v8::Isolate::Allocate();
    CHECK(isolate_);
    platform_->RegisterIsolate(isolate_);

    v8::Isolate::CreateParams params;
    params.array_buffer_allocator_shared = std::shared_ptr<v8::ArrayBuffer::Allocator>(
            v8::ArrayBuffer::Allocator::NewDefaultAllocator());
    v8::Isolate::Initialize(isolate_, params);

    isolate_->SetData(ISOLATE_DATA_SLOT_RUNTIME_PTR, this);

    isolate_->SetMicrotasksPolicy(v8::MicrotasksPolicy::kExplicit);

    {
        v8::Isolate::Scope isolate_scope(isolate_);
        v8::HandleScope handle_scope(isolate_);
        v8::Local<v8::Context> context = v8::Context::New(isolate_);
        context_.Reset(isolate_, context);
    }

    isolate_->SetHostImportModuleDynamicallyCallback(dynamic_import_handler);
    isolate_->SetHostInitializeImportMetaObjectCallback(on_init_import_meta_object);
    isolate_->SetPromiseHook(promise_hook);

    event_prepare_.Start([&] {
        PerformIdleEventCheckpoint();
    });

    event_check_.Start([&] {
        PerformTasksCheckpoint();
        PerformIdleEventCheckpoint();
    });

    v8::Isolate::Scope isolate_scope(isolate_);
    v8::HandleScope handle_scope(isolate_);
    v8::Context::Scope context_scope(GetContext());
    OnInitialize(isolate_, GetContext());

    external_callbacks_.CallGroup(ExternalCallbackType::kAfterRuntimeInitialize);
}

void RuntimeBase::Dispose()
{
    if (disposed_)
        return;

    external_callbacks_.CallGroup(ExternalCallbackType::kBeforeRuntimeDispose);
    OnPreDispose();

    CHECK(!isolate_->IsInUse() && "V8 Isolate is still being used when disposing");

    QLOG(LOG_DEBUG, "{} imported modules (URL):", runtime_id_);
    for (auto& module : module_cache_)
    {
        QLOG(LOG_DEBUG, "  %fg<cyan,hl>{}%reset", module.first->toString());
        module.second.reset();
    }

    // The destructors of language binding classes will be called during `Cleanup`,
    // which means the JavaScript code may be executed by those destructors,
    // so we create temporary scopes to execute JavaScript code or allow the destructors
    // to manipulate JavaScript objects.
    {
        v8::Isolate::Scope isolate_scope(isolate_);
        v8::HandleScope handle_scope(isolate_);
        v8::Context::Scope context_scope(GetContext());
        binder::Cleanup(isolate_);
    }

    context_.Reset();
    platform_->UnregisterIsolate(isolate_);
    isolate_->Dispose();

    OnPostDispose();
    disposed_ = true;

    external_callbacks_.CallGroup(ExternalCallbackType::kAfterRuntimeDispose);
}

bindings::BindingBase *RuntimeBase::GetSyntheticModuleBinding(v8::Local<v8::Module> module)
{
    for (auto& pair : module_cache_)
    {
        if (pair.second.module == module)
            return pair.second.binding;
    }
    return nullptr;
}

namespace {

v8::MaybeLocal<v8::Value>
synthetic_module_evaluation_steps(v8::Local<v8::Context> context,
                                  v8::Local<v8::Module> module)
{
    v8::Isolate *isolate = context->GetIsolate();
    v8::EscapableHandleScope scope(isolate);
    auto *runtime_base = RuntimeBase::FromIsolate(isolate);

    bindings::BindingBase *binding = runtime_base->GetSyntheticModuleBinding(module);
    CHECK(binding);

    binder::Module bound_module(isolate);
    binding->onGetModule(bound_module);

    v8::Local<v8::Object> exports = bound_module.new_instance();
    exports->Set(context,
                 binder::to_v8(isolate, "__name__"),
                 binder::to_v8(isolate, binding->name())).Check();

    exports->Set(context,
                 binder::to_v8(isolate, "__desc__"),
                 binder::to_v8(isolate, binding->description())).Check();

    exports->Set(context,
                 binder::to_v8(isolate, "__unique_id__"),
                 binder::to_v8(isolate, binding->onGetUniqueId())).Check();

    // Synthetic modules set their own specified dynamic properties here
    binding->onSetInstanceProperties(exports);

    // Store the `exports` object into the module cache entry
    RuntimeBase *runtime = RuntimeBase::FromIsolate(isolate);
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
            QLOG(LOG_ERROR, "Synthetic module {} has a non-string-named export", binding->name());
            return {};
        }

        v8::Local<v8::String> name = v8::Local<v8::String>::Cast(export_name_value);
        v8::Local<v8::Value> value = CHECKED(exports->Get(context, name));
        module->SetSyntheticModuleExport(isolate, name, value).Check();
    }

    return scope.EscapeMaybe(v8::MaybeLocal<v8::Value>(v8::True(isolate)));
}

v8::MaybeLocal<v8::Module>
create_synthetic_module(v8::Isolate *isolate,
                        bindings::BindingBase *binding)
{
    v8::EscapableHandleScope scope(isolate);

    // Classes registering is performed here to make sure all the exported
    // classes are available after `RuntimeBase::GetAndCacheSyntheticModule()`
    // is called.
    binding->onRegisterClasses(isolate);

    std::vector<v8::Local<v8::String>> exports{
        binder::to_v8(isolate, "__name__"),
        binder::to_v8(isolate, "__desc__"),
        binder::to_v8(isolate, "__unique_id__")
    };
    for (const char **p = binding->onGetExports(); *p; p++)
        exports.emplace_back(CHECKED(v8::String::NewFromUtf8(isolate, *p)));

    v8::Local<v8::Module> module = v8::Module::CreateSyntheticModule(
        isolate,
        binder::to_v8(isolate, binding->name().c_str()),
        exports,
        synthetic_module_evaluation_steps
    );

    if (module.IsEmpty())
    {
        QLOG(LOG_ERROR, "Failed to create synthetic module `{}`", binding->name());
        return {};
    }
    return scope.Escape(module);
}

} // namespace anonymous

v8::MaybeLocal<v8::Module>
RuntimeBase::GetAndCacheSyntheticModule(const ModuleImportURL::SharedPtr& url)
{
    v8::EscapableHandleScope scope(GetIsolate());
    if (url->getProtocol() != ModuleImportURL::Protocol::kSynthetic)
        return {};
    for (auto& cache : module_cache_)
    {
        if (*cache.first == *url)
            return scope.Escape(cache.second.module.Get(GetIsolate()));
    }
    bindings::BindingBase *binding = url->getSyntheticBinding();

    auto maybe = create_synthetic_module(GetIsolate(), binding);
    if (maybe.IsEmpty())
        return {};
    v8::Local<v8::Module> module = maybe.ToLocalChecked();

    module_cache_[url] = ESModuleCache(GetIsolate(), module, binding);
    return scope.Escape(module);
}

v8::MaybeLocal<v8::Module>
RuntimeBase::CompileModule(const ModuleImportURL::SharedPtr& referer,
                           const std::string& url,
                           int script_source_flags)
{
    v8::Isolate::Scope isolateScope(GetIsolate());
    v8::EscapableHandleScope handleScope(GetIsolate());
    v8::Context::Scope contextScope(this->GetContext());

    ModuleImportURL::ResolvedAs resolve_strategy;
    if (script_source_flags & kFromImport_ScriptSourceFlag)
    {
        resolve_strategy = script_source_flags & kSysInvoke_ScriptSourceFlag
                         ? ModuleImportURL::ResolvedAs::kSysImport
                         : ModuleImportURL::ResolvedAs::kUserImport;
    }
    else
    {
        resolve_strategy = script_source_flags & kSysInvoke_ScriptSourceFlag
                         ? ModuleImportURL::ResolvedAs::kSysExecute
                         : ModuleImportURL::ResolvedAs::kUserExecute;
    }

    ModuleImportURL::SharedPtr resolved = ModuleImportURL::Resolve(referer, url, resolve_strategy);
    if (!resolved)
    {
        QLOG(LOG_ERROR, "({}) Failed to resolve module path `{}`", runtime_id_, url);
        return {};
    }

    for (auto& cache : module_cache_)
    {
        if (*cache.first == *resolved)
            return handleScope.Escape(cache.second.module.Get(GetIsolate()));
    }

    // Synthetic modules don't need to be compiled
    if (resolved->getProtocol() == ModuleImportURL::Protocol::kSynthetic)
    {
        v8::Local<v8::Module> module = GetAndCacheSyntheticModule(resolved).ToLocalChecked();
        return handleScope.Escape(module);
    }

    v8::ScriptOrigin script_origin(GetIsolate(),
                                   binder::to_v8(GetIsolate(), resolved->toString()),
                                   0,
                                   0,
                                   false,
                                   -1,
                                   v8::Local<v8::Value>(),
                                   false,
                                   false,
                                   true);

    v8::ScriptCompiler::Source source(binder::to_v8(GetIsolate(), *resolved->loadResourceText()),
                                      script_origin);
    v8::Local<v8::Module> module;
    v8::TryCatch tryCatch(GetIsolate());
    if (!v8::ScriptCompiler::CompileModule(GetIsolate(), &source).ToLocal(&module))
    {
        if (tryCatch.HasCaught())
        {
            auto message = binder::from_v8<std::string>(GetIsolate(), tryCatch.Message()->Get());
            QLOG(LOG_ERROR, "({}) Failed to compile JavaScript module `{}`: {}",
                 runtime_id_, resolved->toString(), message);
        }
        else
        {
            QLOG(LOG_ERROR, "({}) Failed to compile JavaScript module `{}`",
                 runtime_id_, resolved->toString());
        }
        return {};
    }

    module_cache_[resolved] = ESModuleCache(GetIsolate(), module);
    return handleScope.Escape(module);
}

namespace {

v8::MaybeLocal<v8::Module>
instantiate_module_callback(v8::Local<v8::Context> context,
                            v8::Local<v8::String> specifier,
                            g_maybe_unused v8::Local<v8::FixedArray> assertions,
                            v8::Local<v8::Module> referer)
{
    v8::Isolate *isolate = context->GetIsolate();
    auto *runtime_base = RuntimeBase::FromIsolate(isolate);

    for (auto& cache : runtime_base->GetModuleCache())
    {
        if (cache.second.module != referer)
            continue;
        auto url = binder::from_v8<std::string>(isolate, specifier);

        // FIXME(sora): propagate `SysInvoke` script source
        v8::MaybeLocal<v8::Module> maybe_module = runtime_base->CompileModule(
                cache.first,
                url,
                RuntimeBase::kFromImport_ScriptSourceFlag
        );
        QLOG(LOG_DEBUG, "({}) Resolved ES module {} (from {})",
             runtime_base->GetRuntimeId(), url, cache.first->toString());

        return maybe_module;
    }
    return {};
}

} // namespace anonymous

v8::MaybeLocal<v8::Value>
RuntimeBase::EvaluateModule(const std::string& url,
                            v8::Local<v8::Module> *out,
                            const std::shared_ptr<ModuleImportURL>& referer,
                            int script_source_flags)
{
    v8::Local<v8::Module> module = this->CompileModule(referer, url, script_source_flags)
                                   .FromMaybe(v8::Local<v8::Module>());
    if (module.IsEmpty())
        return {};

    if (out)
        *out = module;

    v8::Local<v8::Context> context = GetContext();

    if (module->GetStatus() == v8::Module::Status::kInstantiated
        || module->GetStatus() == v8::Module::Status::kEvaluated
        || module->GetStatus() == v8::Module::Status::kErrored)
    {
        return module->Evaluate(context);
    }

    v8::TryCatch caught(GetIsolate());
    v8::Maybe<bool> instantiate = module->InstantiateModule(context,
                                                            instantiate_module_callback);

    if (instantiate.IsNothing())
    {
        if (caught.HasCaught())
        {
            auto what = binder::from_v8<std::string>(
                    GetIsolate(), caught.Exception()->ToString(GetContext()).ToLocalChecked());
            QLOG(LOG_ERROR, "%fg<re>Evaluation: {}%reset", what);
        }
        throw RuntimeException(__func__, "Could not instantiate ES6 module " + url);
    }

    v8::MaybeLocal<v8::Value> result = module->Evaluate(context);
    PerformIdleEventCheckpoint();

    return result;
}

v8::MaybeLocal<v8::Value>
RuntimeBase::ExecuteScript(const char *script_name, const char *source_str)
{
    v8::Isolate::Scope isolate_scope(GetIsolate());
    v8::EscapableHandleScope handle_scope(GetIsolate());
    v8::Context::Scope ctx_scope(GetContext());

    v8::ScriptOrigin origin(GetIsolate(),
                            binder::to_v8(isolate_, script_name),
                            0,
                            0,
                            false,
                            -1,
                            v8::Local<v8::Value>(),
                            false,
                            false,
                            false);

    v8::ScriptCompiler::Source source(binder::to_v8(isolate_, source_str), origin);
    v8::Local<v8::Script> script;

    if (!v8::ScriptCompiler::Compile(GetContext(), &source).ToLocal(&script))
        return {};

    v8::MaybeLocal<v8::Value> result = script->Run(GetContext());
    PerformIdleEventCheckpoint();

    return handle_scope.EscapeMaybe(result);
}

void RuntimeBase::promise_hook(v8::PromiseHookType type,
                               v8::Local<v8::Promise> promise,
                               g_maybe_unused v8::Local<v8::Value> parent)
{
    if (type == v8::PromiseHookType::kResolve)
    {
        auto *runtime_base = RuntimeBase::FromIsolate(promise->GetIsolate());
        runtime_base->nb_pending_resolved_promises_++;
    }
}

void RuntimeBase::PerformTasksCheckpoint()
{
    TRACE_EVENT("main", "RuntimeBase::PerformTasksCheckpoint");

    /**
     * We must set `resolved_promises_` counter to zero before `PerformMicrotaskCheckpoint`
     * instead of setting it after that function, because `PerformMicrotaskCheckpoint`
     * may resolve some pending promises.
     */
    nb_pending_resolved_promises_ = 0;
    isolate_->PerformMicrotaskCheckpoint();

    OnPostPerformTasksCheckpoint();
    external_callbacks_.CallGroup(ExternalCallbackType::kAfterTasksCheckpoint);
}

void RuntimeBase::PerformIdleEventCheckpoint()
{
    if (nb_pending_resolved_promises_ > 0)
        event_idle_.Start([]{});
    else
        event_idle_.Stop();
}

void RuntimeBase::ReportUncaughtExceptionInCallback(const v8::TryCatch& catch_block)
{
    OnReportUncaughtExceptionInCallback(catch_block);
}

void RuntimeBase::RegisterExternalValueHolder(BinderExtValueHolderBase *value)
{
    CHECK(value);

    auto itr = std::find(binder_external_value_holders_.begin(),
                         binder_external_value_holders_.end(),
                         value);
    if (itr != binder_external_value_holders_.end())
        return;

    binder_external_value_holders_.emplace_back(value);
}

void RuntimeBase::UnregisterExternalValueHolder(BinderExtValueHolderBase *value)
{
    CHECK(value);

    auto itr = std::find(binder_external_value_holders_.begin(),
                         binder_external_value_holders_.end(),
                         value);
    CHECK(itr != binder_external_value_holders_.end());
    binder_external_value_holders_.erase(itr);
}

void RuntimeBase::DeleteExternalValueHolders()
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

void RuntimeBase::SpinRun()
{
    v8::Isolate::Scope isolate_scope(isolate_);
    v8::HandleScope handle_scope(isolate_);
    v8::Context::Scope context_scope(GetContext());

    do {
        uv_run(event_loop_, UV_RUN_DEFAULT);
        platform_->DrainTasks(isolate_);
        PerformTasksCheckpoint();
    } while (uv_loop_alive(event_loop_));

    external_callbacks_.CallGroup(ExternalCallbackType::kBeforeSpinRunExit);
}

uint64_t RuntimeBase::AddExternalCallback(ExternalCallbackType type,
                                          std::function<ExternalCallbackAfterCall(void)> func)
{
    return external_callbacks_.Add(type, std::move(func));
}

void RuntimeBase::RemoveExternalCallback(ExternalCallbackType type, uint64_t id)
{
    external_callbacks_.Remove(type, id);
}

GALLIUM_NS_END
