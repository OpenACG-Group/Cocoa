#ifndef COCOA_SHADERPROGRAM_H
#define COCOA_SHADERPROGRAM_H

#include <memory>

#include "llvm/ADT/StringRef.h"
#include "llvm/ExecutionEngine/JITSymbol.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/Orc/Core.h"
#include "llvm/ExecutionEngine/Orc/ExecutionUtils.h"
#include "llvm/ExecutionEngine/Orc/IRCompileLayer.h"
#include "llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h"
#include "llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h"
#include "llvm/ExecutionEngine/Orc/TargetProcessControl.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/LLVMContext.h"

#include "Core/UniquePersistent.h"
#include "Komorebi/Namespace.h"
KOMOREBI_NS_BEGIN

void InitializeShaderCompiler();
void DisposeShaderCompiler();

class ShaderProgram
{
public:
    ShaderProgram(std::unique_ptr<llvm::orc::TargetProcessControl> tpc,
                  std::unique_ptr<llvm::orc::ExecutionSession> es,
                  llvm::orc::JITTargetMachineBuilder machineBuilder,
                  const llvm::DataLayout& layout);
    ~ShaderProgram();

    static std::shared_ptr<ShaderProgram> Make();

    void addModule(std::unique_ptr<llvm::Module> module,
                   std::unique_ptr<llvm::LLVMContext> context,
                   llvm::orc::ResourceTrackerSP tracker = nullptr);
    /**
     * Note that this function will trigger the compilation of sym implicitly.
     */
    llvm::Expected<llvm::JITEvaluatedSymbol> lookupSymbol(const std::string& sym);

    inline llvm::DataLayout& getDataLayout() { return fDataLayout; }

private:
    std::unique_ptr<llvm::orc::TargetProcessControl>    fTpc;
    std::unique_ptr<llvm::orc::ExecutionSession>        fExecutionSession;
    llvm::DataLayout                                    fDataLayout;
    llvm::orc::MangleAndInterner                        fOrcMangle;
    llvm::orc::RTDyldObjectLinkingLayer                 fObjectLinkingLayer;
    llvm::orc::IRCompileLayer                           fCompileLayer;
    llvm::orc::JITDylib&                                fJitDynamicLib;
};

KOMOREBI_NS_END
#endif //COCOA_SHADERPROGRAM_H
