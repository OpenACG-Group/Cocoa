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

#include "llvm/ADT/StringRef.h"
#include "llvm/ExecutionEngine/JITSymbol.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/Orc/Core.h"
#include "llvm/ExecutionEngine/Orc/ExecutorProcessControl.h"
#include "llvm/ExecutionEngine/Orc/IRCompileLayer.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/ExecutionEngine/JITEventListener.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Verifier.h"

#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"

#include "fmt/format.h"
#include "Core/Journal.h"
#include "Reactor/Reactor.h"
#include "Reactor/JitSession.h"
#include "Reactor/GShaderExternals.h"
REACTOR_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Reactor.GShaderModule)

class JITEventListener : public llvm::JITEventListener
{
public:
    void notifyFreeingObject(ObjectKey objectKey) override
    {
        QLOG(LOG_DEBUG, "JIT object freed: key={}", objectKey);
    }

    void notifyObjectLoaded(ObjectKey objectKey,
                            const llvm::object::ObjectFile &file,
                            const llvm::RuntimeDyld::LoadedObjectInfo &info) override
    {
        QLOG(LOG_DEBUG, "JIT object loaded: key={}", objectKey);
    }
};

std::shared_ptr<GShaderModule> GShaderModule::Compile(GShaderBuilder& builder)
{
    std::unique_ptr<llvm::Module> module(std::move(builder.module_));
    std::unique_ptr<llvm::LLVMContext> context(std::move(builder.context_));
    std::vector<llvm::Function *> functions(std::move(builder.exposed_functions_));

    std::string verifyMessages;
    llvm::raw_string_ostream stream(verifyMessages);
    if (llvm::verifyModule(*module, &stream))
        throw std::runtime_error(verifyMessages);

    auto *hostContext = new HostContext{};
    hostContext->magic_number = external::HOST_CTX_MAGIC_NUMBER;
    hostContext->v8_method_map = std::move(builder.v8_method_map_);

    llvm::legacy::PassManager passes;
    Bitfield<Options::CodeOptPass> op = JitSession::Ref().GetOptions().codegen_opt_passes;

    using P = Options::CodeOptPass;
    if (op & P::kCFGSimplification)
        passes.add(llvm::createCFGSimplificationPass());
    if (op & P::kReassociate)
        passes.add(llvm::createReassociatePass());
    if (op & P::kLICM)
        passes.add(llvm::createLICMPass());
    if (op & P::kAggressiveDCE)
        passes.add(llvm::createAggressiveDCEPass());
    if (op & P::kGVN)
        passes.add(llvm::createNewGVNPass());
    if (op & P::kInstructionCombining)
        passes.add(llvm::createInstructionCombiningPass());
    if (op & P::kDeadStoreElimination)
        passes.add(llvm::createDeadStoreEliminationPass());
    if (op & P::kSCCP)
        passes.add(llvm::createSCCPPass());
    if (op & P::kSROA)
        passes.add(llvm::createSROAPass());
    if (op & P::kEarlyCSE)
        passes.add(llvm::createEarlyCSEPass());

    passes.run(*module);

    module->print(llvm::outs(), nullptr);

    return std::make_shared<GShaderModule>(std::move(module),
                                           std::move(context),
                                           builder.name_,
                                           std::move(functions),
                                           hostContext);
}

GShaderModule::GShaderModule(std::unique_ptr<llvm::Module> module,
                             std::unique_ptr<llvm::LLVMContext> context,
                             std::string name,
                             std::vector<llvm::Function *> funcs,
                             HostContext *hostContext)
    : name_(std::move(name))
    , session_(std::move(*llvm::orc::SelfExecutorProcessControl::Create()))
    , object_layer_(session_, []() {
        static MemoryMapper memoryMapper;
        return std::make_unique<llvm::SectionMemoryManager>(&memoryMapper);
    })
    , host_context_(hostContext)
{
    static JITEventListener listener;
    object_layer_.registerJITEventListener(listener);

    if (JitSession::Ref().GetTargetTriple().isOSBinFormatCOFF())
    {
        // Hack to support symbol visibility in COFF.
        // Matches hack in llvm::orc::LLJIT::createObjectLinkingLayer().
        // See documentation on these functions for more detail.
        object_layer_.setOverrideObjectFlagsWithResponsibilityFlags(true);
        object_layer_.setAutoClaimResponsibilityForObjectSymbols(true);
    }

    llvm::SmallVector<llvm::orc::SymbolStringPtr, 8> functionNames(funcs.size());
    llvm::orc::MangleAndInterner mangler(session_, JitSession::Ref().GetDataLayout());

    for (size_t i = 0; i < funcs.size(); i++)
    {
        llvm::Function *func = funcs[i];
        if (!func->hasName())
        {
            func->setName("__anonymous_f" + llvm::Twine(i).str());
        }
        functionNames[i] = mangler(func->getName());
    }

    /* Once the module is passed to the compilerLayer, the llvm::Functions are freed.
     * Make sure funcs are not referenced after this point. */
    funcs.clear();

    auto concurrentCompiler = std::make_unique<llvm::orc::ConcurrentIRCompiler>(
            JitSession::Ref().GetTargetMachineBuilder());

    llvm::orc::IRCompileLayer compileLayer(session_,
                                           object_layer_,
                                           std::move(concurrentCompiler));

    llvm::orc::JITDylib& dylib(session_.createJITDylib("<gshadermodule>").get());
    dylib.addGenerator(std::make_unique<ExternalSymbolGenerator>());

    llvm::cantFail(compileLayer.add(dylib,
                                    llvm::orc::ThreadSafeModule(std::move(module), std::move(context))));

    for (size_t i = 0; i < functionNames.size(); i++)
    {
        auto symbol = session_.lookup({ &dylib }, functionNames[i]);
        if (!symbol)
        {
            QLOG(LOG_ERROR, "Failed to lookup address of compiled function {}: {}",
                 (*functionNames[i]).str(), llvm::toString(symbol.takeError()));
        }
        function_entries_[(*functionNames[i]).str()] = reinterpret_cast<void *>(
                static_cast<intptr_t>(symbol->getAddress()));
    }
}

GShaderModule::~GShaderModule()
{
    delete host_context_;
}

bool GShaderModule::Execute()
{
    using GShaderStartUserPfn = int32_t(*)(void *);

    if (function_entries_.count(GSHADER_ENTRY_NAME) == 0)
        return false;

    auto *start_user_main_pfn = reinterpret_cast<GShaderStartUserPfn>(
            function_entries_[GSHADER_ENTRY_NAME]);
    if (!start_user_main_pfn)
        return false;

    return (start_user_main_pfn(host_context_) == external::START_USER_RET_NORMAL);
}

REACTOR_NAMESPACE_END
