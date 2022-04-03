#include "fmt/format.h"

#include "Core/Errors.h"
#include "Cobalt/LapsCompiler/Parser/ASTNodeBase.h"
#include "Cobalt/LapsCompiler/Parser/location.hh"
LAPS_COMPILER_BEGIN_NS

ASTNodeBase::ASTNodeBase(Type type, const location& loc)
    : node_type_(type)
    , location_info_(loc)
{
}

ASTNodeBase::ASTNodeBase(Type type)
    : node_type_(type)
    , location_info_()
{
}

co_sp<ASTNodeBase> ASTNodeBase::GetChildNode(ChildNodeID id)
{
    if (child_nodes_.count(id) > 0)
        return child_nodes_[id];
    return nullptr;
}

void ASTNodeBase::SetChildNode(ChildNodeID id, const co_sp<ASTNodeBase>& child)
{
    CHECK(child != nullptr && "Nullptr child node");
    child_nodes_[id] = child;
    if (this->Self())
        child->parent_node_ = this->Self();
    else
        deferred_set_parent_nodes_.emplace_back(child);
}

void ASTNodeBase::AcceptDeferredSetParentNodes()
{
    for (const auto& child : deferred_set_parent_nodes_)
        child->parent_node_ = this->Self();
    deferred_set_parent_nodes_.clear();
}

ASTNodeBase::Location::Location(const location& loc)
    : start_line(loc.begin.line)
    , start_column(loc.begin.column)
    , end_line(loc.end.line)
    , end_column(loc.end.column)
{
}

std::string ASTNodeBase::Location::ToString() const
{
    return fmt::format("[{}:{},{}:{}]", start_line, start_column, end_line, end_column);
}

LAPS_COMPILER_END_NS
