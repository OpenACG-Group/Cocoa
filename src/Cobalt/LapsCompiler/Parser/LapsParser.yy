%skeleton "lalr1.cc"
%require "3.8.1"

%define api.namespace { cocoa::cobalt::laps }
%define api.token.constructor
%define api.value.type variant
%define parse.assert
%define api.parser.class { LapsParser }
%defines

%code requires {
    #include <iostream>
    #include <string>
    #include "Cobalt/LapsCompiler/LapsCompiler.h"

    #include "ASTNodeBase.h"
    #include "ASTLiteralValueNode.h"
    #include "ASTDeclAttrNodes.h"
    #include "ASTDeclarationNodes.h"
    #include "ASTToplevelNodes.h"

    LAPS_COMPILER_BEGIN_NS
    class ParserDriver;
    class LapsScanner;
    LAPS_COMPILER_END_NS
}

%code top {
    #include "Cobalt/LapsCompiler/Parser/LapsScanner.h"
    #include "Cobalt/LapsCompiler/Parser/LapsParser.hh"
    #include "Cobalt/LapsCompiler/Parser/LapsParserDriver.h"
    #include "Cobalt/LapsCompiler/Parser/location.hh"

    LAPS_COMPILER_BEGIN_NS
    static LapsParser::symbol_type yylex(LapsScanner& scanner, ParserDriver& drv)
    {
        return scanner.nextToken(drv);
    }
    LAPS_COMPILER_END_NS
}

%lex-param { LapsScanner& scanner }
%lex-param { ParserDriver& drv }
%parse-param { LapsScanner& scanner }
%parse-param { ParserDriver& drv }

%locations

%define parse.trace
%define parse.error detailed
%define api.token.prefix {TOKEN_}

%token LBRACE           "["
       RBRACE           "]"
       LPAR             "("
       RPAR             ")"
       COMMA            ","
       SEMICOLON        ";"
       COLON            ":"
       ;

%token KEYWORD_LET      "let"

%token <std::string> LITERAL_DEC_INT
%token <std::string> LITERAL_HEX_INT
%token <std::string> LITERAL_FLOAT
%token <std::string> IDENTIFIER

%nterm <ASTNodeBase::Ptr> literal_value
%nterm <ASTNodeBase::Ptr> decl_attribute_literal_list
%nterm <ASTNodeBase::Ptr> decl_attribute_spec
%nterm <ASTNodeBase::Ptr> decl_attributes_spec_list
%nterm <ASTNodeBase::Ptr> decl_attributes
%nterm <ASTNodeBase::Ptr> let_declaration
%nterm <ASTNodeBase::Ptr> statement
%nterm <ASTNodeBase::Ptr> statement_list
%nterm <ASTNodeBase::Ptr> translation_unit

%start translation_unit

%%

translation_unit: statement_list YYEOF
                {
                    $$ = ASTNodeBase::New<ASTTranslationUnitNode>($1);
                    drv.SetTranslationUnit($$->Cast<ASTTranslationUnitNode>());
                }
                ;

statement_list: statement
                {
                    $$ = ASTNodeBase::New<ASTStatementListNode>()
                         ->AppendChild($1);
                }
              | statement_list statement
                {
                    $$ = $1->Cast<ASTStatementListNode>()
                         ->AppendChild($2);
                }
              ;

statement: let_declaration ";"                      { $$ = $1; }
         ;

literal_value: LITERAL_DEC_INT
                {
                    $$ = ASTNodeBase::New<ASTLiteralValueNode>(@1,
                        ASTLiteralValueNode::LiteralType::kDecInteger, $1);
                    @$ = @1;
                }
             | LITERAL_HEX_INT
                {
                    $$ = ASTNodeBase::New<ASTLiteralValueNode>(@1,
                        ASTLiteralValueNode::LiteralType::kHexInteger, $1);
                    @$ = @1;
                }
             | LITERAL_FLOAT
                {
                    $$ = ASTNodeBase::New<ASTLiteralValueNode>(@1,
                        ASTLiteralValueNode::LiteralType::kFloat, $1);
                    @$ = @1;
                }
             ;

let_declaration: "let" IDENTIFIER ":" IDENTIFIER
                    {
                        auto t = ASTNodeBase::New<ASTTypeNameExprNode>(@4, $4);
                        $$ = ASTNodeBase::New<ASTLetDeclarationStmtNode>(nullptr, t, $2);
                    }
               | decl_attributes "let" IDENTIFIER ":" IDENTIFIER
                    {
                        auto t = ASTNodeBase::New<ASTTypeNameExprNode>(@5, $5);
                        $$ = ASTNodeBase::New<ASTLetDeclarationStmtNode>($1, t, $3);
                    }
               ;

decl_attributes: "[" "[" decl_attributes_spec_list "]" "]"
                    { $$ = $3; }
               ;

decl_attributes_spec_list: decl_attribute_spec
                            {
                                $$ = ASTNodeBase::New<ASTDeclAttrSpecListNode>()
                                     ->AppendChild($1);
                            }
                         | decl_attributes_spec_list "," decl_attribute_spec
                            {
                                $$ = $1->Cast<ASTDeclAttrSpecListNode>()
                                     ->AppendChild($3);
                            }
                         ;

decl_attribute_spec: IDENTIFIER "(" decl_attribute_literal_list ")"
                        {
                            $$ = ASTNodeBase::New<ASTDeclAttrSpec>($1, $3);
                        }
                   | IDENTIFIER
                        {
                            $$ = ASTNodeBase::New<ASTDeclAttrSpec>($1);
                        }
                   ;

decl_attribute_literal_list: literal_value
                                {
                                    $$ = ASTNodeBase::New<ASTDeclAttrSpecLiteralListNode>()
                                         ->AppendChild($1);
                                }
                           | decl_attribute_literal_list "," literal_value
                                {
                                    $$ = $1->Cast<ASTDeclAttrSpecLiteralListNode>()
                                         ->AppendChild($3);
                                }
                           ;

%%
