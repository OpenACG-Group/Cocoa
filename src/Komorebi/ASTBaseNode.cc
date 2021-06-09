#include "Komorebi/Namespace.h"
#include "Komorebi/ASTBaseNode.h"
#include "Komorebi/ASTCodeBlockNode.h"
KOMOREBI_NS_BEGIN

ASTBaseNode::ASTBaseNode(Kind kind, const Ptr& parent)
    : fNodeKind(kind), fParent(parent)
{
}

void ASTBaseNode::appendChild(const Ptr& child)
{
    fChildren.push_back(child);
    child->setParent(shared_from_this());
}

void ASTBaseNode::setParent(const Ptr& parent)
{
    fParent = parent;
}

ASTBaseNode::WeakPtr ASTBaseNode::findNearestCodeBlock()
{
    if (fNodeKind == Kind::kCodeBlockNode)
    {
        return shared_from_this();
    }
    else if (fParent.lock() != nullptr)
    {
        return fParent.lock()->findNearestCodeBlock();
    }
    return Ptr(nullptr);
}

ASTValue ASTBaseNode::findVarInScope(const std::string& name)
{
    // TODO: Rewrite this

    WeakPtr scope = findNearestCodeBlock();
    if (scope.lock() == nullptr)
        return ASTValue();
    else
    {
        // TODO: Wrong!!!
        auto scopeNode = scope.lock();
        auto *ptr = dynamic_cast<ASTCodeBlockNode*>(scopeNode.get());
        return ptr->getLValueInScope(name);
    }
}

void ASTBaseNode::defineVarInScope(const std::string& name, ASTValue value)
{
    // TODO: Implement this
}

ASTValue ASTBaseNode::codeGen(llvm::IRBuilder<>& emitter)
{
    return this->onCodeGen(emitter);
}

KOMOREBI_NS_END
