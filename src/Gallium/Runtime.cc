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

v8::MaybeLocal<v8::Value> createSyntheticModuleEvalStep(v8::Local<v8::Context> context,
                                                        v8::Local<v8::Module> module)
{
    v8::Isolate *isolate = context->GetIsolate();
    v8::EscapableHandleScope scope(isolate);
    auto *pRT = reinterpret_cast<Runtime*>(isolate->GetData(ISOLATE_DATA_SLOT_RUNTIME_PTR));

    bindings::BindingBase *binding = pRT->getSyntheticModuleBinding(module);
    CHECK(binding);

    binding->onRegisterClasses(isolate);

    binder::Module binderModule(isolate);
    binding->onGetModule(binderModule);

    v8::Local<v8::Object> exportObject = binderModule.new_instance();
    exportObject->Set(context, CASTJS("__name__"), CASTJS(binding->name())).Check();
    exportObject->Set(context, CASTJS("__desc__"), CASTJS(binding->description())).Check();
    exportObject->Set(context, CASTJS("__unique_id__"), CASTJS(binding->onGetUniqueId())).Check();
    binding->onSetInstanceProperties(exportObject);

    Runtime *runtime = Runtime::GetBareFromIsolate(isolate);
    auto& moduleCacheMap = runtime->getModuleCache();
    bool exportObjectStored = false;
    for (auto& cache : moduleCacheMap)
    {
        if (cache.second.module == module)
        {
            cache.second.setExportsObject(isolate, exportObject);
            exportObjectStored = true;
        }
    }
    CHECK(exportObjectStored);

    v8::Local<v8::Array> properties = exportObject->GetPropertyNames(context).ToLocalChecked();
    for (uint32_t i = 0; i < properties->Length(); i++)
    {
        v8::Local<v8::String> name = v8::Local<v8::String>::Cast(properties->Get(context, i).ToLocalChecked());
        if (name.IsEmpty())
            continue;
        v8::Local<v8::Value> value = exportObject->Get(context, name).ToLocalChecked();
        module->SetSyntheticModuleExport(isolate, name, value).Check();
    }
    return scope.EscapeMaybe(v8::MaybeLocal<v8::Value>(v8::True(isolate)));
}

v8::Local<v8::Module> createSyntheticModule(v8::Isolate *isolate,
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
                                                                     createSyntheticModuleEvalStep);
    if (module.IsEmpty())
    {
        throw RuntimeException(__func__,
                               fmt::format("Failed to create synthetic module \"{}\"", binding->name()));
    }
    return scope.Escape(module);
}

v8::StartupData *load_startup_snapshot()
{
    auto snapshotData = QResource::Instance()->Lookup("org.cocoa.internal.v8", "/snapshot_blob.bin");
    if (!snapshotData)
    {
        QLOG(LOG_ERROR, "Failed to load snapshot data from package org.cocoa.internal.v8");
        return nullptr;
    }

    size_t snapshotDataSize = snapshotData->size();
    auto *startupData = new v8::StartupData{
            .data = new char[snapshotDataSize],
            .raw_size = static_cast<int>(snapshotDataSize)
    };

    snapshotData->read(const_cast<char*>(startupData->data), snapshotDataSize);
    v8::V8::SetSnapshotDataBlob(startupData);

    return startupData;
}

} // namespace anonymous

void Runtime::AdoptV8CommandOptions(const Options& options)
{
    for (const auto& arg : options.v8_options)
        v8::V8::SetFlagsFromString(arg.c_str(), arg.length());
}

