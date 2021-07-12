#include <thread>
#include <tuple>
#include <iostream>

#include "include/libplatform/libplatform.h"
#include "include/v8.h"

#include "Scripter/ScripterBase.h"
#include "Scripter/Runtime.h"
#include "Scripter/Ops.h"
SCRIPTER_NS_BEGIN

void vmcall_return_err_promise(const v8::FunctionCallbackInfo<v8::Value>& info,
                               v8::Local<v8::Promise::Resolver> resolver, int32_t val)
{
    resolver->Reject(info.GetIsolate()->GetCurrentContext(),
                     v8::Integer::New(info.GetIsolate(), val)).ToChecked();
    info.GetReturnValue().Set(resolver->GetPromise());
}

void vmInvokeOpSynchronously(v8::Isolate *isolate, v8::Local<v8::Object> param, const OpEntry *opEntry,
                             v8::ReturnValue<v8::Value>& returnValue)
{
    OpParameterInfo info(isolate, param, OpParameterInfo::StorageType::kLocal);
    returnValue.Set(opEntry->pfn(info));
}

void vmInvokeOpAsynchronously(v8::Isolate *isolate, v8::Local<v8::Object> param, const OpEntry *opEntry,
                              v8::ReturnValue<v8::Value>& returnValue)
{
    auto *rt = reinterpret_cast<Runtime*>(isolate->GetData(ISOLATE_DATA_SLOT_RUNTIME_PTR));
    auto info = std::make_shared<OpParameterInfo>(isolate,
                                                  param,
                                                  OpParameterInfo::StorageType::kPersistent);
    v8::Local<v8::Promise::Resolver> promiseResolver =
            v8::Promise::Resolver::New(isolate->GetCurrentContext()).ToLocalChecked();

    rt->pushAsyncOpEntry(opEntry, param, promiseResolver);
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

    switch (opEntry->executionType)
    {
    case OpEntry::ExecutionType::kSynchronous:
        vmInvokeOpSynchronously(isolate, parameterObject, opEntry, retValue);
        break;
    case OpEntry::ExecutionType::kAsynchronous:
        vmInvokeOpAsynchronously(isolate, parameterObject, opEntry, retValue);
        break;
    }
}

static const char internalCoreJs[] = R"(
/**
 * CocoaJs Core Object, Javascript ES6 Standard.
 * Copyright(C) 2021, Jingheng Luo (masshiro.io@qq.com)
 */
let Cocoa = new Object();
(function (Cocoa) {
    "use strict";
    Cocoa.core = {
        OP_SUCCESS: 0,
        OP_ETYPE: 1,
        OP_EARGC: 2,
        OP_EOPNUM: 3,
        OP_EINTERNAL: 4,
        OP_EINVARG: 5,
        OP_EASYNC: 6,
        OP_ENOMEM: 7,

        OP_PRINT: "op_print"
    };

    Cocoa.core.opCall = (name, args) => {
        return __cocoa_op_call(name, args);
    };

    Cocoa.core.getScripterInfo = () => {
        return {
            version: [1, 0, 0],
            manufacture: "org.OpenACG.Cocoa",
            capabilities: ["capabilities::lang", "capabilities::opCall"]
        };
    };
})(Cocoa);
)";

void Initialize()
{
    OpsTableHeapInitialize();
}

void Dispose()
{
    OpsTableHeapDispose();
}

Runtime::Options::Options()
    : v8_platform_thread_pool(std::thread::hardware_concurrency() / 2)
{
}

std::shared_ptr<Runtime> Runtime::MakeFromSnapshot(EventLoop *loop,
                                                   const std::string& snapshotFile,
                                                   const std::string& icuDataFile,
                                                   const Options& options)
{
    v8::V8::InitializeExternalStartupDataFromFile(snapshotFile.c_str());
    v8::V8::InitializeICU(icuDataFile.c_str());

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
    : CheckHandleSource(loop),
      fPlatform(std::move(platform)),
      fArrayBufferAllocator(allocator),
      fIsolate(isolate),
      fContext(std::move(context))
{
    v8::Isolate::Scope isolateScope(fIsolate);
    v8::HandleScope handleScope(fIsolate);
    v8::Context::Scope ctxScope(this->context());

    startCheckHandle();
    fIsolate->SetData(ISOLATE_DATA_SLOT_RUNTIME_PTR, this);
    this->execute("internal:core.js", internalCoreJs);
}

Runtime::~Runtime()
{
    fContext.Reset();
    fIsolate->Dispose();
    v8::V8::Dispose();
    v8::V8::ShutdownPlatform();
    delete fArrayBufferAllocator;
    fPlatform.reset();
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

KeepInLoop Runtime::checkHandleDispatch()
{
    fIsolate->PerformMicrotaskCheckpoint();
    return KeepInLoop::kYes;
}

void Runtime::pushAsyncOpEntry(const OpEntry *entry,
                               v8::Local<v8::Object> param,
                               v8::Local<v8::Promise::Resolver> resolver)
{
    std::cout << "Push async task " << entry->name << std::endl;
    resolver->Resolve(fIsolate->GetCurrentContext(),
                      v8::Integer::New(fIsolate, OpRet(OP_SUCCESS)));
}

SCRIPTER_NS_END
