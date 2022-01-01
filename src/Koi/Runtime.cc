#include <tuple>
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
#include "Koi/KoiBase.h"
#include "Koi/Runtime.h"
#include "Koi/BindingManager.h"
#include "Koi/ModuleImportURL.h"

#include "Koi/binder/Module.h"
#include "Koi/binder/Class.h"
#include "Koi/binder/CallV8.h"
#include "Koi/bindings/Base.h"
KOI_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Koi)

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

    binder::Module binderModule(isolate);
    binding->getModule(binderModule);
    v8::Local<v8::Object> exportObject = binderModule.new_instance();
    exportObject->Set(context, CASTJS("__name__"), CASTJS(binding->name())).Check();
    exportObject->Set(context, CASTJS("__desc__"), CASTJS(binding->description())).Check();
    exportObject->Set(context, CASTJS("__unique_id__"), CASTJS(binding->getUniqueId())).Check();
    binding->setInstanceProperties(exportObject);

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
    for (const char **p = binding->getExports(); *p; p++)
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

void executeInternalScript(const std::shared_ptr<Runtime>& rt, const std::string& url)
{
    v8::Isolate *isolate = rt->isolate();
    v8::HandleScope handleScope(isolate);

    auto resolvedUrl = ModuleImportURL::Resolve(nullptr,
                                                url,
                                                ModuleImportURL::ResolvedAs::kSysExecute);
    if (!resolvedUrl)
    {
        throw RuntimeException::Builder(__func__)
            .append("Failed in decompressing internal script ")
            .append(resolvedUrl->toString())
            .make<RuntimeException>();
    }

    auto maybeSource = resolvedUrl->loadResourceText();
    if (!maybeSource)
    {
        throw RuntimeException::Builder(__func__)
                .append("Failed in decompressing internal script ")
                .append(resolvedUrl->toString())
                .make<RuntimeException>();
    }

    auto result = rt->execute(resolvedUrl->toString().c_str(),
                              maybeSource->c_str());
    if (result.IsEmpty())
    {
        throw RuntimeException::Builder(__func__)
                .append("Failed in executing internal script ")
                .append(resolvedUrl->toString())
                .make<RuntimeException>();
    }
}

} // namespace anonymous

std::shared_ptr<Runtime> Runtime::MakeFromSnapshot(EventLoop *loop,
                                                   const std::string& snapshotFile,
                                                   const std::string& icuDataFile,
                                                   const Options& options)
{
    if (!snapshotFile.empty())
        v8::V8::InitializeExternalStartupDataFromFile(snapshotFile.c_str());
    if (!icuDataFile.empty())
        v8::V8::InitializeICU(icuDataFile.c_str());
    else
        v8::V8::InitializeICU();

    for (const auto& arg : options.v8_options)
        v8::V8::SetFlagsFromString(arg.c_str(), arg.length());

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
                                            std::move(platform),
                                            params.array_buffer_allocator,
                                            isolate,
                                            v8::Global<v8::Context>(isolate, context),
                                            options);
        auto isolateGuard = std::make_unique<GlobalIsolateGuard>(runtime);
        runtime->setGlobalIsolateGuard(std::move(isolateGuard));

        executeInternalScript(runtime, "internal://context/refvalue");
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
                 std::unique_ptr<v8::Platform> platform,
                 v8::ArrayBuffer::Allocator *allocator,
                 v8::Isolate *isolate,
                 v8::Global<v8::Context> context,
                 Options opts)
    : LoopPrologueSource(loop)
    , AsyncSource(loop)
    , fOptions(std::move(opts))
    , fPlatform(std::move(platform))
    , fArrayBufferAllocator(allocator)
    , fIsolate(isolate)
    , fContext(std::move(context))
{
    v8::Isolate::Scope isolateScope(fIsolate);
    v8::HandleScope handleScope(fIsolate);
    v8::Context::Scope ctxScope(this->context());

    LoopPrologueSource::startLoopPrologue();
    fIsolate->SetData(ISOLATE_DATA_SLOT_RUNTIME_PTR, this);
    if (fOptions.rt_expose_introspect)
        fIntrospect = VMIntrospect::InstallGlobal(fIsolate);

    v8::Local<v8::Context> ctx = this->context();
    ctx->Global()->Set(ctx, binder::to_v8(fIsolate, "__global__"), ctx->Global()).Check();
}

