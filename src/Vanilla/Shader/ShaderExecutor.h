#ifndef COCOA_SHADEREXECUTOR_H
#define COCOA_SHADEREXECUTOR_H

#include "llvm/ExecutionEngine/Orc/LLJIT.h"

#include "Vanilla/Base.h"

struct VaShaderLDT;
VANILLA_NS_BEGIN

class ShaderModule;
class ShaderContext;

class ShaderExecutor
{
public:
    explicit ShaderExecutor(UniqueHandle<llvm::orc::LLJIT>&& jit, const VaShaderLDT *ldt);
    ~ShaderExecutor();

    ShaderExecutor(const ShaderExecutor&) = delete;
    ShaderExecutor& operator=(const ShaderExecutor&) = delete;

    static Handle<ShaderExecutor> Create();

    void addModule(Handle<ShaderModule> module);
    void execute(const std::string& name, Handle<ShaderContext> context);

private:
    UniqueHandle<llvm::orc::LLJIT>      fJit;
        llvm::orc::JITDylib            *fShaderLibrary;
        const VaShaderLDT              *fpLDT;
};

VANILLA_NS_END
#endif //COCOA_SHADEREXECUTOR_H
