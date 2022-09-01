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

#ifndef COCOA_MOEJITSHADERCOMPILER_H
#define COCOA_MOEJITSHADERCOMPILER_H

#include <cxxabi.h>
#include <variant>

#include "asmjit/asmjit.h"
#include "asmjit/x86.h"

#include "fmt/format.h"

#include "include/core/SkCanvas.h"

#include "Core/Errors.h"
#include "Glamor/Glamor.h"
#include "Glamor/Moe/MoeJITShaderModule.h"
#include "Glamor/Moe/MoeJITShaderX86Compiler.h"
#include "Glamor/Moe/MoeJITContext.h"
GLAMOR_NAMESPACE_BEGIN

namespace {

/* To mark a function which will be called by generated code */
#define ASM_LINKAGE

template<typename T, typename...ArgsT>
ASM_LINKAGE void cxa_typed_placement_new(void *address, ArgsT...args)
{
    CHECK(address);
    new(address) T(args...);
}

template<typename T>
ASM_LINKAGE void cxa_typed_destruct_trampoline(void *address)
{
    reinterpret_cast<T*>(address)->~T();
}

template<typename T, typename RetT, typename...ArgsT>
ASM_LINKAGE RetT cxa_thiscall_trampoline(void *this_, void *member, ArgsT...args)
{
    using MemberT = RetT(T::*)(ArgsT...);
    MemberT pfn;

    *reinterpret_cast<intptr_t*>(static_cast<void*>(&pfn)) = reinterpret_cast<intptr_t>(member);
    if constexpr(std::is_void<RetT>::value)
        (reinterpret_cast<T*>(this_)->*pfn)(args...);
    else
        return (reinterpret_cast<T *>(this_)->*pfn)(args...);
}

struct BaseRegOrImm
{
    enum Type
    {
        kBaseReg,
        kImm
    };

    explicit BaseRegOrImm(asmjit::BaseReg *reg) : v_type(Type::kBaseReg), v_reg(reg) {}
    explicit BaseRegOrImm(const asmjit::Imm& imm) : v_type(Type::kImm), v_reg(nullptr), v_imm(imm) {}
    ~BaseRegOrImm() = default;

    Type v_type;
    asmjit::BaseReg *v_reg;
    asmjit::Imm v_imm;
};
using VariantArgsPack = std::vector<BaseRegOrImm>;

void unwrap_invocation_args_pack(asmjit::InvokeNode *node, int argOffset, const VariantArgsPack& args)
{
    for (size_t i = 0; i < args.size(); i++)
    {
        if (args[i].v_type == BaseRegOrImm::kBaseReg)
            node->setArg(argOffset + i, *args[i].v_reg);
        else
            node->setArg(argOffset + i, args[i].v_imm);
    }
}

template<typename T, typename...ArgsT>
asmjit::x86::Mem helper_emit_construct_on_stack(asmjit::x86::Compiler& cc, const VariantArgsPack& args)
{
    asmjit::x86::Mem ptr = cc.newStack(sizeof(T), alignof(T), typeid(T).name());
    asmjit::x86::Gp thisAddressReg = cc.newIntPtr("__this");
    cc.lea(thisAddressReg, ptr);

    asmjit::InvokeNode *invokeNode;
    cc.invoke(&invokeNode, asmjit::imm(&cxa_typed_placement_new<T, ArgsT...>),
              asmjit::FuncSignatureT<void, void*, ArgsT...>());

    invokeNode->setArg(0, thisAddressReg);
    unwrap_invocation_args_pack(invokeNode, 1, args);

    invokeNode->setInlineComment("cxa_typed_placement_new<...> [stack object constructor]");
    return ptr;
}

template<typename T>
void helper_emit_destruct_on_stack(asmjit::x86::Compiler& cc, const asmjit::x86::Mem& ptr)
{
    asmjit::x86::Gp thisAddressReg = cc.newIntPtr("__this");
    cc.lea(thisAddressReg, ptr);

    asmjit::InvokeNode *invokeNode;
    cc.invoke(&invokeNode, asmjit::imm(&cxa_typed_destruct_trampoline<T>),
              asmjit::FuncSignatureT<void, void*>());

    invokeNode->setArg(0, thisAddressReg);
    invokeNode->setInlineComment("cxa_typed_destruct_trampoline<...> [stack object destructor]");
}

template<typename T, typename RetT, typename...ArgsT>
asmjit::InvokeNode *helper_emit_thiscall(asmjit::x86::Compiler& cc,
                                         const asmjit::x86::Mem& thisPtr,
                                         RetT(T::*memberFunc)(ArgsT...),
                                         const VariantArgsPack& args)
{
    static_assert(!std::is_reference<RetT>::value, "Reference return type is not allowed");
    static_assert(!(... || std::is_reference_v<ArgsT>), "Reference type in arguments is not allowed");

    // Do a type-hack by awful pointer skills.
    // It is just like we "copy the value" from `memberFunc` to `memberFuncValue` directly.
    auto memberFuncValue = *reinterpret_cast<void**>(static_cast<void*>(&memberFunc));

    asmjit::x86::Gp thisAddressReg = cc.newIntPtr("__this");
    cc.lea(thisAddressReg, thisPtr);

    asmjit::InvokeNode *node;
    cc.invoke(&node, asmjit::imm(&cxa_thiscall_trampoline<T, RetT, ArgsT...>),
              asmjit::FuncSignatureT<RetT, void*, void*, ArgsT...>());

    node->setArg(0, thisAddressReg);
    node->setArg(1, asmjit::imm(memberFuncValue));
    unwrap_invocation_args_pack(node, 2, args);

    node->setInlineComment("cxa_thiscall_trampoline<...>");
    return node;
}

} // namespace anonymous

