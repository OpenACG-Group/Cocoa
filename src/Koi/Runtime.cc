#include <thread>
#include <tuple>
#include <iostream>

#include "include/libplatform/libplatform.h"
#include "include/v8.h"

#include "Core/Utils.h"
#include "Core/Exception.h"
#include "Core/Journal.h"
#include "Core/EventLoop.h"
#include "Koi/KoiBase.h"
#include "Koi/Runtime.h"
#include "Koi/Ops.h"
#include "Koi/ModuleUrl.h"
KOI_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Koi)

void vmInvokeOpSynchronously(v8::Isolate *isolate, v8::Local<v8::Object> param, const OpEntry *opEntry,
                             v8::ReturnValue<v8::Value>& returnValue)
{
    OpParameterInfo info(isolate, param, OpParameterInfo::StorageType::kLocal);
    returnValue.Set(opEntry->pfn(info));
}

void vmInvokeOpAsynchronously(v8::Isolate *isolate, v8::Local<v8::Object> param, const OpEntry *opEntry,
                              v8::ReturnValue<v8::Value>& returnValue)
{
    v8::Local<v8::Promise::Resolver> promiseResolver =
            v8::Promise::Resolver::New(isolate->GetCurrentContext()).ToLocalChecked();
    OpParameterInfo info(isolate,
                         param,
                         promiseResolver,
                         OpParameterInfo::StorageType::kLocal);

    opEntry->pfn(info);
    returnValue.Set(promiseResolver->GetPromise());
}

void vmInvokeOp(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    v8::Isolate *isolate = info.GetIsolate();
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::ReturnValue<v8::Value> retValue = info.GetReturnValue();
    if (info.Length() != 2)
    {
        retValue.Set(-OP_EARGC);
        return;
    }

    if (!info[0]->IsString() || !info[1]->IsObject())
    {
        retValue.Set(-OP_ETYPE);
        return;
    }
    v8::Local<v8::String> opName = info[0]->ToString(context).ToLocalChecked();
    v8::Local<v8::Object> parameterObject = info[1]->ToObject(context).ToLocalChecked();

    const OpEntry *opEntry = OpsTableFind(*v8::String::Utf8Value(isolate, opName));
    if (opEntry == nullptr)
    {
        retValue.Set(-OP_EOPNUM);
        return;
    }
    if (opEntry->pfn == nullptr)
    {
        retValue.Set(-OP_EINTERNAL);
        return;
    }

    const_cast<OpEntry*>(opEntry)->callCount++;
    try {
        switch (opEntry->executionType)
        {
        case OpEntry::ExecutionType::kSynchronous:
            vmInvokeOpSynchronously(isolate, parameterObject, opEntry, retValue);
            break;
        case OpEntry::ExecutionType::kAsynchronous:
            vmInvokeOpAsynchronously(isolate, parameterObject, opEntry, retValue);
            break;
        }
    } catch (const RuntimeException& e) {
        auto rt = reinterpret_cast<Runtime*>(isolate->GetData(ISOLATE_DATA_SLOT_RUNTIME_PTR));
        rt->takeOverNativeException(e);
    }
}

void Initialize()
{
    OpsTableHeapInitialize();
}

void Dispose()
{
    OpsTableHeapDispose();
}

Runtime::Options::Options()
    : v8_platform_thread_pool(0)
{
}

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
            v8::platform::NewDefaultPlatform(options.v8_platform_thread_pool,
                                             v8::platform::IdleTaskSupport::kEnabled,
                                             v8::platform::InProcessStackDumping::kEnabled);

    v8::V8::InitializePlatform(platform.get());
    v8::V8::Initialize();

    v8::Isolate::CreateParams params;
    // params.array_buffer_allocator = new ArrayBufferAllocator();
    params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();

    auto *isolate = v8::Isolate::New(params);
    isolate->SetMicrotasksPolicy(v8::MicrotasksPolicy::kExplicit);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope scope(isolate);

    auto objectTemplate = v8::ObjectTemplate::New(isolate);
    objectTemplate->Set(isolate, "__cocoa_op_call", v8::FunctionTemplate::New(isolate, vmInvokeOp));

    v8::Global<v8::Context> context(isolate, v8::Context::New(isolate, nullptr, objectTemplate));
    return std::make_shared<Runtime>(loop,
                                     std::move(platform),
                                     params.array_buffer_allocator,
                                     isolate,
                                     std::move(context));
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
      fContext(std::move(context)),
      fResourcePool(new ResourceDescriptorPool())
{
    v8::Isolate::Scope isolateScope(fIsolate);
    v8::HandleScope handleScope(fIsolate);
    v8::Context::Scope ctxScope(this->context());

    LoopPrologueSource::startLoopPrologue();
    fIsolate->SetData(ISOLATE_DATA_SLOT_RUNTIME_PTR, this);
}

Runtime::~Runtime()
{
    delete fResourcePool;

    for (auto& module : fModuleCache)
        module.second.Reset();

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
        return v8::Local<v8::Value>();

    v8::Local<v8::Value> result;
    if (!script->Run(this->context()).ToLocal(&result))
        return v8::Local<v8::Value>();
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
