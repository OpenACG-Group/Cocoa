#ifndef COCOA_KRSLSCANNER_H
#define COCOA_KRSLSCANNER_H

#if !defined(yyFlexLexerOnce)
#undef yyFlexLexer
#define yyFlexLexer cocoa_krsl_FlexLexer
#include <FlexLexer.h>
#endif

#undef YY_DECL
#define YY_DECL \
    Parser::symbol_type KRSLScanner::nextToken()

#include <string>
#include "Komorebi/Namespace.h"
#include "Komorebi/KRSLParser.hh"

KOMOREBI_NS_BEGIN
class KRSLParserDriver;

class KRSLScanner : public yyFlexLexer
{
public:
    explicit KRSLScanner(KRSLParserDriver& driver)
                : fDriver(driver) {}
    ~KRSLScanner() override = default;

    Parser::symbol_type nextToken();

private:
    [[maybe_unused]] KRSLParserDriver&   fDriver;
};

KOMOREBI_NS_END
#endif //COCOA_KRSLSCANNER_H
