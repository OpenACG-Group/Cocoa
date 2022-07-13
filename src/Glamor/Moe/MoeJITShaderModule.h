#ifndef COCOA_MOE_MOEJITSHADERMODULE_H
#define COCOA_MOE_MOEJITSHADERMODULE_H

#include "Glamor/Glamor.h"
class SkCanvas;

GLAMOR_NAMESPACE_BEGIN

class MoeJITShaderModule
{
public:
    using EntrypointFunc = void(*)(SkCanvas*);

    static Shared<MoeJITShaderModule> Compile();

    MoeJITShaderModule(EntrypointFunc pfn, asmjit::String&& disassembled);
    ~MoeJITShaderModule();

private:
    EntrypointFunc              entrypoint_pfn_;
    asmjit::String              disassembled_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_MOE_MOEJITSHADERMODULE_H
