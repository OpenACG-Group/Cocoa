#include "llvm/Support/TargetSelect.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"

#include "Core/Exception.h"
#include "Komorebi/Namespace.h"
#include "Komorebi/ShaderProgram.h"
KOMOREBI_NS_BEGIN

void InitializeShaderCompiler()
{
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmParser();
    llvm::InitializeNativeTargetAsmPrinter();
}

void DisposeShaderCompiler()
{
}

// --------------------------------------------------------------------------------------------

// .dso means "Dynamic Shared Object"
#define SHADER_MAIN_JITDYLIB_NAME   "libStdShader.dso"

std::shared_ptr<ShaderProgram> ShaderProgram::Make()
{
    auto ssp = std::make_shared<llvm::orc::SymbolStringPool>();
    auto tpc = llvm::orc::SelfTargetProcessControl::Create(ssp);
    if (!tpc)
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("Failed to initialize shader compiler: ")
                .append(llvm::toString(tpc.takeError()))
                .make<RuntimeException>();
    }

    auto executionSession = std::make_unique<llvm::orc::ExecutionSession>(std::move(ssp));
    llvm::orc::JITTargetMachineBuilder machineBuilder((*tpc)->getTargetTriple());

    auto dataLayout = machineBuilder.getDefaultDataLayoutForTarget();
    if (!dataLayout)
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("Failed to get default data layout: ")
                .append(llvm::toString(dataLayout.takeError()))
                .make<RuntimeException>();
    }

    return std::make_shared<ShaderProgram>(std::move(*tpc),
                                           std::move(executionSession),
                                           std::move(machineBuilder),
                                           std::move(*dataLayout));
}

ShaderProgram::ShaderProgram(std::unique_ptr<llvm::orc::TargetProcessControl> tpc,
                                           std::unique_ptr<llvm::orc::ExecutionSession> es,
                                           llvm::orc::JITTargetMachineBuilder machineBuilder,
                                           const llvm::DataLayout& layout)
    : fTpc(std::move(tpc)),
      fExecutionSession(std::move(es)),
      fDataLayout(layout),
      fOrcMangle(*fExecutionSession, fDataLayout),
      fObjectLinkingLayer(*fExecutionSession,
                          []() { return std::make_unique<llvm::SectionMemoryManager>(); }),
      fCompileLayer(*fExecutionSession, fObjectLinkingLayer,
                    std::make_unique<llvm::orc::ConcurrentIRCompiler>(std::move(machineBuilder))),
      fJitDynamicLib(fExecutionSession->createBareJITDylib(SHADER_MAIN_JITDYLIB_NAME))
{
    fJitDynamicLib.addGenerator(llvm::cantFail(
            llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(
                    fDataLayout.getGlobalPrefix())));
}

ShaderProgram::~ShaderProgram()
{
    if (auto err = fExecutionSession->endSession())
        fExecutionSession->reportError(std::move(err));
}

void ShaderProgram::addModule(std::unique_ptr<llvm::Module> module,
                              std::unique_ptr<llvm::LLVMContext> context,
                              llvm::orc::ResourceTrackerSP tracker)
{
    if (!tracker)
        tracker = fJitDynamicLib.getDefaultResourceTracker();
    auto ret = fCompileLayer.add(tracker,
                                 llvm::orc::ThreadSafeModule(std::move(module), std::move(context)));
    if (ret)
    {
        throw RuntimeException::Builder(__FUNCTION__)
            .append("Failed to add LLVM IR module to JIT dynamic library: ")
            .append(llvm::toString(std::move(ret)))
            .make<RuntimeException>();
    }
}

llvm::Expected<llvm::JITEvaluatedSymbol> ShaderProgram::lookupSymbol(const std::string& sym)
{
    llvm::StringRef symRef(sym);
    return fExecutionSession->lookup({&fJitDynamicLib},
                                     symRef,
                                     llvm::orc::SymbolState::Ready);
}

KOMOREBI_NS_END
