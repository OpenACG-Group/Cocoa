#ifndef COCOA_SHADERMODULE_H
#define COCOA_SHADERMODULE_H

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/ExecutionEngine/Orc/ThreadSafeModule.h"

#include "Komorebi/Namespace.h"
#include "Komorebi/ShaderContext.h"
#include "Komorebi/ShaderProgram.h"
KOMOREBI_NS_BEGIN

class ShaderModule
{
public:
    ShaderModule(std::unique_ptr<llvm::Module> module,
                 std::unique_ptr<llvm::LLVMContext> context,
                 std::shared_ptr<ShaderProgram> program);
    ~ShaderModule() = default;

    static std::unique_ptr<ShaderModule> MakeFromSource(const std::string& name,
                                                        std::shared_ptr<ShaderProgram> program,
                                                        const char *source);

    void moveToProgram();
    inline bool isMoved()           { return fModule == nullptr; }
    inline llvm::Module& module()   { return *fModule; }

private:
    std::unique_ptr<llvm::LLVMContext>  fContext;
    std::unique_ptr<llvm::Module>       fModule;
    std::shared_ptr<ShaderProgram>      fProgram;
};

KOMOREBI_NS_END
#endif // COCOA_SHADERMODULE_H
