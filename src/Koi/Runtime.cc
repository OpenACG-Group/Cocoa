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

#include "Koi/binder/Module.h"
#include "Koi/binder/Class.h"
#include "Koi/lang/Base.h"
KOI_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Koi)

void Initialize()
{
}

void Dispose()
{
}

Runtime::Options::Options()
    : v8_platform_thread_pool(0)
{
}

namespace {

void load_bindings(v8::Isolate *isolate, v8::Local<v8::Context> context,
                   const Runtime::Options& options)
{
    v8::HandleScope scope(isolate);
    v8::Local<v8::Object> global = context->Global();
    v8::Local<v8::Object> ccObject = v8::Object::New(isolate);

    lang::ForEachBinding([isolate, &context, &ccObject, options](lang::BaseBindingModule *pBinding) -> bool {
        assert(pBinding);

        auto itr = std::find(options.bindings_blacklist.begin(), options.bindings_blacklist.end(),
                             pBinding->name());
        if (itr != options.bindings_blacklist.end())
            return true;

        binder::Module mod(isolate);
        pBinding->getModule(mod);
        ccObject->Set(context, binder::to_v8(isolate, pBinding->name()), mod.new_instance()).Check();

        LOGF(LOG_DEBUG, R"(Loaded language binding "{}" ({}))",
             pBinding->name(), pBinding->description())
        return true;
    });

    global->Set(context, binder::to_v8(isolate, "Cocoa"), ccObject).Check();
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

    load_bindings(isolate, context, options);
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
    for (auto& module : fModuleCache)
        module.second.Reset();

    binder::Cleanup(fIsolate);
    fContext.Reset();
    fIsolate->Dispose();
    v8::V8::Dispose();
    v8::V8::ShutdownPlatform();
    delete fArrayBufferAllocator;
    fPlatform.reset();
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

    auto [moduleSourceMaybe, moduleUrl] = ResolveModuleImportUrl(fIsolate, refererUrl, url);
    if (moduleSourceMaybe.IsEmpty())
        throw RuntimeException(__func__, "Failed to resolve ES6 module path " + url);

    auto moduleSource = moduleSourceMaybe.ToLocalChecked();

    if (fModuleCache.contains(moduleUrl))
        return handleScope.Escape(fModuleCache[moduleUrl].Get(fIsolate));

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
        throw RuntimeException(__func__, "Failed to compile ES6 module " + moduleUrl);
    }

    fModuleCache[moduleUrl] = v8::Global<v8::Module>(fIsolate, module);
    return handleScope.Escape(module);
}

v8::MaybeLocal<v8::Value> Runtime::evaluateModule(const std::string& url)
{
    v8::Local<v8::Module> module = this->compileModule(url);
    if (module->GetStatus() == v8::Module::Status::kInstantiated
        || module->GetStatus() == v8::Module::Status::kEvaluated)
    {
        return module->Evaluate(this->context());
    }
    v8::Maybe<bool> instantiate =
            module->InstantiateModule(this->context(), [](v8::Local<v8::Context> context,
                                                          v8::Local<v8::String> specifier,
                                                          v8::Local<v8::FixedArray> assertions,
                                                          v8::Local<v8::Module> referer) {
                auto *pRT = reinterpret_cast<Runtime*>(context->GetIsolate()->GetData(ISOLATE_DATA_SLOT_RUNTIME_PTR));
                std::string refererUrl;
                for (auto& cache : pRT->fModuleCache)
                {
                    if (cache.second == referer)
                    {
                        refererUrl = cache.first;
                        break;
                    }
                }
                if (refererUrl.empty())
                    return v8::MaybeLocal<v8::Module>();

                std::string url(*v8::String::Utf8Value(pRT->fIsolate, specifier));
                v8::MaybeLocal<v8::Module> maybeModule;
                try {
                    maybeModule = pRT->compileModule(refererUrl, url);
                } catch (const RuntimeException& e) {
                    LOGF(LOG_DEBUG, "Failed in resolving module {}", url)
                }

                LOGF(LOG_DEBUG, "Resolved ES module {} (from {})", url, refererUrl)
                return maybeModule;
            });

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
