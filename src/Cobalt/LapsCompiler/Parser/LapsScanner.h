#ifndef COCOA_COBALT_LAPSCOMPILER_PARSER_LAPSSCANNER_H
#define COCOA_COBALT_LAPSCOMPILER_PARSER_LAPSSCANNER_H

#if !defined(yyFlexLexerOnce)
#undef yyFlexLexer
#define yyFlexLexer cocoa_laps_FlexLexer
#include <FlexLexer.h>
#endif

#undef YY_DECL
#define YY_DECL \
    LapsParser::symbol_type LapsScanner::nextToken(ParserDriver& drv)

#include "Cobalt/LapsCompiler/LapsCompiler.h"
#include "Cobalt/LapsCompiler/Parser/LapsParser.hh"
LAPS_COMPILER_BEGIN_NS

class ParserDriver;

class LapsScanner : public yyFlexLexer
{
public:
    explicit LapsScanner(ParserDriver& drv) {}
    ~LapsScanner() override = default;

    LapsParser::symbol_type nextToken(ParserDriver& drv);
};

LAPS_COMPILER_END_NS
#endif //COCOA_COBALT_LAPSCOMPILER_PARSER_LAPSSCANNER_H
