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

#include "llvm/ExecutionEngine/Orc/ExecutorProcessControl.h"
#include "llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h"
#include "llvm/IR/DataLayout.h"

#include "Core/Errors.h"
#include "Core/Journal.h"
#include "Reactor/Reactor.h"
#include "Reactor/JitSession.h"
#include "Reactor/ExecutableMemory.h"
#include "Reactor/GShaderExternals.h"
REACTOR_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Reactor.JitSession)

JitSession::JitSession(const Options& options,
                       llvm::orc::JITTargetMachineBuilder&& targetMachineBuilder,
                       llvm::DataLayout&& dataLayout)
    : options_(options)
    , target_machine_builder_(std::move(targetMachineBuilder))
    , data_layout_(dataLayout)
{
    llvm::CodeGenOpt::Level level;
    switch (options_.codegen_opt_level)
    {
    case Options::CodeOptLevels::kNone: level = llvm::CodeGenOpt::None;             break;
    case Options::CodeOptLevels::kLess: level = llvm::CodeGenOpt::Less;             break;
    case Options::CodeOptLevels::kDefault: level = llvm::CodeGenOpt::Default;       break;
    case Options::CodeOptLevels::kAggressive: level = llvm::CodeGenOpt::Aggressive; break;
    default:
        MARK_UNREACHABLE("Unknown optimization level");
    }
    target_machine_builder_.setCodeGenOptLevel(level);
}

const llvm::orc::JITTargetMachineBuilder& JitSession::GetTargetMachineBuilder()
{
    return target_machine_builder_;
}

const llvm::DataLayout& JitSession::GetDataLayout() const
{
    return data_layout_;
}

const llvm::Triple& JitSession::GetTargetTriple() const
{
    return target_machine_builder_.getTargetTriple();
}

namespace {

Bitfield<MemPermission> flags_to_permissions(unsigned int flags)
{
    Bitfield<MemPermission> r;
    if (flags & llvm::sys::Memory::MF_READ)
        r |= MemPermission::kRead;
    if (flags & llvm::sys::Memory::MF_WRITE)
        r |= MemPermission::kWrite;
    if (flags & llvm::sys::Memory::MF_EXEC)
        r |= MemPermission::kExecute;
    return r;
}

}

llvm::sys::MemoryBlock
MemoryMapper::allocateMappedMemory(llvm::SectionMemoryManager::AllocationPurpose Purpose,
                                   size_t NumBytes,
                                   const llvm::sys::MemoryBlock *const NearBlock,
                                   unsigned int Flags,
                                   std::error_code& EC)
{
    EC = std::error_code();

    size_t pageSize = MemoryPageSize();
    NumBytes = (NumBytes + pageSize - 1) & ~(pageSize - 1);

    bool need_exec = (Purpose == llvm::SectionMemoryManager::AllocationPurpose::Code);
    void *addr = AllocateMemoryPages(NumBytes, flags_to_permissions(Flags), need_exec);

    if (!addr)
        return {};
    return {addr, NumBytes};
}

std::error_code MemoryMapper::protectMappedMemory(const llvm::sys::MemoryBlock& Block, unsigned int Flags)
{
    void *addr = Block.base();
    size_t size = Block.allocatedSize();
    size_t pageSize = MemoryPageSize();

    addr = reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(addr) & ~(pageSize - 1));
    size += reinterpret_cast<uintptr_t>(Block.base()) - reinterpret_cast<uintptr_t>(addr);

    ProtectMemoryPages(addr, size, flags_to_permissions(Flags));
    return {};
}

std::error_code MemoryMapper::releaseMappedMemory(llvm::sys::MemoryBlock& M)
{
    DeallocateMemoryPages(M.base(), M.allocatedSize());
    return {};
}

llvm::Error
ExternalSymbolGenerator::tryToGenerate(llvm::orc::LookupState& lookupState,
                                       llvm::orc::LookupKind lookupKind,
                                       llvm::orc::JITDylib& JD,
                                       llvm::orc::JITDylibLookupFlags JDLookupFlags,
                                       const llvm::orc::SymbolLookupSet& lookupSet)
{
    const llvm::StringMap<void*>& external = GetExternalSymbolMap();
    llvm::orc::SymbolMap symbols;

    std::string missing;

    for (const auto& symbol : lookupSet)
    {
        llvm::orc::SymbolStringPtr name = symbol.first;
        auto it = external.find((*name).str());
        if (it != external.end())
        {
            symbols[name] = llvm::JITEvaluatedSymbol(
                    static_cast<llvm::JITTargetAddress>(reinterpret_cast<uintptr_t>(it->second)),
                    llvm::JITSymbolFlags::Exported);
            continue;
        }

        missing = (missing.empty() ? "'" : ", '") + (*name).str() + "'";
    }

    if (!missing.empty())
    {
        QLOG(LOG_WARNING, "Missing external symbols: {}", missing);
    }

    if (symbols.empty())
        return llvm::Error::success();
    return JD.define(llvm::orc::absoluteSymbols(std::move(symbols)));
}

REACTOR_NAMESPACE_END
