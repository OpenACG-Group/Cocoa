%skeleton "lalr1.cc"
%require "3.7.2"

%define api.namespace { cocoa::prop }
%define api.parser.class { Parser }
%define api.token.constructor
%define api.value.type variant
%define parse.assert
%defines
%code requires
{
namespace cocoa::prop
{
class Scanner;
class ParserDriver;
}
}

%code top
{
#include "Scanner.h"
#include "Parser.hh"
#include "ParserDriver.h"
#include "location.hh"

static cocoa::prop::Parser::symbol_type yylex(Scanner& s, ParserDriver& d)
{
    return s.nextToken();
}
using namespace cocoa::prop;
}

%lex-param { cocoa::prop::Scanner& scanner }
%lex-param { cocoa::prop::ParserDriver& driver }
%parse-param { cocoa::prop::Scanner& scanner }
%parse-param { cocoa::prop::ParserDriver& driver }

%locations

%define parse.error verbose
%define api.token.prefix {TOKEN_}

%token <std::string> T_Comment
%token T_Semicolon
%token T_Comma
%token T_Period
%token T_True
%token T_False
%token T_Lp
%token T_Rp
%token T_Lb
%token T_Rb
%token T_Lt
%token T_Bt
%token T_New
%token T_Set
%token T_Relocate
%token T_Append
%token T_Insert
%token T_Int
%token T_Float
%token T_Bool
%token T_String
%token T_Object
%token T_Array
%token T_Data
%token T_Private
%token T_Protected
%token T_Public
%token <std::string> T_StrLiteral
%token <std::string> T_DecLiteral
%token <std::string> T_HexLiteral
%token <std::string> T_FltLiteral
%token <std::string> T_Ident
%left T_Comma

%start world

%%

world: statement_list ;

statement_list: statement
              | statement_list statement
              ;

