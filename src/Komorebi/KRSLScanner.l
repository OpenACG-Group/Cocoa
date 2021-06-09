%{

#include <iostream>
#include <cerrno>
#include <string>

#include "Komorebi/KRSLParser.hh"
#include "Komorebi/KRSLScanner.h"
#include "Komorebi/location.hh"

static cocoa::komorebi::location loc;
#define YY_USER_ACTION  loc.columns(yyleng);

#undef yywrap
#define yywrap() 1

using namespace cocoa::komorebi;
#define yyterminate() Parser::make_T_EOF(loc);

#define KMR_RETLEX(type, ...) \
    return Parser::make_T_##type(__VA_ARGS__);

%}

%option c++
%option noyywrap debug
%option yyclass="KRSLScanner"
%option prefix="cocoa_krsl_"

VOID                    (void)
SCALAR                  (scalar)
VEC2                    (vec2)
VEC3                    (vec3)
VEC4                    (vec4)
MAT2                    (mat2)
MAT3                    (mat3)
MAT4                    (mat4)
PATH_T                  (Path)
PAINT_T                 (Paint)
SHADER_T                (Shader)
IMAGE_T                 (Image)
PATH_EFFECT_T           (PathEffect)
COLOR_FILTER_T          (ColorFilter)
CONST                   (const)
UNIFORM                 (uniform)
EXPORT                  (export)
DECLARE                 (declare)
EMIT                    (emit)
STROKE                  (stroke)
FILL                    (fill)

ASSIGN                  =
OP_ADD                  \+
OP_SUB                  -
OP_MUL                  \*
OP_DIV                  \/
OP_MOD                  %
BIT_AND                 &
BIT_OR                  \|
BIT_XOR                 \^
BIT_NOT                 ~
BIT_RSHIFT              >>
BIT_LSHIFT              <<
REL_BIGGER_THAN_EQ      >=
REL_LESS_THAN_EQ        <=
REL_EQUAL               ==
REL_NOT_EQUAL           \!=
REL_BIGGER_THAN         >
REL_LESS_THAN           <
LOGICAL_AND             &&
LOGICAL_OR              \|\|
LOGICAL_NOT             \!
ARROW                   =>

DOT                     \.
COMMA                   ,
SEMICOLON               ;
COLON                   :
LEFT_PARENTHESES        \(
RIGHT_PARENTHESES       \)
LEFT_BRACKET            \[
RIGHT_BRACKET           \]
LEFT_BRACE              \{
RIGHT_BRACE             \}

PREPROCESSOR            #\[\[.+\]\]
IDENT                   [a-zA-Z][a-zA-Z0-9]*
NEWLINE                 \n+
WHITE_SPACE             [ \t]+

%%

%{
    loc.step();    
%}

{VOID}
{SCALAR}
{VEC2}
{VEC3}
{VEC4}
{MAT2}
{MAT3}
{MAT4}
{CONST}
{UNIFORM}
{EXPORT}
{DECLARE}

{ASSIGN}
{OP_ADD}
{OP_SUB}
{OP_MUL}
{OP_DIV}
{OP_MOD}
{BIT_AND}
{BIT_OR}
{BIT_XOR}
{BIT_NOT}
{BIT_RSHIFT}
{BIT_LSHIFT}
{REL_BIGGER_THAN_EQ}
{REL_LESS_THAN_EQ}
{REL_EQUAL}
{REL_NOT_EQUAL}
{REL_BIGGER_THAN}
{REL_LESS_THAN}
{LOGICAL_AND}
{LOGICAL_OR}
{LOGICAL_NOT}

{HASH}
{DOT}
{COMMA}
{SEMICOLON}
{LEFT_PARENTHESES}
{RIGHT_PARENTHESES}
{LEFT_BRACKET}
{RIGHT_BRACKET}
{LEFT_BRACE}
{RIGHT_BRACE}
{LEFT_D_BRACKET}
{RIGHT_D_BRACKET}
{LEFT_D_BRACE}
{RIGHT_D_BRACE}

{IDENT}                 { KMR_RETLEX(Identifier, yytext, loc) }
{NEWLINE}               { loc.lines(yyleng); loc.step(); }
{WHITE_SPACE}           { loc.step(); }
<<EOF>>                 { KMR_RETLEX(EOF, loc) }

%%
