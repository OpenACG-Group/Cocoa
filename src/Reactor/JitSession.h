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

    g_nodiscard const llvm::orc::JITTargetMachineBuilder& GetTargetMachineBuilder();
    g_nodiscard const llvm::DataLayout& GetDataLayout() const;
    g_nodiscard const llvm::Triple& GetTargetTriple() const;

    g_nodiscard const Options& GetOptions() const {
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