std::shared_ptr<Runtime> Runtime::Make(EventLoop *loop, const Options& options)
{
    v8::StartupData *startupData = load_startup_snapshot();
    if (!startupData)
        return nullptr;

    ScopeExitAutoInvoker startupDataReleaser([startupData] {
        delete[] startupData->data;
        delete startupData;
    });

    std::unique_ptr<v8::Platform> platform =
            v8::platform::NewDefaultPlatform(static_cast<int>(options.v8_platform_thread_pool),
                                             v8::platform::IdleTaskSupport::kEnabled);

    v8::V8::InitializePlatform(platform.get());
    v8::V8::Initialize();

    v8::Isolate::CreateParams params;
    // TODO: Implement a more efficient allocator
    params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();

    auto *isolate = v8::Isolate::New(params);
    isolate->SetMicrotasksPolicy(v8::MicrotasksPolicy::kExplicit);
    std::shared_ptr<Runtime> runtime;

    {
        v8::Isolate::Scope isolateScope(isolate);
        v8::HandleScope scope(isolate);

        v8::Local<v8::Context> context = v8::Context::New(isolate);
        v8::Context::Scope contextScope(context);

        runtime = std::make_shared<Runtime>(loop,
                                            nullptr,
                                            std::move(platform),
                                            params.array_buffer_allocator,
                                            isolate,
                                            v8::Global<v8::Context>(isolate, context),
                                            options);
        startupDataReleaser.cancel();

        auto isolateGuard = std::make_unique<GlobalIsolateGuard>(runtime);
        runtime->setGlobalIsolateGuard(std::move(isolateGuard));

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
                 std::unique_ptr<v8::Platform> platform,
                 v8::ArrayBuffer::Allocator *allocator,
                 v8::Isolate *isolate,
                 v8::Global<v8::Context> context,
                 Options opts)
    : PrepareSource(loop)
    , CheckSource(loop)
    , fOptions(std::move(opts))
    , fStartupData(startupData)
    , fPlatform(std::move(platform))
    , fArrayBufferAllocator(allocator)
    , fIsolate(isolate)
    , fContext(std::move(context))
    , fResolvedPromises(0)
    , fIdle{}
{
    v8::Isolate::Scope isolateScope(fIsolate);
    v8::HandleScope handleScope(fIsolate);
    v8::Context::Scope ctxScope(this->context());

    fIsolate->SetData(ISOLATE_DATA_SLOT_RUNTIME_PTR, this);
    fIsolate->SetPromiseHook(Runtime::PromiseHookCallback);
    fIsolate->SetHostImportModuleDynamicallyCallback(Runtime::DynamicImportHostCallback);

    PrepareSource::startPrepare();
    PrepareSource::unrefEventSource();
    CheckSource::startCheck();
    CheckSource::unrefEventSource();

    infra::InstallOnGlobalContext(fIsolate, this->context());
    if (fOptions.rt_expose_introspect)
        fIntrospect = VMIntrospect::InstallGlobal(fIsolate);

    uv_idle_init(loop->handle(), &fIdle);

    evaluateModule("internal:///gallium/bootstrap.js", nullptr, nullptr, false, true);
}

Runtime::~Runtime()
{
    uv_close((uv_handle_t *)&fIdle, nullptr);

    QLOG(LOG_DEBUG, "Imported modules (URL):");
    for (auto& module : fModuleCache)
    {
        QLOG(LOG_DEBUG, "  %fg<cyan,hl>{}%reset", module.first->toString());
        module.second.reset();
    }

    ModuleImportURL::FreeInternalCaches();
    binder::Cleanup(fIsolate);
    fIntrospect.reset();
    fContext.Reset();
    fIsolateGuard.reset();
    fIsolate->Dispose();
    v8::V8::Dispose();
    v8::V8::ShutdownPlatform();
    delete fArrayBufferAllocator;
    fPlatform.reset();

    if (fStartupData)
    {
        delete[] fStartupData->data;
        delete fStartupData;
    }
}

bindings::BindingBase *Runtime::getSyntheticModuleBinding(v8::Local<v8::Module> module)
{
    for (auto& pair : fModuleCache)
    {
        if (pair.second.module == module)
            return pair.second.binding;
    }
    return nullptr;
}

Runtime::ModuleCacheMap& Runtime::getModuleCache()
{
    return fModuleCache;
}

v8::MaybeLocal<v8::Module> Runtime::getAndCacheSyntheticModule(const ModuleImportURL::SharedPtr& url)
{
    v8::EscapableHandleScope scope(fIsolate);
    if (url->getProtocol() != ModuleImportURL::Protocol::kSynthetic)
        return {};
    for (auto& cache : fModuleCache)
    {
        if (*cache.first == *url)
            return scope.Escape(cache.second.module.Get(fIsolate));
    }
    bindings::BindingBase *binding = url->getSyntheticBinding();
    v8::Local<v8::Module> module = createSyntheticModule(fIsolate, binding);
    fModuleCache[url] = ESModuleCache(fIsolate, module, binding);
    return scope.Escape(module);
}

v8::Local<v8::Object> Runtime::getSyntheticModuleExportObject(v8::Local<v8::Module> module)
{
    for (auto& cache : fModuleCache)
    {
        if (cache.second.module == module)
            return cache.second.exports.IsEmpty()
                    ? v8::Local<v8::Object>()
                    : cache.second.exports.Get(fIsolate);
    }
    return {};
}

v8::Local<v8::Module> Runtime::compileModule(const ModuleImportURL::SharedPtr& referer,
                                             const std::string& url,
                                             bool isImport,
                                             bool isSysInvoke)
{
    v8::Isolate::Scope isolateScope(fIsolate);
    v8::EscapableHandleScope handleScope(fIsolate);
    v8::Context::Scope contextScope(this->context());

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
    for (auto& cache : fModuleCache)
    {
        if (*cache.first == *resolved)
            return handleScope.Escape(cache.second.module.Get(fIsolate));
    }

    // Synthetic modules don't need to be compiled
    if (resolved->getProtocol() == ModuleImportURL::Protocol::kSynthetic)
    {
        v8::Local<v8::Module> module = getAndCacheSyntheticModule(resolved).ToLocalChecked();
        return handleScope.Escape(module);
    }

    v8::ScriptOrigin scriptOrigin(binder::to_v8(fIsolate, resolved->toString()),
                                  0,
                                  0,
                                  false,
                                  -1,
                                  v8::Local<v8::Value>(),
                                  false,
                                  false,
                                  true);

    v8::ScriptCompiler::Source source(binder::to_v8(fIsolate, *resolved->loadResourceText()),
                                      scriptOrigin);
    v8::Local<v8::Module> module;
    v8::TryCatch tryCatch(fIsolate);
    if (!v8::ScriptCompiler::CompileModule(fIsolate, &source).ToLocal(&module))
    {
        if (tryCatch.HasCaught())
        {
            auto message = binder::from_v8<std::string>(fIsolate, tryCatch.Message()->Get());
            QLOG(LOG_ERROR, "Failed to compile ES6 module {}: {}", resolved->toString(), message);
        }
        throw RuntimeException(__func__, "Failed to compile ES6 module \"" + resolved->toString() + "\"");
    }

    fModuleCache[resolved] = ESModuleCache(fIsolate, module);
    return handleScope.Escape(module);
}

namespace {

v8::MaybeLocal<v8::Module> instantiateModuleCallback(v8::Local<v8::Context> context,
                                                     v8::Local<v8::String> specifier,
                                                     g_maybe_unused v8::Local<v8::FixedArray> assertions,
                                                     v8::Local<v8::Module> referer)
{
    auto *pRT = reinterpret_cast<Runtime*>(context->GetIsolate()->GetData(ISOLATE_DATA_SLOT_RUNTIME_PTR));
    for (auto& cache : pRT->getModuleCache())
    {
        if (cache.second.module != referer)
            continue;
        auto url = binder::from_v8<std::string>(pRT->isolate(), specifier);

        // TODO: propagate `isSysInvoke`
        v8::MaybeLocal<v8::Module> maybeModule = pRT->compileModule(cache.first, url, true, false);
        QLOG(LOG_DEBUG, "Resolved ES module {} (from {})", url, cache.first->toString());
        return maybeModule;
    }
    return {};
}
} // namespace anonymous

v8::MaybeLocal<v8::Value> Runtime::evaluateModule(const std::string& url,
                                                  v8::Local<v8::Module> *outModule,
                                                  const std::shared_ptr<ModuleImportURL> &referer,
                                                  bool isImport, bool isSysInvoke)
{
    v8::Local<v8::Module> module = this->compileModule(referer, url, isImport, isSysInvoke);
    if (outModule)
        *outModule = module;
    if (module->GetStatus() == v8::Module::Status::kInstantiated
        || module->GetStatus() == v8::Module::Status::kEvaluated
        || module->GetStatus() == v8::Module::Status::kErrored)
    {
        return module->Evaluate(this->context());
    }

    v8::TryCatch caught(fIsolate);
    v8::Maybe<bool> instantiate = module->InstantiateModule(this->context(), instantiateModuleCallback);

    if (instantiate.IsNothing())
    {
        if (caught.HasCaught())
        {
            auto what = binder::from_v8<std::string>(
                    fIsolate, caught.Exception()->ToString(context()).ToLocalChecked());
            QLOG(LOG_ERROR, "%fg<re>Evaluation: {}%reset", what);
        }
        throw RuntimeException(__func__, "Could not instantiate ES6 module " + url);
    }

    v8::MaybeLocal<v8::Value> result = module->Evaluate(this->context());
    updateIdleRequirementByPromiseCounter();

    return result;
}

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

    auto referrerUrl = binder::from_v8<std::string>(isolate, resource_name);
    Runtime *runtime = Runtime::GetBareFromIsolate(isolate);

    auto& moduleCache = runtime->getModuleCache();
    auto itr = std::find_if(moduleCache.begin(), moduleCache.end(),
                            [&referrerUrl](ModuleCacheMap::value_type& pair) {
        return (pair.first->toString() == referrerUrl);
    });

    if (itr == moduleCache.end())
    {
        resolver->Reject(context,
                         binder::to_v8(isolate, "Dynamic import: Referrer is not cached"))
                         .Check();
        return scope.EscapeMaybe(promise);
    }

    auto specifierUrl = binder::from_v8<std::string>(isolate, specifier);
    v8::Local<v8::Module> module;
    try
    {
        v8::Local<v8::Value> value;
        if (!runtime->evaluateModule(specifierUrl, &module, itr->first, true).ToLocal(&value))
            throw RuntimeException(__func__, fmt::format("Error evaluating module {}", specifierUrl));
    }
    catch (const std::exception& e)
    {
        resolver->Reject(context,
                         binder::to_v8(isolate, fmt::format("Dynamic import: {}", e.what())))
                         .Check();
        return scope.EscapeMaybe(promise);
    }

    QLOG(LOG_DEBUG, "Resolved ES module {} (from {}, dynamically)", specifierUrl, itr->first->toString());
    resolver->Resolve(context, module->GetModuleNamespace()).Check();
    return scope.EscapeMaybe(promise);
}

