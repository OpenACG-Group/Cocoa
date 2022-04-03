#ifndef COCOA_COBALT_LAPSCOMPILER_PARSER_ASTARRAYLIKENODE_H
#define COCOA_COBALT_LAPSCOMPILER_PARSER_ASTARRAYLIKENODE_H

#include "Cobalt/LapsCompiler/Parser/ASTNodeBase.h"
LAPS_COMPILER_BEGIN_NS

class ASTArrayLikeNode : public ASTNodeBase
{
public:
    ASTArrayLikeNode(Type type, const location& loc)
        : ASTNodeBase(type, loc), child_counter_(0) {}
    explicit ASTArrayLikeNode(Type type)
        : ASTNodeBase(type), child_counter_(0) {}
    ~ASTArrayLikeNode() override = default;

    g_nodiscard g_inline co_sp<ASTNodeBase> AppendChild(const co_sp<ASTNodeBase>& node) {
        SetChildNode(child_counter_, node);
        child_counter_++;
        return ASTNodeBase::Self();
    }

    g_nodiscard g_inline ChildNodeID GetChildNumber() const {
        return child_counter_;
    }

private:
    ChildNodeID    child_counter_;
};

LAPS_COMPILER_END_NS
#endif //COCOA_COBALT_LAPSCOMPILER_PARSER_ASTARRAYLIKENODE_H
