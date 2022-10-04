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

#ifndef COCOA_REACTOR_REACTOR_H
#define COCOA_REACTOR_REACTOR_H

#include <memory>

#include "llvm/ExecutionEngine/Orc/Core.h"
#include "llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h"
#include "llvm/IR/IRBuilder.h"

#include "include/v8.h"

#include "Core/EnumClassBitfield.h"
#include "Core/Project.h"

/**
 * Reactor: A runtime machine code generator based on LLVM JIT compiler.
 */

#define REACTOR_NAMESPACE_BEGIN         namespace cocoa::reactor {
#define REACTOR_NAMESPACE_END           }

REACTOR_NAMESPACE_BEGIN

#define GSHADER_ENTRY_NAME          "__start_user_main"
#define GSHADER_USER_ENTRY_NAME     "main"

struct Options
{
    enum class CodeOptLevels
    {
        kNone,
        kLess,
        kDefault,
        kAggressive
    };

    enum class CodeOptPass : uint32_t
    {
        kCFGSimplification      = 1 << 1,
        kLICM                   = 1 << 2,
        kAggressiveDCE          = 1 << 3,
        kGVN                    = 1 << 4,
        kInstructionCombining   = 1 << 5,
        kReassociate            = 1 << 6,
        kDeadStoreElimination   = 1 << 7,
        kSCCP                   = 1 << 8,
        kSROA                   = 1 << 9,
        kEarlyCSE               = 1 << 10
    };

    CodeOptLevels               codegen_opt_level{CodeOptLevels::kDefault};
    Bitfield<CodeOptPass>       codegen_opt_passes{};
};

void InitializePlatform(const Options& options);
void DisposePlatform();

class GShaderBuilder;
class GShaderModule;

class GShaderBuilder
{
    friend class GShaderModule;

public:
    explicit GShaderBuilder(const std::string& name);
    ~GShaderBuilder();

    /* Create constants */
    llvm::Constant *NewSByte(int8_t v);
    llvm::Constant *NewShort(int16_t v);
    llvm::Constant *NewInt(int32_t v);
    llvm::Constant *NewLong(int64_t v);
    llvm::Constant *NewByte(uint8_t v);
    llvm::Constant *NewUShort(uint16_t v);
    llvm::Constant *NewUInt(uint32_t v);
    llvm::Constant *NewULong(uint64_t v);
    llvm::Constant *NewFloat(float v);
    llvm::Constant *NewByte2(uint8_t x, uint8_t y);
    llvm::Constant *NewShort2(int16_t x, int16_t y);
    llvm::Constant *NewInt2(int32_t x, int32_t y);
    llvm::Constant *NewLong2(int64_t x, int64_t y);
    llvm::Constant *NewFloat2(float x, float y);
    llvm::Constant *NewSByte2(int8_t x, int8_t y);
    llvm::Constant *NewUShort2(uint16_t x, uint16_t y);
    llvm::Constant *NewUInt2(uint32_t x, uint32_t y);
    llvm::Constant *NewULong2(uint64_t x, uint64_t y);
    llvm::Constant *NewByte4(uint8_t x, uint8_t y, uint8_t z, uint8_t w);
    llvm::Constant *NewShort4(int16_t x, int16_t y, int16_t z, int16_t w);
    llvm::Constant *NewInt4(int32_t x, int32_t y, int32_t z, int32_t w);
    llvm::Constant *NewLong4(int64_t x, int64_t y, int64_t z, int64_t w);
    llvm::Constant *NewFloat4(float x, float y, float z, float w);
    llvm::Constant *NewSByte4(int8_t x, int8_t y, int8_t z, int8_t w);
    llvm::Constant *NewUShort4(uint16_t x, uint16_t y, uint16_t z, uint16_t w);
    llvm::Constant *NewUInt4(uint32_t x, uint32_t y, uint32_t z, uint32_t w);
    llvm::Constant *NewULong4(uint64_t x, uint64_t y, uint64_t z, uint64_t w);

    llvm::Value *NewByte();
    llvm::Value *NewByte2();
    llvm::Value *NewByte4();
    llvm::Value *NewSByte();
    llvm::Value *NewSByte2();
    llvm::Value *NewSByte4();
    llvm::Value *NewShort();
    llvm::Value *NewShort2();
    llvm::Value *NewShort4();
    llvm::Value *NewUShort();
    llvm::Value *NewUShort2();
    llvm::Value *NewUShort4();
    llvm::Value *NewInt();
    llvm::Value *NewInt2();
    llvm::Value *NewInt4();
    llvm::Value *NewUInt();
    llvm::Value *NewUInt2();
    llvm::Value *NewUInt4();
    llvm::Value *NewLong();
    llvm::Value *NewLong2();
    llvm::Value *NewLong4();
    llvm::Value *NewULong();
    llvm::Value *NewULong2();
    llvm::Value *NewULong4();
    llvm::Value *NewFloat();
    llvm::Value *NewFloat2();
    llvm::Value *NewFloat4();

    void InsertV8FunctionSymbol(v8::Local<v8::Function> func, const std::string& name);

    void CreateBuiltinV8FunctionCall(llvm::BasicBlock *insert, const std::string& name);
    llvm::CallInst *CreateExternalFunctionCall(llvm::BasicBlock *insert, int32_t id,
                                               llvm::ArrayRef<llvm::Value *> args = llvm::None);
    llvm::LoadInst *LoadHostContextGV(llvm::BasicBlock *insert);

    g_nodiscard inline llvm::BasicBlock *GetMainEntrypointBasicBlock() {
        return main_basic_block_;
    }

private:
    void CreateEntryFunction();

    std::string                         name_;
    std::unique_ptr<llvm::LLVMContext>  context_;
    std::unique_ptr<llvm::Module>       module_;
    std::vector<llvm::Function*>        exposed_functions_;
    llvm::Function                     *main_function_;
    llvm::BasicBlock                   *main_basic_block_;

    std::map<std::string, uint32_t>     v8_method_id_map_;
    std::map<uint32_t, v8::Global<v8::Function>> v8_method_map_;
};

class GShaderModule
{
public:
    struct HostContext
    {
        uint32_t            magic_number;
        std::map<uint32_t, v8::Global<v8::Function>> v8_method_map;
    };

    GShaderModule(std::unique_ptr<llvm::Module> module,
                  std::unique_ptr<llvm::LLVMContext> context,
                  std::string name,
                  std::vector<llvm::Function*> funcs,
                  HostContext *hostContext);
    ~GShaderModule();

    g_nodiscard static std::shared_ptr<GShaderModule> Compile(GShaderBuilder& builder);

    bool Execute();

private:
    std::string                         name_;
    llvm::orc::ExecutionSession         session_;
    llvm::orc::RTDyldObjectLinkingLayer object_layer_;
    std::map<std::string, void*>        function_entries_;
    HostContext                        *host_context_;
};

REACTOR_NAMESPACE_END
#endif //COCOA_REACTOR_REACTOR_H
