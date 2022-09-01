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

#include "Reactor/Reactor.h"
#include "Reactor/JitSession.h"
#include "Reactor/GShaderExternals.h"
REACTOR_NAMESPACE_BEGIN

GShaderBuilder::GShaderBuilder(const std::string& name)
    : name_(name)
    , context_(std::make_unique<llvm::LLVMContext>())
    , module_(std::make_unique<llvm::Module>(name, *context_))
    , main_function_(nullptr)
    , main_basic_block_(nullptr)
{
    module_->setTargetTriple(JitSession::Ref().GetTargetTriple().str());
    module_->setDataLayout(JitSession::Ref().GetDataLayout());
    CreateEntryFunction();
}

GShaderBuilder::~GShaderBuilder()
{
}

void GShaderBuilder::CreateEntryFunction()
{
    using namespace llvm;

    auto hostCtxPtrT = PointerType::get(Type::getInt8Ty(*context_), 0);

    auto *hostContextGV = new GlobalVariable(*module_,
                                             hostCtxPtrT,
                                             false,
                                             llvm::GlobalValue::ExternalLinkage,
                                             ConstantPointerNull::get(hostCtxPtrT),
                                             "__program_host_context");

    /* int32_t __start_user_main(void *ctx) */
    FunctionType *startUserFuncT = FunctionType::get(Type::getInt32Ty(*context_),
                                                     { hostCtxPtrT }, false);
    Function *startUserFunc= Function::Create(startUserFuncT, Function::ExternalLinkage,
                                              GSHADER_ENTRY_NAME, *module_);
    exposed_functions_.push_back(startUserFunc);

    BasicBlock *block = BasicBlock::Create(*context_, "", startUserFunc);
    IRBuilder<> builder(block);

    builder.CreateStore(startUserFunc->getArg(0), hostContextGV);

    CallInst *call = CreateExternalFunctionCall(block, external::kBuiltinCheckHostContext,
                                                LoadHostContextGV(block));

    BasicBlock *retNormalBlock = BasicBlock::Create(*context_, "normal_ret", startUserFunc);
    {
        BasicBlock *retFailBlock = BasicBlock::Create(*context_, "check_failed", startUserFunc);
        Value *cond = builder.CreateICmpNE(call, NewInt(0));
        builder.CreateCondBr(cond, retFailBlock, retNormalBlock);

        builder.SetInsertPoint(retFailBlock);

        /* return START_USER_RET_FAILED */
        builder.CreateRet(NewInt(external::START_USER_RET_FAILED));
    }
    builder.SetInsertPoint(retNormalBlock);

    /* void main() */
    FunctionType *mainFuncT = FunctionType::get(Type::getVoidTy(*context_), {}, false);
    main_function_ = Function::Create(mainFuncT, Function::ExternalLinkage, "main", *module_);
    exposed_functions_.push_back(main_function_);

    main_basic_block_ = BasicBlock::Create(*context_, "__user_main_entrypoint", main_function_);

    /* main() */
    builder.CreateCall(main_function_);

    /* return START_USER_RET_NORMAL */
    builder.CreateRet(NewInt(external::START_USER_RET_NORMAL));
}

void GShaderBuilder::InsertV8FunctionSymbol(v8::Local<v8::Function> func, const std::string& name)
{
    static uint32_t idCounter = 1;
    v8_method_id_map_[name] = idCounter;
    v8_method_map_[idCounter].Reset(v8::Isolate::GetCurrent(), func);
    idCounter++;
}

void GShaderBuilder::CreateBuiltinV8FunctionCall(llvm::BasicBlock *insert, const std::string& name)
{
    using namespace llvm;

    if (v8_method_id_map_.count(name) == 0)
        return;
    uint32_t id = v8_method_id_map_[name];

    GlobalVariable *gv = module_->getGlobalVariable("__program_host_context");

    IRBuilder<> builder(insert);
    LoadInst *loadGV = builder.CreateLoad(Type::getInt8PtrTy(*context_), gv);
    Constant *methodIdConst = ConstantInt::get(Type::getInt32Ty(*context_), id);

    CreateExternalFunctionCall(insert, external::kBuiltinV8Trampoline,
                               {loadGV, methodIdConst});
}

