#include <thread>
#include <tuple>
#include <iostream>
#include <memory>

#include "include/libplatform/libplatform.h"
#include "include/v8.h"

#include "Core/Utils.h"
#include "Core/Exception.h"
#include "Core/Journal.h"
#include "Core/EventLoop.h"
#include "Koi/KoiBase.h"
#include "Koi/Runtime.h"
#include "Koi/ModuleUrl.h"
#include "Koi/BindingManager.h"

#include "Koi/binder/Module.h"
#include "Koi/binder/Class.h"
#include "Koi/lang/Base.h"
KOI_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Koi)

#define CASTJS(v) binder::to_v8(isolate, v)

Runtime::Options::Options()
    : v8_platform_thread_pool(0)
{
}

namespace {

v8::MaybeLocal<v8::Value> create_synthetic_module_eval_step(v8::Local<v8::Context> context,
                                                            v8::Local<v8::Module> module)
{
    v8::Isolate *isolate = context->GetIsolate();
    v8::EscapableHandleScope scope(isolate);
    auto *pRT = reinterpret_cast<Runtime*>(isolate->GetData(ISOLATE_DATA_SLOT_RUNTIME_PTR));

    lang::BindingBase *binding = pRT->getSyntheticModuleBinding(module);
    assert(binding);

    binder::Module binderModule(isolate);
    binding->getModule(binderModule);
    v8::Local<v8::Object> exportObject = binderModule.new_instance();
    exportObject->Set(context, CASTJS("__name__"), CASTJS(binding->name())).Check();
    exportObject->Set(context, CASTJS("__desc__"), CASTJS(binding->description())).Check();
    exportObject->Set(context, CASTJS("__unique_id__"), CASTJS(binding->getUniqueId())).Check();
    binding->setInstanceProperties(exportObject);

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

v8::Local<v8::Module> create_synthetic_module(v8::Isolate *isolate,
                                              v8::Local<v8::Context> context,
                                              lang::BindingBase *binding)
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
                                                                     create_synthetic_module_eval_step);
    if (module.IsEmpty())
    {
        throw RuntimeException(__func__,
                               fmt::format("Failed to create synthetic module \"{}\"", binding->name()));
    }
    return scope.Escape(module);
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

    std::unique_ptr<v8::Platform> platform =
            v8::platform::NewDefaultPlatform(static_cast<int>(options.v8_platform_thread_pool),
                                             v8::platform::IdleTaskSupport::kEnabled);

    v8::V8::InitializePlatform(platform.get());
    v8::V8::Initialize();

    v8::Isolate::CreateParams params;
    // params.array_buffer_allocator = new ArrayBufferAllocator();
    params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();

    auto *isolate = v8::Isolate::New(params);
    isolate->SetMicrotasksPolicy(v8::MicrotasksPolicy::kExplicit);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope scope(isolate);

    v8::Local<v8::Context> context = v8::Context::New(isolate);
    v8::Context::Scope contextScope(context);

    // load_bindings(isolate, context, options);
    return std::make_shared<Runtime>(loop,
                                     std::move(platform),
                                     params.array_buffer_allocator,
                                     isolate,
                                     v8::Global<v8::Context>(isolate, context));
}

Runtime::Runtime(EventLoop *loop,
                 std::unique_ptr<v8::Platform> platform,
                 v8::ArrayBuffer::Allocator *allocator,
                 v8::Isolate *isolate,
                 v8::Global<v8::Context> context)
    : LoopPrologueSource(loop),
      fPlatform(std::move(platform)),
      fArrayBufferAllocator(allocator),
      fIsolate(isolate),
      fContext(std::move(context))
{
    v8::Isolate::Scope isolateScope(fIsolate);
    v8::HandleScope handleScope(fIsolate);
    v8::Context::Scope ctxScope(this->context());

    LoopPrologueSource::startLoopPrologue();
    fIsolate->SetData(ISOLATE_DATA_SLOT_RUNTIME_PTR, this);
}

Runtime::~Runtime()
{
    LOGW(LOG_DEBUG, "Imported modules (URL):")
    for (auto& module : fModuleCache)
    {
        LOGF(LOG_DEBUG, "  %fg<cyan,hl>{}%reset", module.first)
        module.second.module.Reset();
    }

    binder::Cleanup(fIsolate);
    fContext.Reset();
    fIsolate->Dispose();
    v8::V8::Dispose();
    v8::V8::ShutdownPlatform();
    delete fArrayBufferAllocator;
    fPlatform.reset();
}

lang::BindingBase *Runtime::getSyntheticModuleBinding(v8::Local<v8::Module> module)
{
    for (auto& pair : fModuleCache)
    {
        if (pair.second.module == module)
            return pair.second.binding;
    }
    return nullptr;
}

Runtime::ModuleCache& Runtime::getModuleCache()
{
    return fModuleCache;
}

v8::Local<v8::Module> Runtime::compileModule(const std::string& url)
{
    return Runtime::compileModule(".", url);
}

