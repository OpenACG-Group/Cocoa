#ifndef COCOA_ASTBASENODE_H
#define COCOA_ASTBASENODE_H

#include <memory>
#include <vector>

#include "llvm/IR/IRBuilder.h"
#include "Komorebi/Namespace.h"
#include "Komorebi/ASTValue.h"
KOMOREBI_NS_BEGIN

class ASTBaseNode : std::enable_shared_from_this<ASTBaseNode>
{
public:
    using Ptr = std::shared_ptr<ASTBaseNode>;
    using WeakPtr = std::weak_ptr<ASTBaseNode>;

    enum Kind
    {
        kCompoundStmt,
        kDeclStmt,
        kFunctionDecl,
        kVarDecl,
        kParamVarDecl,
        kBinaryOperator,
        kUnaryOperator,
        kScalarLiteral,
        kDeclRefExpr,
        kCallExpr,
        kConstructExpr,
        kImplicitCastExpr
    };

    explicit ASTBaseNode(Kind kind, const Ptr& parent = nullptr);
    virtual ~ASTBaseNode() = default;

    inline Kind nodeKind() const
    { return fNodeKind; }

    inline std::vector<Ptr>& children()
    { return fChildren; }

    inline WeakPtr parent()
    { return fParent; }

    ASTValue codeGen(llvm::IRBuilder<>& emitter);

    WeakPtr findNearestCodeBlock();
    ASTValue findVarInScope(const std::string& name);
    void defineVarInScope(const std::string& name, ASTValue value);

protected:
    void setParent(const Ptr& parent);
    void appendChild(const Ptr& child);
    virtual ASTValue onCodeGen(llvm::IRBuilder<>& emitter) = 0;

private:
    Kind                    fNodeKind;
    std::vector<Ptr>        fChildren;
    WeakPtr                 fParent;
};

KOMOREBI_NS_END
#endif //COCOA_ASTBASENODE_H
