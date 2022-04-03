#ifndef COCOA_COBALT_LAPSCOMPILER_COMPILER_H
#define COCOA_COBALT_LAPSCOMPILER_COMPILER_H

#include "Cobalt/LapsCompiler/LapsCompiler.h"
LAPS_COMPILER_BEGIN_NS

class Module;

class Compiler
{
public:
    using OutputContent = std::string;
    using Artifact = std::tuple<OutputContent, co_sp<Module>>;

    static Artifact Compile(const std::string& source);
};

LAPS_COMPILER_END_NS
#endif //COCOA_COBALT_LAPSCOMPILER_COMPILER_H
