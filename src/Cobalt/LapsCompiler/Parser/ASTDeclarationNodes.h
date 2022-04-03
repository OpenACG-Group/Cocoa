#ifndef COCOA_COBALT_LAPSCOMPILER_PARSER_ASTDECLARATIONNODES_H
#define COCOA_COBALT_LAPSCOMPILER_PARSER_ASTDECLARATIONNODES_H

#include "Cobalt/LapsCompiler/Parser/ASTNodeBase.h"
LAPS_COMPILER_BEGIN_NS

class ASTDeclAttrSpecListNode;

class ASTTypeNameExprNode : public ASTNodeBase
{
public:
    ASTTypeNameExprNode(const location& loc, std::string name)
        : ASTNodeBase(Type::kTypeNameExpr, loc), typename_(std::move(name)) {}
    ~ASTTypeNameExprNode() override = default;

    g_nodiscard g_inline const std::string& GetTypeName() const {
        return typename_;
    }

private:
    std::string     typename_;
};

class ASTLetDeclarationStmtNode : public ASTNodeBase
{
public:
    AST_DECL_CHILDID(DeclAttrSpecList, 1)
    AST_DECL_CHILDID(TypeNameExpr, 2)

    ASTLetDeclarationStmtNode(const co_sp<ASTNodeBase>& declAttrNode,
                              const co_sp<ASTNodeBase>& typeNameExprNode,
                              std::string name)
        : ASTNodeBase(Type::kLetDeclarationStmt)
        , declaration_name_(std::move(name))
    {
        if (declAttrNode)
        {
            AST_ASSERT_TYPE(declAttrNode, DeclAttrSpecList);
            SetChildNode(kChildDeclAttrSpecList, declAttrNode);
        }
        if (typeNameExprNode)
        {
            AST_ASSERT_TYPE(typeNameExprNode, TypeNameExpr);
            SetChildNode(kChildTypeNameExpr, typeNameExprNode);
        }
    }
    ~ASTLetDeclarationStmtNode() override = default;

    g_nodiscard g_inline const std::string& GetDeclarationName() const {
        return declaration_name_;
    }

private:
    std::string         declaration_name_;
};

LAPS_COMPILER_END_NS
#endif //COCOA_COBALT_LAPSCOMPILER_PARSER_ASTDECLARATIONNODES_H