v8::MaybeLocal<v8::Value> Runtime::execute(const char *scriptName, const char *str)
{
    v8::Isolate::Scope isolateScope(fIsolate);
    v8::EscapableHandleScope handleScope(fIsolate);
    v8::Context::Scope contextScope(this->context());

    v8::Local<v8::String> sourceCode = v8::String::NewFromUtf8(fIsolate, str).ToLocalChecked();
    v8::Local<v8::String> scriptNameS = v8::String::NewFromUtf8(fIsolate, scriptName).ToLocalChecked();
    v8::ScriptOrigin origin(scriptNameS,
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

    if (!v8::ScriptCompiler::Compile(this->context(), &source).ToLocal(&script))
        return {};

    v8::MaybeLocal<v8::Value> result = script->Run(this->context());
    updateIdleRequirementByPromiseCounter();

    return handleScope.EscapeMaybe(result);
}

KeepInLoop Runtime::prepareDispatch()
{
    updateIdleRequirementByPromiseCounter();
    return KeepInLoop::kYes;
}

KeepInLoop Runtime::checkDispatch()
{
    performTasksCheckpoint();
    updateIdleRequirementByPromiseCounter();
    return KeepInLoop::kYes;
}

void Runtime::performTasksCheckpoint()
{
    while (v8::platform::PumpMessageLoop(fPlatform.get(), fIsolate));

    /**
     * We must set `fResolvedPromises` counter to zero before `PerformMicrotaskCheckpoint`
     * instead of setting it after that function, because `PerformMicrotaskCheckpoint`
     * may resolve some pending promises.
     */
    fResolvedPromises = 0;
    fIsolate->PerformMicrotaskCheckpoint();
    fIsolateGuard->performUnhandledRejectPromiseCheck();

    // TODO(Important): Where should it be placed?
    if (fIntrospect)
        fIntrospect->performScheduledTasksCheckpoint();
}

void Runtime::PromiseHookCallback(v8::PromiseHookType type, v8::Local<v8::Promise> promise,
                                  [[maybe_unused]] v8::Local<v8::Value> parent)
{
    Runtime *runtime = Runtime::GetBareFromIsolate(promise->GetIsolate());
    if (type == v8::PromiseHookType::kResolve)
        runtime->fResolvedPromises++;
}

void Runtime::updateIdleRequirementByPromiseCounter()
{
    if (fResolvedPromises > 0)
        uv_idle_start(&fIdle, [](uv_idle_t *) {});
    else
        uv_idle_stop(&fIdle);
}

bool Runtime::isInstanceOfGlobalClass(v8::Local<v8::Value> value, const std::string& class_)
{
    if (!value->IsObject())
        g_throw(TypeError, "value is not an object");

    v8::HandleScope scope(fIsolate);
    v8::Local<v8::Context> ctx = context();

    v8::Local<v8::Value> constructor;
    if (!ctx->Global()->Get(ctx, binder::to_v8(fIsolate, class_)).ToLocal(&constructor))
    {
        g_throw(ReferenceError, "No global constructor function named " + class_);
    }
    if (!constructor->IsFunction())
        g_throw(TypeError, "Constructor is not a function");

    v8::Local<v8::Value> method;
    if (!constructor.As<v8::Function>()->Get(ctx, binder::to_v8(fIsolate, "__has_instance__")).ToLocal(&method))
    {
        binder::JSException::Throw(binder::ExceptT::kReferenceError,
                                   "Constructor has no property named __has_instance__");
    }
    if (!method->IsFunction())
    {
        g_throw(TypeError, "Property __has_instance__ is not a function");
    }

    v8::Local<v8::Value> ret = binder::Invoke(fIsolate, method.As<v8::Function>(), ctx->Global(), value);
    if (!ret->IsBoolean())
    {
        g_throw(TypeError, "Method __has_instance__ does not return a boolean value");
    }
    return ret.As<v8::Boolean>()->Value();
}

void Runtime::reportUncaughtExceptionInCallback(const v8::TryCatch& catchBlock)
{
    fIsolateGuard->reportUncaughtExceptionFromCallback(catchBlock);
}

void Runtime::notifyRuntimeWillExit()
{
    if (fIntrospect)
        fIntrospect->notifyBeforeExit();
}

GALLIUM_NS_END
