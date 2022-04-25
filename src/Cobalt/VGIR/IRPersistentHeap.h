#ifndef COCOA_COBALT_VGIR_IRPERSISTENTHEAP_H
#define COCOA_COBALT_VGIR_IRPERSISTENTHEAP_H

#include <map>

#include "Cobalt/Cobalt.h"
#include "Cobalt/VGIR/IREvaluationStack.h"
COBALT_NAMESPACE_BEGIN
namespace ir {

struct HeapObject
{
    enum class Type
    {
        kStackValue,
        kString
    };

    Type             type;
    union {
        StackValue      *stack_value;
        const char      *str;
    } ptr;
};

class PersistentHeap
{
public:
    PersistentHeap();
    ~PersistentHeap();

    using HeapRefId = uint32_t;

    bool StoreConstant(HeapRefId id, HeapObject *object);
    bool Store(HeapRefId id, HeapObject *object);

    HeapObject *Load(HeapRefId id);
    HeapObject *LoadConstant(HeapRefId id);

    bool Free(HeapRefId id);

private:
    std::map<HeapRefId, HeapObject*>    runtime_heap_;
    std::map<HeapRefId, HeapObject*>    constant_heap_;
};

} // namespace ir
COBALT_NAMESPACE_END
#endif //COCOA_COBALT_VGIR_IRPERSISTENTHEAP_H
