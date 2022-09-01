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

#include "fmt/format.h"

#include "Reactor/GShaderExternals.h"
#include "Core/Errors.h"

#include <x86intrin.h>

REACTOR_NAMESPACE_BEGIN

#undef V
#define V(x)    reinterpret_cast<void*>((x))

#define GSHADER_API         [[gnu::visibility("default")]]

GSHADER_API int32_t builtin_check_host_context(GShaderModule::HostContext *ptr)
{
    if (ptr == nullptr)
        return external::START_USER_RET_FAILED;
    if (ptr->magic_number != external::HOST_CTX_MAGIC_NUMBER)
        return external::START_USER_RET_FAILED;
    return external::START_USER_RET_NORMAL;
}

GSHADER_API void builtin_v8_trampoline(GShaderModule::HostContext *ptr, uint32_t method_id)
{
    if (ptr->v8_method_map.count(method_id) == 0)
        return;

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    v8::Local<v8::Function> func = ptr->v8_method_map[method_id].Get(isolate);
    func->Call(isolate->GetCurrentContext(),
               isolate->GetCurrentContext()->Global(), 0, nullptr).ToLocalChecked();
}

typedef float __attribute__((vector_size(8))) __float2;     // NOLINT

GSHADER_API __float2 builtin_sinf2(__float2 v)
{
    return __float2{sinf(v[0]), sinf(v[1])};
}

GSHADER_API __float2 builtin_cosf2(__float2 v)
{
    return __float2{cosf(v[0]), cosf(v[1])};
}

GSHADER_API __float2 builtin_tanf2(__float2 v)
{
    return __float2{tanf(v[0]), tanf(v[1])};
}

GSHADER_API __float2 builtin_cossinf2r(__float2 v)
{
    return __float2{cosf(v[0]), sinf(v[1])};
}

GSHADER_API __float2 builtin_sincosf2r(__float2 v)
{
    return __float2{sinf(v[0]), cosf(v[1])};
}

namespace {

using namespace llvm;

FunctionType *float_float_func_type(llvm::LLVMContext& ctx)
{
    return FunctionType::get(Type::getFloatTy(ctx), {Type::getFloatTy(ctx)}, false);
}

FunctionType *float2_float2_func_type(llvm::LLVMContext& ctx)
{
    return FunctionType::get(FixedVectorType::get(Type::getFloatTy(ctx), 2),
                             {FixedVectorType::get(Type::getFloatTy(ctx), 2)}, false);
}

struct SymbolTable
{
    using TypeGenerator = llvm::FunctionType*(*)(llvm::LLVMContext&);
    const char *name;
    int32_t id;
    void *pfn;
    TypeGenerator type_generator;
// NOLINTNEXTLINE
} g_symbol_table[] = {
    { "sinf", external::kSinF, V(sinf), float_float_func_type },
    { "cosf", external::kCosF, V(cosf), float_float_func_type },
    { "tanf", external::kTanF, V(tanf), float_float_func_type },
    { "sinf2", external::kSinF2, V(builtin_sinf2), float2_float2_func_type },
    { "cosf2", external::kCosF2, V(builtin_cosf2), float2_float2_func_type },
    { "tanf2", external::kTanF2, V(builtin_tanf2), float2_float2_func_type },
    { "sincosf2r", external::kSinCosF2R, V(builtin_sincosf2r), float2_float2_func_type },
    { "cossinf2r", external::kCosSinF2R, V(builtin_cossinf2r), float2_float2_func_type },
    {
        "__builtin_v8_trampoline",
        external::kBuiltinV8Trampoline,
        V(builtin_v8_trampoline),
        [](LLVMContext& ctx) {
            return FunctionType::get(Type::getVoidTy(ctx),
                {Type::getInt8PtrTy(ctx), Type::getInt32Ty(ctx)}, false);
        }
    },
    {
        "__builtin_check_host_context",
        external::kBuiltinCheckHostContext,
        V(builtin_check_host_context),
        [](LLVMContext& ctx) {
            return FunctionType::get(Type::getInt32Ty(ctx),
                {Type::getInt8PtrTy(ctx)}, false);
        }
    }
};

}

const llvm::StringMap<void *>& GetExternalSymbolMap()
{
    static llvm::StringMap<void *> map = [] {
        llvm::StringMap<void *> r;

        for (const SymbolTable& st : g_symbol_table)
        {
            r.try_emplace(st.name, st.pfn);
        }

        return r;
    }();

    return map;
}

llvm::FunctionType *GetExternalFunctionType(llvm::LLVMContext& context, int32_t id)
{
    for (const SymbolTable& st : g_symbol_table)
    {
        if (st.id == id)
            return st.type_generator(context);
    }
    CHECK_FAILED("Invalid external symbol");
}

const char *GetExternalFunctionName(int32_t id)
{
    for (const SymbolTable& st : g_symbol_table)
    {
        if (st.id == id)
            return st.name;
    }
    CHECK_FAILED("Invalid external symbol");
}

REACTOR_NAMESPACE_END
