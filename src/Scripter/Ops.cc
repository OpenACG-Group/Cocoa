#include <vector>
#include <iostream>

#include "Scripter/ScripterBase.h"
#include "Scripter/Ops.h"
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

OpHandlerImpl(op_print)
{
    std::cout << "op_print: " << *v8::String::Utf8Value(param.isolate(), param["str"]) << std::endl;
    return OP_SUCCESS;
}

void OpsTableHeapInitialize()
{
    pEntryRoot = nullptr;

    OpsTableInsertEntry("op_print", OpEntry::ExecutionType::kSynchronous, op_print);
}

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

SCRIPTER_NS_END
