#ifndef COCOA_COBALT_LAPSCOMPILER_PARSER_LAPSPARSERDRIVER_H
#define COCOA_COBALT_LAPSCOMPILER_PARSER_LAPSPARSERDRIVER_H

#include <string>
#include <istream>
#include <ostream>

#include "Cobalt/LapsCompiler/LapsCompiler.h"
LAPS_COMPILER_BEGIN_NS

class LapsParser;
class LapsScanner;
class location;
class ASTTranslationUnitNode;

class ParserDriver
{
public:
    ParserDriver();
    ~ParserDriver();

    bool Parse(std::istream& is, std::ostream& os);

    g_nodiscard g_inline location& GetLocation() {
        return *location_;
    }

    g_nodiscard g_inline co_sp<ASTTranslationUnitNode> GetTranslationUnit() {
        return translation_unit_;
    }

    void SetTranslationUnit(const co_sp<ASTTranslationUnitNode>& TU);

private:
    co_unique<location>             location_;
    co_unique<LapsScanner>          scanner_;
    co_unique<LapsParser>           parser_;
    co_sp<ASTTranslationUnitNode>   translation_unit_;
};

LAPS_COMPILER_END_NS
#endif //COCOA_COBALT_LAPSCOMPILER_PARSER_LAPSPARSERDRIVER_H