llvm::CallInst *
GShaderBuilder::CreateExternalFunctionCall(llvm::BasicBlock *insert, int32_t id,
                                           llvm::ArrayRef<llvm::Value *> args)
{
    llvm::IRBuilder<> builder(insert);
    llvm::FunctionType *funcT = GetExternalFunctionType(*context_, id);
    const char *funcName = GetExternalFunctionName(id);

    llvm::FunctionCallee func = module_->getOrInsertFunction(funcName, funcT);
    return builder.CreateCall(func, args);
}

llvm::LoadInst *GShaderBuilder::LoadHostContextGV(llvm::BasicBlock *insert)
{
    llvm::IRBuilder<> builder(insert);
    llvm::GlobalVariable *gv = module_->getGlobalVariable("__program_host_context");
    return builder.CreateLoad(gv->getValueType(), gv);
}

#define DECL_NEW_INT_CONSTANT(name, width) \
llvm::Constant *GShaderBuilder::New##name (int##width##_t v) { \
    return llvm::ConstantInt::get(llvm::Type::getInt##width##Ty(*context_), v); \
}

#define DECL_NEW_UINT_CONSTANT(name, width) \
llvm::Constant *GShaderBuilder::New##name (uint##width##_t v) { \
    int##width##_t sv = *reinterpret_cast<int##width##_t*>(static_cast<void*>(&v)); \
    return llvm::ConstantInt::get(llvm::Type::getInt##width##Ty(*context_), sv); \
}

DECL_NEW_INT_CONSTANT(SByte, 8)
DECL_NEW_INT_CONSTANT(Short, 16)
DECL_NEW_INT_CONSTANT(Int, 32)
DECL_NEW_INT_CONSTANT(Long, 64)
DECL_NEW_UINT_CONSTANT(Byte, 8)
DECL_NEW_UINT_CONSTANT(UShort, 16)
DECL_NEW_UINT_CONSTANT(UInt, 32)
DECL_NEW_UINT_CONSTANT(ULong, 64)

llvm::Constant *GShaderBuilder::NewFloat(float v)
{
    return llvm::ConstantFP::get(llvm::Type::getFloatTy(*context_), v);
}

#undef DECL_NEW_INT_CONSTANT
#undef DECL_NEW_UINT_CONSTANT

template<typename R, typename...Tp>
llvm::Constant *get_vector_constant(llvm::LLVMContext& ctx, Tp&&...args)
{
    return llvm::ConstantDataVector::get(ctx, llvm::ArrayRef<R>{
        *reinterpret_cast<R*>(static_cast<void*>(&args))...
    });
}

#define DECL_NEW_VEC2_CONSTANT(vec, ctype, rtype)                   \
llvm::Constant *GShaderBuilder::New##vec (ctype x, ctype y) {       \
    return get_vector_constant<rtype>(*context_, x, y);             \
}

#define DECL_NEW_VEC4_CONSTANT(vec, ctype, rtype)                                    \
llvm::Constant *GShaderBuilder::New##vec (ctype x, ctype y, ctype z, ctype w) {      \
    return get_vector_constant<rtype>(*context_, x, y, z, w);                        \
}

