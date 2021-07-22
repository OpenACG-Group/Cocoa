#include <vector>
#include <iostream>

#include "Core/Journal.h"
#include "Scripter/ScripterBase.h"
#include "Scripter/Ops.h"
#include "Scripter/Runtime.h"

#include "Scripter/GprTimer.h"
#include "Scripter/GprObjectWrapper.h"

#include "Vanilla/Context.h"
SCRIPTER_NS_BEGIN

namespace {

bool operator<(const OpEntry& p1, const OpEntry& p2)
{
    return p1.nameHash < p2.nameHash;
}
bool operator>(const OpEntry& p1, const OpEntry& p2)
{
    return p1.nameHash > p2.nameHash;
}
OpEntry *pEntryRoot;

void disposeEntryTree(OpEntry *pNode)
{
    if (pNode->pLeft)
        disposeEntryTree(pNode->pLeft);
    if (pNode->pRight)
        disposeEntryTree(pNode->pRight);
    delete pNode;
}

} // namespace anonymous

void OpsTableHeapDispose()
{
    if (pEntryRoot)
        disposeEntryTree(pEntryRoot);
}

const OpEntry *OpsTableFind(const char *name)
{
    size_t hash = std::hash<std::string>()(std::string(name));
    auto *current = pEntryRoot;
    while (current)
    {
        if (hash > current->nameHash)
            current = current->pLeft;
        else if (hash < current->nameHash)
            current = current->pRight;
        else
        {
            current->callCount++;
            break;
        }
    }
    return current;
}

void OpsTableInsertEntry(const char *name, OpEntry::ExecutionType type, OpHandler pfn)
{
    auto *ptr = new OpEntry;
    if (pEntryRoot == nullptr)
        pEntryRoot = ptr;
    ptr->name = name;
    ptr->nameHash = std::hash<std::string>()(ptr->name);
    ptr->executionType = type;
    ptr->pfn = pfn;
    ptr->callCount = 0;
    ptr->pLeft = ptr->pRight = nullptr;

    if (pEntryRoot == ptr)
        return;

    auto **current = &pEntryRoot;
    while (*current)
    {
        if (*ptr > **current)
            current = &(*current)->pLeft;
        else if (*ptr < **current)
            current = &(*current)->pRight;
        else
        {
            assert(false || "Hash conflicted");
        }
    }
    *current = ptr;
}

/* [[Op Implementations]] */

namespace
{
v8::Local<v8::String> operator""_js(const char *str, size_t size)
{
    return v8::String::NewFromUtf8(v8::Isolate::GetCurrent(), str,
                                   v8::NewStringType::kNormal,
                                   static_cast<int>(size)).ToLocalChecked();
}

RID extract_rid_from_args(OpParameterInfo& info)
{
    v8::Local<v8::Context> context = info.context();
    v8::Local<v8::Object> args = info.get();
    if (args->Has(context, "rid"_js).IsNothing())
        return -OP_EINVARG;

    v8::Local<v8::Value> rid = info["rid"];
    if (!rid->IsInt32())
        return -OP_ETYPE;
    return rid->ToInt32(context).ToLocalChecked()->Value();
}

} // namespace anonymous

OpHandlerImpl(op_print)
{
    auto value = v8::String::Utf8Value(param.isolate(), param["str"]);
    std::printf("%s\n", *value);
    return OP_SUCCESS;
}

OpHandlerImpl(op_dispose)
{
    RID rid = extract_rid_from_args(param);
    param.runtime()->resourcePool().release(rid);
    return OP_SUCCESS;
}

OpHandlerImpl(op_timer_create)
{
    v8::Local<v8::Context> context = param.runtime()->context();
    if (param.get()->Has(context, "callback"_js).IsNothing())
        return -OP_EINVARG;

    auto callback = v8::Local<v8::Function>::Cast(param["callback"]);
    return param.runtime()->resourcePool().resourceGen<GprTimer>(param.runtime(), callback)->getRID();
}

