#ifndef COCOA_COBALT_LAPSCOMPILER_PARSER_ASTTOPLEVELNODES_H
#define COCOA_COBALT_LAPSCOMPILER_PARSER_ASTTOPLEVELNODES_H

#include "Cobalt/LapsCompiler/Parser/ASTNodeBase.h"
#include "Cobalt/LapsCompiler/Parser/ASTArrayLikeNode.h"
LAPS_COMPILER_BEGIN_NS

class ASTStatementListNode : public ASTArrayLikeNode
{
public:
    ASTStatementListNode()
        : ASTArrayLikeNode(Type::kStatementList) {}
    ~ASTStatementListNode() override = default;
};

class ASTTranslationUnitNode : public ASTNodeBase
{
public:
    AST_DECL_CHILDID(StatementList, 1);

    explicit ASTTranslationUnitNode(const co_sp<ASTNodeBase>& statementList)
        : ASTNodeBase(Type::kTranslationUnit)
    {
        AST_ASSERT_TYPE(statementList, StatementList);
        SetChildNode(kChildStatementList, statementList);
    }
    ~ASTTranslationUnitNode() override = default;
};

LAPS_COMPILER_END_NS
#endif //COCOA_COBALT_LAPSCOMPILER_PARSER_ASTTOPLEVELNODES_H
