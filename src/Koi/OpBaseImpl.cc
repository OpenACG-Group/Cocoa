#include "Koi/Ops.h"
#include "Koi/Runtime.h"
KOI_NS_BEGIN

OpHandlerImpl(op_print)
{
    auto value = v8::String::Utf8Value(param.isolate(), param["str"]);
    std::printf("%s\n", *value);
    return OP_SUCCESS;
}

OpHandlerImpl(op_dispose)
{
    RID rid = OpExtractRIDFromArgs(param);
    if (rid < 0)
        return rid;
    param.runtime()->resourcePool().release(rid);
    return OP_SUCCESS;
}

KOI_NS_END