Runtime::~Runtime()
{
    if (fIntrospect)
        fIntrospect->notifyBeforeExit();

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
                                             bool isImport)
{
    v8::Isolate::Scope isolateScope(fIsolate);
    v8::EscapableHandleScope handleScope(fIsolate);
    v8::Context::Scope contextScope(this->context());

    ModuleImportURL::ResolvedAs resolvedAs = isImport ? ModuleImportURL::ResolvedAs::kUserImport
                                             : ModuleImportURL::ResolvedAs::kUserExecute;
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
                                                     koi_maybe_unsed v8::Local<v8::FixedArray> assertions,
                                                     v8::Local<v8::Module> referer)
{
    auto *pRT = reinterpret_cast<Runtime*>(context->GetIsolate()->GetData(ISOLATE_DATA_SLOT_RUNTIME_PTR));
    for (auto& cache : pRT->getModuleCache())
    {
        if (cache.second.module != referer)
            continue;
        auto url = binder::from_v8<std::string>(pRT->isolate(), specifier);
        v8::MaybeLocal<v8::Module> maybeModule = pRT->compileModule(cache.first, url, true);
        QLOG(LOG_DEBUG, "Resolved ES module {} (from {})", url, cache.first->toString());
        return maybeModule;
    }
    return {};
}
} // namespace anonymous

v8::MaybeLocal<v8::Value> Runtime::evaluateModule(const std::string& url)
{
    v8::Local<v8::Module> module = this->compileModule(nullptr, url, false);
    if (module->GetStatus() == v8::Module::Status::kInstantiated
        || module->GetStatus() == v8::Module::Status::kEvaluated)
    {
        return module->Evaluate(this->context());
    }
    v8::Maybe<bool> instantiate = module->InstantiateModule(this->context(), instantiateModuleCallback);

    if (instantiate.IsNothing())
        throw RuntimeException(__func__, "Could not instantiate ES6 module " + url);
    return module->Evaluate(this->context());
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
    return handleScope.EscapeMaybe(script->Run(this->context()));
}

KeepInLoop Runtime::loopPrologueDispatch()
{
    while (v8::platform::PumpMessageLoop(fPlatform.get(),
                                         fIsolate,
                                         v8::platform::MessageLoopBehavior::kDoNotWait))
    {
        // Do nothing, just waiting
    }
    fIsolate->PerformMicrotaskCheckpoint();
    fIntrospect->performScheduledTasksCheckpoint();

    int handlesCount = 0;
    LoopPrologueSource::eventLoop()->walk([&handlesCount](uv_handle_t *handle) -> void {
        if (uv_is_active(handle))
            handlesCount++;
    });

    if (handlesCount <= 2)
    {
        LoopPrologueSource::stopLoopPrologue();
        AsyncSource::close();
    }
    return KeepInLoop::kYes;
}

void Runtime::asyncDispatch()
{
    // TODO: Implement inspector
}

bool Runtime::isInstanceOfGlobalClass(v8::Local<v8::Value> value, const std::string& class_)
{
    if (!value->IsObject())
        binder::JSException::Throw(binder::ExceptT::kTypeError, "value is not an object");

    v8::HandleScope scope(fIsolate);
    v8::Local<v8::Context> ctx = context();

    v8::Local<v8::Value> constructor;
    if (!ctx->Global()->Get(ctx, binder::to_v8(fIsolate, class_)).ToLocal(&constructor))
    {
        binder::JSException::Throw(binder::ExceptT::kReferenceError,
                                   "No global constructor function named " + class_);
    }
    if (!constructor->IsFunction())
        binder::JSException::Throw(binder::ExceptT::kTypeError, "Constructor is not a function");

    v8::Local<v8::Value> method;
    if (!constructor.As<v8::Function>()->Get(ctx, binder::to_v8(fIsolate, "__has_instance__")).ToLocal(&method))
    {
        binder::JSException::Throw(binder::ExceptT::kReferenceError,
                                   "Constructor has no property named __has_instance__");
    }
    if (!method->IsFunction())
    {
        binder::JSException::Throw(binder::ExceptT::kTypeError,
                                   "Property __has_instance__ is not a function");
    }

    v8::Local<v8::Value> ret = binder::Invoke(fIsolate, method.As<v8::Function>(), ctx->Global(), value);
    if (!ret->IsBoolean())
    {
        binder::JSException::Throw(binder::ExceptT::kTypeError,
                                   "Method __has_instance__ does not return a boolean value");
    }
    return ret.As<v8::Boolean>()->Value();
}

KOI_NS_END
