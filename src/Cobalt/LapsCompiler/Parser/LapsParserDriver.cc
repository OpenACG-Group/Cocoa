#include <iostream>
#include "fmt/format.h"

#include "Cobalt/LapsCompiler/LapsCompiler.h"
#include "Cobalt/LapsCompiler/Parser/LapsParserDriver.h"
#include "Cobalt/LapsCompiler/Parser/LapsScanner.h"
#include "Cobalt/LapsCompiler/Parser/LapsParser.hh"
#include "Cobalt/LapsCompiler/Parser/location.hh"
LAPS_COMPILER_BEGIN_NS

namespace {

#define E(x) { ASTNodeBase::Type::k##x, #x }
struct {
    ASTNodeBase::Type tp;
    const char *name;
} g_ast_node_type_name[] = {
    E(TranslationUnit),
    E(StatementList),
    E(LiteralValue),
    E(TypeNameExpr),
    E(LetDeclarationStmt),
    E(DeclAttrSpecList),
    E(DeclAttrSpec),
    E(DeclAttrSpecLiteralList)
};
#undef E

void print_ast_node_rec(const co_sp<ASTNodeBase>& node, int32_t depth)
{
    for (int32_t i = 0; i < 0 * 2; i++)
        fmt::print(" ");
    for (const auto& pair : g_ast_node_type_name)
    {
        if (pair.tp == node->GetNodeType())
        {
            fmt::print("{}", pair.name);
            break;
        }
    }
    fmt::print("\n");

    node->ForeachChildNode([depth](const co_sp<ASTNodeBase>& child) {
        print_ast_node_rec(child, depth + 1);
    });
}

void print_ast_node(const co_sp<ASTNodeBase>& node)
{
    print_ast_node_rec(node, 0);
}

}

ParserDriver::ParserDriver()
    : location_(std::make_unique<location>())
    , scanner_(std::make_unique<LapsScanner>(*this))
    , parser_(std::make_unique<LapsParser>(*scanner_, *this))
{
}

ParserDriver::~ParserDriver() = default;

bool ParserDriver::Parse(std::istream& is, std::ostream& os)
{
    scanner_->switch_streams(is, os);

    try {
        return (parser_->parse() == 0);
    } catch (const LapsParser::syntax_error& e) {
        position begin = e.location.begin, end = e.location.end;
        os << fmt::format("SyntaxError: [{}:{}, {}:{}] {}",
                          begin.line, begin.column, end.line, end.column, e.what());
        return false;
    }
}

void LapsParser::error(const location_type& loc, const std::string& msg)
{
    throw LapsParser::syntax_error(loc, msg);
}

void ParserDriver::SetTranslationUnit(const co_sp<ASTTranslationUnitNode>& TU)
{
    translation_unit_ = TU;
    print_ast_node(TU);
}

LAPS_COMPILER_END_NS
