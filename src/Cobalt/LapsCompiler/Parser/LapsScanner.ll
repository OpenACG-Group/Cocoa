%{
#include <string>
#include "Cobalt/LapsCompiler/Parser/LapsParserDriver.h"
#include "Cobalt/LapsCompiler/Parser/LapsParser.hh"
#include "Cobalt/LapsCompiler/Parser/location.hh"
#include "Cobalt/LapsCompiler/Parser/LapsScanner.h"

#undef yywrap
#define yywrap() 1

using namespace cocoa::cobalt::laps;

#define GT(token)   return LapsParser::make_##token(loc)
#define GT2(token)  return LapsParser::make_##token(yytext, loc)
%}

%option c++
%option noyywrap debug
%option yyclass="LapsScanner"
%option prefix="cocoa_laps_"

IDENT                       [a-zA-Z][a-zA-Z_0-9]*
INTEGER_POSTFIX             U?L?L?
DEC_INTEGER                 [0-9]+{INTEGER_POSTFIX}
HEX_INTEGER                 0x[a-fA-F0-9]+{INTEGER_POSTFIX}
FLOAT                       [0-9]*\.[0-9]+f?

WHITESPACE                  [ \t\r]

%%

%{
    location& loc = drv.GetLocation();
    loc.step();
%}

{WHITESPACE}+               loc.step();
\n+                         loc.lines(yyleng); loc.step();

\[                          GT(LBRACE);
\]                          GT(RBRACE);
\(                          GT(LPAR);
\)                          GT(RPAR);
,                           GT(COMMA);
;                           GT(SEMICOLON);
:                           GT(COLON);
let                         GT(KEYWORD_LET);
{DEC_INTEGER}               GT2(LITERAL_DEC_INT);
{HEX_INTEGER}               GT2(LITERAL_HEX_INT);
{FLOAT}                     GT2(LITERAL_FLOAT);
{IDENT}                     GT2(IDENTIFIER);

<<EOF>>                     return LapsParser::make_YYEOF(loc);
.                           { throw LapsParser::syntax_error
                                (loc, "Unrecognized character: " + std::string(yytext)); }

%%