OpHandlerImpl(op_timer_ctl)
{
    v8::Local<v8::Context> context = param.runtime()->context();
    v8::Local<v8::Object> args = param.get();

    RID rid = extract_rid_from_args(param);
    if (rid < 0)
        return rid;
    ResourceDescriptorPool::ScopedAcquire<GprTimer> timer(param.runtime()->resourcePool(), rid);
    if (!timer.valid())
        return -OP_EBADRID;

    if (args->Has(context, "verb"_js).IsNothing())
        return -OP_EINVARG;

    int32_t verb = param["verb"]->ToInt32(context).ToLocalChecked()->Value();
    if (verb >= static_cast<int8_t>(OpTimerCtlVerb::kMaxEnum))
        return -OP_EINVARG;

    int64_t interval;
    switch (static_cast<OpTimerCtlVerb>(verb))
    {
    case OpTimerCtlVerb::kStart:
    case OpTimerCtlVerb::kSetInterval:
        if (args->Has(context, "interval"_js).IsNothing())
            return -OP_EINVARG;
        interval = param["interval"]->ToInteger(context).ToLocalChecked()->Value();
        timer->startTimer(interval, interval);
        break;

    case OpTimerCtlVerb::kStop:
        timer->stopTimer();
        break;

    case OpTimerCtlVerb::kMaxEnum:
        return -OP_EINVARG;
    }
    return OP_SUCCESS;
}

OpHandlerImpl(op_va_ctx_create)
{
    using namespace vanilla;

    v8::Local<v8::Context> context = param.runtime()->context();
    v8::Isolate *isolate = param.isolate();
    v8::Local<v8::Object> args = param.get();

    Context::Backend backendEnum;
    {
        if (args->Has(context, "backend"_js).IsNothing())
            return -OP_EINVARG;
        std::string backend(*v8::String::Utf8Value(isolate, param["backend"]));
        if (backend == "X11" || backend == "XCB")
            backendEnum = Context::Backend::kXcb;
        else
            return -OP_EINVARG;
    }

    auto vanillaContext = Context::Make(param.runtime()->eventLoop(), backendEnum);
    if (vanillaContext == nullptr)
        return -OP_EINTERNAL;

    return param.runtime()->resourcePool().resourceGen<GprObjectWrapper>(param.runtime(),
                                                                         std::move(vanillaContext),
                                                                         [](GprObjectWrapper *pThis) -> void {
        auto ctx = pThis->get<Context>();
        if (!ctx->allDisplaysAreUnique())
            throw VanillaException("op_va_ctx_create::lambda",
                                   "Displays are owned by other objects while disposing");
    })->getRID();
}

OpHandlerImpl(op_va_ctx_connect)
{
    v8::Local<v8::Context> context = param.context();
    v8::Local<v8::Object> args = param.get();

    RID rid = extract_rid_from_args(param);
    if (rid < 0)
        return rid;
    ResourceDescriptorPool::ScopedAcquire<GprObjectWrapper> scope(param.runtime()->resourcePool(), rid);
    if (!scope.valid())
        return -OP_EBADRID;
    auto ctx = scope->get<vanilla::Context>();

    const char *displayName = nullptr;
    if (!args->Has(context, "displayName"_js).IsNothing()
        && !param["displayName"]->IsNull())
    {
        displayName = *v8::String::Utf8Value(param.isolate(), param["displayName"]);
    }

    int32_t displayId;
    if (args->Has(context, "displayId"_js).IsNothing())
        return -OP_EINVARG;
    displayId = param["displayId"]->ToInt32(context).ToLocalChecked()->Value();

    if (ctx->hasDisplay(displayId))
        return -OP_EBUSY;

    ctx->connectTo(displayName, displayId);
    return OP_SUCCESS;
}

void OpsTableHeapInitialize()
{
    pEntryRoot = nullptr;

    OpsTableInsertEntry(OP_PRINT, OpEntry::ExecutionType::kSynchronous, op_print);
    OpsTableInsertEntry(OP_DISPOSE, OpEntry::ExecutionType::kSynchronous, op_dispose);
    OpsTableInsertEntry(OP_TIMER_CREATE, OpEntry::ExecutionType::kSynchronous, op_timer_create);
    OpsTableInsertEntry(OP_TIMER_CTL, OpEntry::ExecutionType::kSynchronous, op_timer_ctl);

    OpsTableInsertEntry(OP_VA_CTX_CREATE, OpEntry::ExecutionType::kSynchronous, op_va_ctx_create);
    OpsTableInsertEntry(OP_VA_CTX_CONNECT, OpEntry::ExecutionType::kSynchronous, op_va_ctx_connect);
}

SCRIPTER_NS_END
