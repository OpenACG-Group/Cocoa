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

#ifndef COCOA_GLAMOR_MOE_MOEJITSHADERX86COMPILER_H
#define COCOA_GLAMOR_MOE_MOEJITSHADERX86COMPILER_H

#include <typeinfo>

#include "asmjit/asmjit.h"
#include "asmjit/x86.h"

#include "Glamor/Glamor.h"
#include "Glamor/Moe/MoeJITShaderModule.h"
GLAMOR_NAMESPACE_BEGIN

class MoeJITShaderX86Compiler
{
public:
    MoeJITShaderX86Compiler();
    ~MoeJITShaderX86Compiler();

    MoeJITShaderModule::EntrypointFunc Finalize();

    g_nodiscard g_inline asmjit::String& GetCodeGenLogging() {
        return codegen_logger_.content();
    }

    void InsertTestCode();

private:
    Unique<asmjit::CodeHolder>  code_holder_;
    asmjit::x86::Compiler       compiler_;
    asmjit::FuncNode           *func_node_;
    asmjit::x86::Mem            mem_arg_canvas_ptr_;
    asmjit::StringLogger        codegen_logger_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_MOE_MOEJITSHADERX86COMPILER_H