MoeJITShaderX86Compiler::MoeJITShaderX86Compiler()
    : code_holder_(GlobalScope::Ref().GetJITContext()->GetInitializedCodeHolder())
    , compiler_(code_holder_.get())
    , func_node_(nullptr)
{
    // compiler_.setLogger(&codegen_logger_);
    code_holder_->setLogger(&codegen_logger_);
    codegen_logger_.setFlags(asmjit::FormatFlags::kHexImms |
                             asmjit::FormatFlags::kHexOffsets);
    codegen_logger_.setIndentation(asmjit::FormatIndentationGroup::kCode, 2);
    codegen_logger_.setIndentation(asmjit::FormatIndentationGroup::kLabel, 0);

    func_node_ = compiler_.addFunc(asmjit::FuncSignatureT<void, void*>(asmjit::CallConvId::kCDecl));

    // store the first argument to register %reg
    asmjit::x86::Gp arg0Reg = compiler_.newIntPtr("__arg_canvas_ptr");
    func_node_->setArg(0, arg0Reg);
    // store the first argument to stack
    mem_arg_canvas_ptr_ = compiler_.newStack(sizeof(intptr_t), 4);
    compiler_.mov(mem_arg_canvas_ptr_, arg0Reg);
}

MoeJITShaderX86Compiler::~MoeJITShaderX86Compiler() = default;

MoeJITShaderModule::EntrypointFunc MoeJITShaderX86Compiler::Finalize()
{
    compiler_.endFunc();
    compiler_.finalize();

    auto& context = GlobalScope::Ref().GetJITContext();
    auto pFunction = context->AddFunction<MoeJITShaderModule::EntrypointFunc>(code_holder_);

    code_holder_.reset();
    return pFunction;
}

class R
{
public:
    virtual ~R() = default;
    virtual void foo(int k) {
        fmt::print("R::foo\n");
    }
};

class T : public R
{
public:
    T(int a, int b): num(1234) {
        fmt::print("construct with {}, {}, this={}\n", a, b, fmt::ptr(this));
    }

    ~T() override {
        fmt::print("destructor, this={}\n", fmt::ptr(this));
    }

    void foo(int k) override {
        fmt::print("T::foo, num={}, k={}\n", num, k);
    }

    int num;
};

void MoeJITShaderX86Compiler::InsertTestCode()
{
    using namespace asmjit;

    asmjit::x86::Mem ptr = helper_emit_construct_on_stack<T, int, int>(compiler_, {
        BaseRegOrImm(asmjit::imm(2233)),
        BaseRegOrImm(asmjit::imm(1000))
    });

    helper_emit_thiscall(compiler_, ptr, &T::foo, {
        BaseRegOrImm(asmjit::imm(9999))
    });

    helper_emit_destruct_on_stack<T>(compiler_, ptr);

    compiler_.ret();
}

GLAMOR_NAMESPACE_END
#endif //COCOA_MOEJITSHADERCOMPILER_H
