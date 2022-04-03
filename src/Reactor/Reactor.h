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

    void InsertV8FunctionSymbol(v8::Local<v8::Function> func, const std::string& name);

    void CreateBuiltinV8FunctionCall(llvm::BasicBlock *insert, const std::string& name);

    void MainTestCodeGen();

private:
    void CreateEntryFunction();

    std::string                         name_;
    std::unique_ptr<llvm::LLVMContext>  context_;
    std::unique_ptr<llvm::Module>       module_;
    std::unique_ptr<llvm::IRBuilder<>>  ir_builder_;
    std::vector<llvm::Function*>        exposed_functions_;
    llvm::Function                     *main_function_;

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

    co_nodiscard static std::shared_ptr<GShaderModule> Compile(GShaderBuilder& builder);

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
