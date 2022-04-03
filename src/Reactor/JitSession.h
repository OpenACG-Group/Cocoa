#ifndef COCOA_REACTOR_JITSESSION_H
#define COCOA_REACTOR_JITSESSION_H

#include "llvm/ExecutionEngine/Orc/Core.h"
#include "llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/IR/DataLayout.h"

#include "Core/UniquePersistent.h"
#include "Reactor/Reactor.h"
REACTOR_NAMESPACE_BEGIN

class JitSession : public UniquePersistent<JitSession>
{
public:
    JitSession(const Options& options,
               llvm::orc::JITTargetMachineBuilder&& targetMachineBuilder,
               llvm::DataLayout&& dataLayout);
    ~JitSession() = default;

    co_nodiscard const llvm::orc::JITTargetMachineBuilder& GetTargetMachineBuilder();
    co_nodiscard const llvm::DataLayout& GetDataLayout() const;
    co_nodiscard const llvm::Triple& GetTargetTriple() const;

    co_nodiscard const Options& GetOptions() const {
        return options_;
    }

private:
    Options                                     options_;
    llvm::orc::JITTargetMachineBuilder          target_machine_builder_;
    const llvm::DataLayout                      data_layout_;
};

class MemoryMapper final : public llvm::SectionMemoryManager::MemoryMapper
{
public:
    MemoryMapper() = default;
    ~MemoryMapper() final = default;

    llvm::sys::MemoryBlock allocateMappedMemory(llvm::SectionMemoryManager::AllocationPurpose Purpose,
                                                size_t NumBytes,
                                                const llvm::sys::MemoryBlock *NearBlock,
                                                unsigned int Flags,
                                                std::error_code &EC) final;

    std::error_code protectMappedMemory(const llvm::sys::MemoryBlock &Block,
                                        unsigned int Flags) final;

    std::error_code releaseMappedMemory(llvm::sys::MemoryBlock &M) override;
};

class ExternalSymbolGenerator : public llvm::orc::DefinitionGenerator
{
public:
    ~ExternalSymbolGenerator() override = default;

    llvm::Error tryToGenerate(llvm::orc::LookupState &lookupState,
                              llvm::orc::LookupKind lookupKind,
                              llvm::orc::JITDylib &JD,
                              llvm::orc::JITDylibLookupFlags JDLookupFlags,
                              const llvm::orc::SymbolLookupSet &lookupSet) override;
};

REACTOR_NAMESPACE_END
#endif //#define COCOA_REACTOR_JITSESSION_H

