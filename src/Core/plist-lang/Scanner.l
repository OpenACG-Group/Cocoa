%{
#include "Parser.hh"
#include "Scanner.h"
#include "location.hh"

static cocoa::prop::location loc;
#define YY_USER_ACTION loc.columns(yyleng);

#undef yywrap
#define yywrap() 1

using namespace cocoa::prop;
#define yyterminate() Parser::make_T_EOF(loc);
%}

%option c++
%option noyywrap debug
%option yyclass="Scanner"
%option prefix="cocoa_prop_"

COMMENT         ^#.*$
SEMICOLON       ;
COMMA           ,
PERIOD          \.
TRUE            true
FALSE           false
LP              \(
RP              \)
LB              \[
RB              \]
LT              <
BT              >
V_NEW           new
V_SET           set
V_RELOCATE      relocate
V_APPEND        append
V_INSERT        insert
T_INT           int
T_FLOAT         float
T_BOOL          bool
T_STRING        string
T_OBJECT        object
T_ARRAY         array
T_DATA          data
Q_PRIVATE       private
Q_PROTECTED     protected
Q_PUBLIC        public
L_STRING        \"(\\.|[^"\\])*\"
L_DECIMAL       [0-9]+
L_HEX           0x[A-Fa-f0-9]+
L_FLOAT         [0-9]*\.[0-9]+
IDENT           [a-zA-Z][a-zA-Z0-9]*
NEWLINE         \n+
WHITESPACE      [ \t]+

%%

%{
    loc.step();
%}

{COMMENT}       { Parser::make_T_Comment(yytext, loc); }
{SEMICOLON}     { Parser::make_T_Semicolon(loc); }
{COMMA}         { Parser::make_T_Comma(loc); }
{PERIOD}        { Parser::make_T_Period(loc); }
{TRUE}          { Parser::make_T_True(loc); }
{FALSE}         { Parser::make_T_False(loc); }
{LP}            { Parser::make_T_Lp(loc); }
{RP}            { Parser::make_T_Rp(loc); }
{LB}            { Parser::make_T_Lb(loc); }
{RB}            { Parser::make_T_Rb(loc); }
{LT}            { Parser::make_T_Lt(loc); }
{BT}            { Parser::make_T_Bt(loc); }
{V_NEW}         { Parser::make_T_New(loc); }
{V_SET}         { Parser::make_T_Set(loc); }
{V_RELOCATE}    { Parser::make_T_Relocate(loc); }
{V_APPEND}      { Parser::make_T_Append(loc); }
{V_INSERT}      { Parser::make_T_Insert(loc); }
{T_INT}         { Parser::make_T_Int(loc); }
{T_FLOAT}       { Parser::make_T_Float(loc); }
{T_BOOL}        { Parser::make_T_Bool(loc); }
{T_STRING}      { Parser::make_T_String(loc); }
{T_OBJECT}      { Parser::make_T_Object(loc); }
{T_ARRAY}       { Parser::make_T_Array(loc); }
{T_DATA}        { Parser::make_T_Data(loc); }
{Q_PRIVATE}     { Parser::make_T_Private(loc); }
{Q_PROTECTED}   { Parser::make_T_Protected(loc); }
{Q_PUBLIC}      { Parser::make_T_Public(loc); }
{L_STRING}      { Parser::make_T_StrLiteral(yytext, loc); }
{L_DECIMAL}     { Parser::make_T_DecLiteral(yytext, loc); }
{L_HEX}         { Parser::make_T_HexLiteral(yytext, loc); }
{L_FLOAT}       { Parser::make_T_FltLiteral(yytext, loc); }
{IDENT}         { Parser::make_T_Ident(yytext, loc); }
{NEWLINE}       { loc.lines(yyleng); loc.step(); }
{WHITESPACE}    { loc.step(); }
<<EOF>>         { return Parser::make_T_EOF(loc); }

%%