DECL_NEW_VEC2_CONSTANT(Byte2, uint8_t, uint8_t)
DECL_NEW_VEC2_CONSTANT(Short2, int16_t, uint16_t)
DECL_NEW_VEC2_CONSTANT(Int2, int32_t, uint32_t)
DECL_NEW_VEC2_CONSTANT(Long2, int64_t, uint64_t)
DECL_NEW_VEC2_CONSTANT(Float2, float, float)
DECL_NEW_VEC2_CONSTANT(SByte2, int8_t, uint8_t)
DECL_NEW_VEC2_CONSTANT(UShort2, uint16_t, uint16_t)
DECL_NEW_VEC2_CONSTANT(UInt2, uint32_t, uint32_t)
DECL_NEW_VEC2_CONSTANT(ULong2, uint64_t, uint64_t)
DECL_NEW_VEC4_CONSTANT(Byte4, uint8_t, uint8_t)
DECL_NEW_VEC4_CONSTANT(Short4, int16_t, uint16_t)
DECL_NEW_VEC4_CONSTANT(Int4, int32_t, uint32_t)
DECL_NEW_VEC4_CONSTANT(Long4, int64_t, uint64_t)
DECL_NEW_VEC4_CONSTANT(Float4, float, float)
DECL_NEW_VEC4_CONSTANT(SByte4, int8_t, uint8_t)
DECL_NEW_VEC4_CONSTANT(UShort4, uint16_t, uint16_t)
DECL_NEW_VEC4_CONSTANT(UInt4, uint32_t, uint32_t)
DECL_NEW_VEC4_CONSTANT(ULong4, uint64_t, uint64_t)

#undef DECL_NEW_VEC2_CONSTANT
#undef DECL_NEW_VEC4_CONSTANT

#define DECL_NEW_UNDEF(ctype, rtype)                                     \
llvm::Value *GShaderBuilder::New##ctype() {                              \
    return llvm::UndefValue::get(llvm::Type::get##rtype##Ty(*context_)); \
}

#define DECL_NEW_VEC_UNDEF(ctype, rtype, count)                 \
llvm::Value *GShaderBuilder::New##ctype() {                     \
    return llvm::UndefValue::get(llvm::FixedVectorType::get(    \
        llvm::Type::get##rtype##Ty(*context_), count));         \
}

DECL_NEW_UNDEF(Byte, Int8)
DECL_NEW_UNDEF(SByte, Int8)
DECL_NEW_UNDEF(Short, Int16)
DECL_NEW_UNDEF(UShort, Int16)
DECL_NEW_UNDEF(Int, Int32)
DECL_NEW_UNDEF(UInt, Int32)
DECL_NEW_UNDEF(Long, Int64)
DECL_NEW_UNDEF(ULong, Int64)
DECL_NEW_UNDEF(Float, Float)

DECL_NEW_VEC_UNDEF(Byte2, Int8, 2)
DECL_NEW_VEC_UNDEF(Byte4, Int8, 4)
DECL_NEW_VEC_UNDEF(SByte2, Int8, 2)
DECL_NEW_VEC_UNDEF(SByte4, Int8, 4)
DECL_NEW_VEC_UNDEF(Short2, Int16, 2)
DECL_NEW_VEC_UNDEF(Short4, Int16, 4)
DECL_NEW_VEC_UNDEF(UShort2, Int16, 2)
DECL_NEW_VEC_UNDEF(UShort4, Int16, 4)
DECL_NEW_VEC_UNDEF(Int2, Int32, 2)
DECL_NEW_VEC_UNDEF(Int4, Int32, 4)
DECL_NEW_VEC_UNDEF(UInt2, Int32, 2)
DECL_NEW_VEC_UNDEF(UInt4, Int32, 4)
DECL_NEW_VEC_UNDEF(Long2, Int64, 2)
DECL_NEW_VEC_UNDEF(Long4, Int64, 4)
DECL_NEW_VEC_UNDEF(ULong2, Int64, 2)
DECL_NEW_VEC_UNDEF(ULong4, Int64, 4)
DECL_NEW_VEC_UNDEF(Float2, Float, 2)
DECL_NEW_VEC_UNDEF(Float4, Float, 4)

#undef DECL_NEW_UNDEF
#undef DECL_NEW_VEC_UNDEF

void GShaderBuilder::MainTestCodeGen()
{
    using namespace llvm;
}

REACTOR_NAMESPACE_END
