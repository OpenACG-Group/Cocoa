#include <string>
#include <sstream>

#include "Cobalt/LapsCompiler/Parser/LapsParserDriver.h"
#include "Cobalt/LapsCompiler/Compiler.h"
LAPS_COMPILER_BEGIN_NS

Compiler::Artifact Compiler::Compile(const std::string& source)
{
    std::ostringstream parserOutputStream;
    std::istringstream parserInputStream(source);

    ParserDriver drv;
    if (!drv.Parse(parserInputStream, parserOutputStream))
        return { parserOutputStream.str(), nullptr };

    return { parserOutputStream.str(), nullptr };
}

LAPS_COMPILER_END_NS
