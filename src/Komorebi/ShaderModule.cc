#include <memory>
#include <iostream>
#include <sstream>

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/DerivedTypes.h"
#include "Komorebi/Namespace.h"
#include "Komorebi/ShaderModule.h"
#include "Komorebi/ShaderProgram.h"
#include "Komorebi/KRSLParserDriver.h"
KOMOREBI_NS_BEGIN

bool CompileShaderLanguage(const char *source, const std::unique_ptr<llvm::Module>& module)
{
    llvm::LLVMContext& ctx = module->getContext();
    std::ostringstream compileOStream;
    std::istringstream compileIStream(source);

    KRSLParserDriver driver;
    driver.parse(compileIStream, compileOStream);

    auto func = llvm::Function::Create(llvm::FunctionType::get(llvm::Type::getInt32Ty(ctx),
                                                               {},
                                                               false),
                                       llvm::Function::ExternalLinkage,
                                       "main",
                                       *module);
    auto basicBlock = llvm::BasicBlock::Create(ctx, "Entry", func);
    llvm::IRBuilder<> builder(basicBlock);

    llvm::Value *v1 = llvm::UndefValue::get(
                        llvm::VectorType::get(llvm::Type::getDoubleTy(ctx),
                                                 4,
                                                 false));
    llvm::Value *p = builder.CreateLoad(v1, "dd");

    builder.CreateRet(builder.getInt32(2233));
    return true;
}

std::unique_ptr<ShaderModule> ShaderModule::MakeFromSource(const std::string& name,
                                                           std::shared_ptr<ShaderProgram> program,
                                                           const char *source)
{
    auto context = std::make_unique<llvm::LLVMContext>();
    auto module = std::make_unique<llvm::Module>(name, *context);
    assert(context != nullptr && module != nullptr);
    module->setDataLayout(program->getDataLayout());

    if (!CompileShaderLanguage(source, module))
        return nullptr;

    std::cout << "LLVM module: " << std::endl;
    module->print(llvm::outs(), nullptr);

    return std::make_unique<ShaderModule>(std::move(module),
                                          std::move(context),
                                          std::move(program));
}

ShaderModule::ShaderModule(std::unique_ptr<llvm::Module> module,
                           std::unique_ptr<llvm::LLVMContext> context,
                           std::shared_ptr<ShaderProgram> program)
    : fContext(std::move(context)),
      fModule(std::move(module)),
      fProgram(std::move(program))
{
}

void ShaderModule::moveToProgram()
{
    fProgram->addModule(std::move(fModule), std::move(fContext));
    fProgram = nullptr;
    fModule = nullptr;
    fContext = nullptr;
}

KOMOREBI_NS_END
