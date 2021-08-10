#include <vector>
#include <iostream>

#include "Core/Journal.h"
#include "Koi/KoiBase.h"
#include "Koi/Ops.h"
#include "Koi/Runtime.h"
KOI_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Koi)

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

v8::Local<v8::String> js_literal::operator""_js(const char *str, size_t size)
{
    return v8::String::NewFromUtf8(v8::Isolate::GetCurrent(), str,
                                   v8::NewStringType::kNormal,
                                   static_cast<int>(size)).ToLocalChecked();
}

OpRet OpExtractRIDFromArgs(OpParameterInfo& info)
{
    using namespace js_literal;

    v8::Local<v8::Context> context = info.context();
    v8::Local<v8::Object> args = info.get();
    if (args->Has(context, "rid"_js).IsNothing())
        return -OP_EINVARG;

    v8::Local<v8::Value> rid = info["rid"];
    if (!rid->IsInt32())
    {
        std::string realType = *v8::String::Utf8Value(info.isolate(), rid->TypeOf(info.isolate()));
        LOGF(LOG_DEBUG, "OpCall: Bad argument type for \"rid\" field: {}", realType)
        return -OP_ETYPE;
    }

    return rid->ToInt32(context).ToLocalChecked()->Value();
}

#define INSERT OpsTableInsertEntry
#define SYNC OpEntry::ExecutionType::kSynchronous
#define ASYNC OpEntry::ExecutionType::kAsynchronous
void OpsTableHeapInitialize()
{
    pEntryRoot = nullptr;

    INSERT(OP_PRINT, SYNC, op_print);
    INSERT(OP_DISPOSE, SYNC, op_dispose);
    INSERT(OP_TIMER_CREATE, SYNC, op_timer_create);
    INSERT(OP_TIMER_CTL, SYNC, op_timer_ctl);
    INSERT(OP_VA_CTX_CREATE, SYNC, op_va_ctx_create);
    INSERT(OP_VA_CTX_CONNECT, SYNC, op_va_ctx_connect);
    INSERT(OP_VA_DIS_CLOSE, SYNC, op_va_dis_close);
    INSERT(OP_VA_DIS_GEOMETRY, SYNC, op_va_dis_geometry);
}
#undef INSERT
#undef SYNC
#undef ASYNC

KOI_NS_END
