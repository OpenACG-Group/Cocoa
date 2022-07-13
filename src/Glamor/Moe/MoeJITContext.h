#ifndef COCOA_MOE_MOEJITCONTEXT_H
#define COCOA_MOE_MOEJITCONTEXT_H

#include <optional>

#include "Glamor/Glamor.h"
#include "asmjit/asmjit.h"
GLAMOR_NAMESPACE_BEGIN

class MoeJITContext
{
public:
    using CodeHolderPtr = Unique<asmjit::CodeHolder>;

    MoeJITContext() = default;
    ~MoeJITContext() = default;

    /**
     * Get an initialized CodeHolder for code generation.
     * Generated assembly code will be stored in CodeHolder.
     */
    g_nodiscard CodeHolderPtr GetInitializedCodeHolder();

    /**
     * Add the generated code as a callable function.
     * `code` is no longer needed and can be destroyed.
     */
    template<typename Func>
    g_inline Func AddFunction(const CodeHolderPtr& code) {
        Func result = nullptr;
        jit_runtime_.add(&result, code.get());
        return result;
    }

    template<typename Func>
    g_inline bool ReleaseFunction(Func pfn) {
        asmjit::Error error = jit_runtime_.release(pfn);
        return (error == asmjit::ErrorCode::kErrorOk);
    }

private:
    asmjit::JitRuntime      jit_runtime_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_MOE_MOEJITCONTEXT_H