v8::Local<v8::Module> Runtime::compileModule(const std::string& refererUrl, const std::string& url)
{
    v8::Isolate::Scope isolateScope(fIsolate);
    v8::EscapableHandleScope handleScope(fIsolate);
    v8::Context::Scope contextScope(this->context());

    std::string maybeSyntheticUrl = "synthetic://" + url;
    if (fModuleCache.contains(maybeSyntheticUrl))
        return handleScope.Escape(fModuleCache[maybeSyntheticUrl].module.Get(fIsolate));

    /* Synthetic module don't need to be compiled, so we have a fastpath */
    lang::BindingBase *pSyntheticBinding = BindingManager::Ref().search(url);
    if (pSyntheticBinding)
    {
        v8::Local<v8::Context> context = this->context();
        v8::Local<v8::Module> module = create_synthetic_module(fIsolate, context, pSyntheticBinding);
        fModuleCache[maybeSyntheticUrl] = {
                v8::Global<v8::Module>(fIsolate, module),
                pSyntheticBinding
        };
        return handleScope.Escape(module);
    }

    auto [moduleSourceMaybe, moduleUrl] = ResolveModuleImportUrl(fIsolate, refererUrl, url);
    if (moduleSourceMaybe.IsEmpty())
        throw RuntimeException(__func__, "Failed to resolve ES6 module path \"" + url + "\"");

    auto moduleSource = moduleSourceMaybe.ToLocalChecked();

    if (fModuleCache.contains(moduleUrl))
        return handleScope.Escape(fModuleCache[moduleUrl].module.Get(fIsolate));

    v8::ScriptOrigin scriptOrigin(v8::String::NewFromUtf8(fIsolate, moduleUrl.c_str()).ToLocalChecked(),
                                  0,
                                  0,
                                  false,
                                  -1,
                                  v8::Local<v8::Value>(),
                                  false,
                                  false,
                                  true);

    v8::ScriptCompiler::Source source(moduleSource, scriptOrigin);
    v8::Local<v8::Module> module;
    v8::TryCatch tryCatch(fIsolate);
    if (!v8::ScriptCompiler::CompileModule(fIsolate, &source).ToLocal(&module))
    {
        if (tryCatch.HasCaught())
        {
            std::string message(*v8::String::Utf8Value(fIsolate, tryCatch.Message()->Get()));
            LOGF(LOG_ERROR, "Failed to compile ES6 module {}: {}", moduleUrl, message)
        }
        throw RuntimeException(__func__, "Failed to compile ES6 module \"" + moduleUrl + "\"");
    }

    fModuleCache[moduleUrl] = {
            v8::Global<v8::Module>(fIsolate, module),
            nullptr
    };
    return handleScope.Escape(module);
}

namespace {

v8::MaybeLocal<v8::Module> instantiate_module_callback(v8::Local<v8::Context> context,
                                                       v8::Local<v8::String> specifier,
                                                       koi_maybe_unsed v8::Local<v8::FixedArray> assertions,
                                                       v8::Local<v8::Module> referer)
{
    auto *pRT = reinterpret_cast<Runtime*>(context->GetIsolate()->GetData(ISOLATE_DATA_SLOT_RUNTIME_PTR));
    std::string refererUrl;
    for (auto& cache : pRT->getModuleCache())
    {
        if (cache.second.module == referer)
        {
            refererUrl = cache.first;
            break;
        }
    }
    assert(!refererUrl.empty());
    std::string url(*v8::String::Utf8Value(pRT->isolate(), specifier));
    v8::MaybeLocal<v8::Module> maybeModule = pRT->compileModule(refererUrl, url);
    LOGF(LOG_DEBUG, "Resolved ES module {} (from {})", url, refererUrl)
    return maybeModule;
}
} // namespace anonymous

v8::MaybeLocal<v8::Value> Runtime::evaluateModule(const std::string& url)
{
    v8::Local<v8::Module> module = this->compileModule(url);
    if (module->GetStatus() == v8::Module::Status::kInstantiated
        || module->GetStatus() == v8::Module::Status::kEvaluated)
    {
        return module->Evaluate(this->context());
    }
    v8::Maybe<bool> instantiate = module->InstantiateModule(this->context(), instantiate_module_callback);

    if (instantiate.IsNothing())
        throw RuntimeException(__func__, "Could not instantiate ES6 module " + url);
    return module->Evaluate(this->context());
}

v8::Local<v8::Value> Runtime::execute(const char *str)
{
    return this->execute("<anonymous>", str);
}

v8::Local<v8::Value> Runtime::execute(const char *scriptName, const char *str)
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

    v8::Local<v8::Value> result;
    if (!script->Run(this->context()).ToLocal(&result))
        return {};
    return handleScope.Escape(result);
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

    int handlesCount = 0;
    EventSource::eventLoop()->walk([&handlesCount](uv_handle_t *handle) -> void {
        if (uv_is_active(handle))
            handlesCount++;
    });

    if (handlesCount <= 1)
        stopLoopPrologue();
    return KeepInLoop::kYes;
}

void Runtime::takeOverNativeException(const RuntimeException& exception)
{
    utils::DumpRuntimeException(exception);
    auto str = fmt::format("Native[method:{}]: {}", exception.who(), exception.what());
    auto message = v8::String::NewFromUtf8(fIsolate, str.c_str()).ToLocalChecked();
    fIsolate->ThrowException(v8::Exception::Error(message));
}

KOI_NS_END
