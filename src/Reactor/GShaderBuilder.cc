#include "Reactor/Reactor.h"
#include "Reactor/JitSession.h"
#include "Reactor/GShaderExternals.h"
REACTOR_NAMESPACE_BEGIN

GShaderBuilder::GShaderBuilder(const std::string& name)
    : name_(name)
    , context_(std::make_unique<llvm::LLVMContext>())
    , module_(std::make_unique<llvm::Module>(name, *context_))
    , ir_builder_(std::make_unique<llvm::IRBuilder<>>(*context_))
    , main_function_(nullptr)
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

    /* int32_t __builtin_check_host_context(void *ctx) */
    FunctionType *checkHostCtxFuncT = FunctionType::get(Type::getInt32Ty(*context_),
                                                        { hostCtxPtrT }, false);
    FunctionCallee checkHostCtxFunc = module_->getOrInsertFunction("__builtin_check_host_context",
                                                                   checkHostCtxFuncT);

    BasicBlock *retNormalBlock = BasicBlock::Create(*context_, "normal_ret", startUserFunc);
    {
        BasicBlock *retFailBlock = BasicBlock::Create(*context_, "check_failed", startUserFunc);

        /* call = __builtin_check_host_context(__program_host_context) */
        LoadInst *loadGV = builder.CreateLoad(hostCtxPtrT, hostContextGV);
        CallInst *call = builder.CreateCall(checkHostCtxFunc, { loadGV });
        Value *cond = builder.CreateICmpNE(call, ConstantInt::get(Type::getInt32Ty(*context_), 0));
        builder.CreateCondBr(cond, retFailBlock, retNormalBlock);

        Value *retValue = ConstantInt::get(Type::getInt32Ty(*context_), external::START_USER_RET_FAILED);
        ReturnInst::Create(*context_, retValue, retFailBlock);
    }

    /* void main() */
    FunctionType *mainFuncT = FunctionType::get(Type::getVoidTy(*context_), {}, false);
    main_function_ = Function::Create(mainFuncT, Function::ExternalLinkage, "main", *module_);

    /* main() */
    CallInst::Create(main_function_, {}, retNormalBlock);

    Value *retValue = ConstantInt::get(Type::getInt32Ty(*context_), external::START_USER_RET_NORMAL);
    ReturnInst::Create(*context_, retValue, retNormalBlock);
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

    FunctionType *funcT = FunctionType::get(Type::getVoidTy(*context_),
                                            {loadGV->getType(), methodIdConst->getType()}, false);
    FunctionCallee func = module_->getOrInsertFunction("__builtin_v8_trampoline", funcT);
    builder.CreateCall(func, { loadGV, methodIdConst });
}

void GShaderBuilder::MainTestCodeGen()
{
    using namespace llvm;

    BasicBlock *bb = BasicBlock::Create(*context_, "", main_function_);
    IRBuilder<> builder(bb);

    CreateBuiltinV8FunctionCall(bb, "test");

    builder.CreateRetVoid();
}

REACTOR_NAMESPACE_END
