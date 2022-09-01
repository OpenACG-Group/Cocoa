/**
 * This file is part of Cocoa.
 *
 * Cocoa is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * Cocoa is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cocoa. If not, see <https://www.gnu.org/licenses/>.
 */

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
