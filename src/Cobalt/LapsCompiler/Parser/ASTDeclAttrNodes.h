#ifndef COCOA_COBALT_LAPSCOMPILER_PARSER_ASTDECLATTRIBUTELITERALLIST_H
#define COCOA_COBALT_LAPSCOMPILER_PARSER_ASTDECLATTRIBUTELITERALLIST_H

#include <utility>

#include "Cobalt/LapsCompiler/Parser/ASTNodeBase.h"
#include "Cobalt/LapsCompiler/Parser/ASTArrayLikeNode.h"
LAPS_COMPILER_BEGIN_NS

class ASTDeclAttrSpecLiteralListNode : public ASTArrayLikeNode
{
public:
    ASTDeclAttrSpecLiteralListNode()
        : ASTArrayLikeNode(Type::kDeclAttrSpecLiteralList) {}
    ~ASTDeclAttrSpecLiteralListNode() override = default;
};

class ASTDeclAttrSpec : public ASTNodeBase
{
public:
    ASTDeclAttrSpec(std::string name, const co_sp<ASTNodeBase>& literalList)
        : ASTNodeBase(Type::kDeclAttrSpec)
        , attr_name_(std::move(name))
        , literal_list_(literalList->Cast<ASTDeclAttrSpecLiteralListNode>()) {}
    explicit ASTDeclAttrSpec(std::string name)
        : ASTNodeBase(Type::kDeclAttrSpec)
        , attr_name_(std::move(name)) {}
    ~ASTDeclAttrSpec() override = default;

    g_nodiscard g_inline co_sp<ASTDeclAttrSpecLiteralListNode> GetLiteralList() {
        return literal_list_;
    }

    g_nodiscard g_inline const std::string& GetSpecName() const {
        return attr_name_;
    }

private:
    std::string                 attr_name_;
    co_sp<ASTDeclAttrSpecLiteralListNode>
                                literal_list_;
};

class ASTDeclAttrSpecListNode : public ASTArrayLikeNode
{
public:
    ASTDeclAttrSpecListNode()
        : ASTArrayLikeNode(Type::kDeclAttrSpecList) {}
    ~ASTDeclAttrSpecListNode() override = default;
};

LAPS_COMPILER_END_NS
#endif //COCOA_COBALT_LAPSCOMPILER_PARSER_ASTDECLATTRIBUTELITERALLIST_H
