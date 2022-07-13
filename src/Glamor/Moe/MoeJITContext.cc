#include "Core/Journal.h"
#include "Glamor/Moe/MoeJITContext.h"
GLAMOR_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Glamor.Moe.MoeJITContext)

MoeJITContext::CodeHolderPtr MoeJITContext::GetInitializedCodeHolder()
{
    CodeHolderPtr ptr = std::make_unique<asmjit::CodeHolder>();
    asmjit::Error error = ptr->init(jit_runtime_.environment());
    if (error != asmjit::ErrorCode::kErrorOk)
    {
        QLOG(LOG_ERROR, "Failed in initializing code holder for JIT compiler: {}",
             asmjit::DebugUtils::errorAsString(error));
        return nullptr;
    }

    return ptr;
}



GLAMOR_NAMESPACE_END
