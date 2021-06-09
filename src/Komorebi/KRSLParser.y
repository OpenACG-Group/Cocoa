%skeleton "lalr1.cc"
%require "3.7.2"

%define api.namespace { cocoa::komorebi }
%define api.parser.class { Parser }
%define api.token.constructor
%define api.value.type variant
// %define parse.assert
%defines
%code requires
{
    #include <iostream>
    #include <string>
    #include "Komorebi/Namespace.h"

    KOMOREBI_NS_BEGIN
    
    class KRSLScanner;
    class KRSLParserDriver;
    
    KOMOREBI_NS_END
}

%code top
{
    #include <iostream>
    #include "Komorebi/Namespace.h"
    #include "Komorebi/KRSLScanner.h"
    #include "Komorebi/KRSLParser.hh"
    #include "Komorebi/KRSLParserDriver.h"
    #include "Komorebi/location.hh"

    static cocoa::komorebi::Parser::symbol_type yylex(
        cocoa::komorebi::KRSLScanner& scanner,
        cocoa::komorebi::KRSLParserDriver &driver)
    {
        return scanner.nextToken();
    }
    using namespace cocoa::komorebi;
}

%lex-param { cocoa::komorebi::KRSLScanner& scanner }
%lex-param { cocoa::komorebi::KRSLParserDriver& driver }

%parse-param { cocoa::komorebi::KRSLScanner& scanner }
%parse-param { cocoa::komorebi::KRSLParserDriver& driver }

%locations
//%define parse-trace

%define parse.error verbose
%define api.token.prefix {TOKEN_}

%token T_EOF 0

/* Basic types */
%token T_Void           // void
%token T_Scalar         // scalar
%token T_Vec2           // vec2
%token T_Vec3           // vec3
%token T_Vec4           // vec4
%token T_Mat2           // mat2
%token T_Mat3           // mat3
%token T_Mat4           // mat4
%token T_PathType       // Path
%token T_PaintType      // Paint
%token T_ShaderType     // Shader
%token T_ImageType      // Image
%token T_PathEffectType // PathEffect
%token T_CrFilterType   // ColorFilter

/* Keywords and symbol constraints */
%token T_Const          // const
%token T_Uniform        // uniform
%token T_Export         // export
%token T_Declare        // declare
%token T_Stroke         // stroke
%token T_Emit           // emit
%token T_Fill           // fill

/* Operators */
%token T_Assign         // =
%token T_Add            // +
%token T_Sub            // -
%token T_Mul            // *
%token T_Div            // /
%token T_Mod            // %
%token T_BitAnd         // &
%token T_BitOr          // |
%token T_BitXor         // ^
%token T_BitNot         // ~
%token T_BitRShift      // >>
%token T_BitLShift      // <<
%token T_BiggerEq       // >=
%token T_LessEq         // <=
%token T_Equal          // ==
%token T_NotEqual       // !=
%token T_BiggerThan     // >
%token T_LessThan       // <
%token T_LogicalAnd     // &&
%token T_LogicalOr      // ||
%token T_LogicalNot     // !
%token T_Arrow          // =>

/* Others */
%token T_Dot            // .
%token T_Comma          // ,
%token T_Semicolon      // ;
%token T_Colon          // :
%token T_Lp             // (
%token T_Rp             // )
%token T_Lbt            // ]
%token T_Rbt            // [
%token T_Lbr            // {
%token T_Rbr            // {

%token <std::string> T_Preprocessor     // #[[...]]
%token <std::string> T_Identifier       // Identifier

/* Operator associativity */
%right T_Assign
%left T_Add
%left T_Sub
%left T_Mul
%left T_Div
%left T_Mod
%left T_BitAnd
%left T_BitOr
%left T_BitXor
%right T_BitNot
%left T_BitRShift
%left T_BitLShift
%left T_BiggerEq
%left T_LessEq
%left T_Equal
%left T_NotEqual
%left T_BiggerThan
%left T_LessThan
%left T_LogicalAnd
%left T_LogicalOr
%right T_LogicalNot
%left T_Comma
%left T_Dot

%start program

%%

program: statement_list { std::cout << "StmtList" << std::endl; }
       ;

statement_list: statement { std::cout << "Stmt" << std::endl; }
              | statement_list statement
              ;

statement: ident_list   { std::cout << "IdentList" << std::endl; }
         ;

ident_list: T_Identifier { std::cout << "Ident[" << $1 << "]" << std::endl; }
          | ident_list T_Comma T_Identifier { std::cout << "Ident[" << $3 << "]" << std::endl; }
          ;

%%

void Parser::error(const location& location, const std::string& message)
{
    std::cout << "At " << location << ": " << message << std::endl;
}
