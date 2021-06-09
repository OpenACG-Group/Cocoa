#ifndef COCOA_ASTCODEBLOCKNODE_H
#define COCOA_ASTCODEBLOCKNODE_H

#include <map>
#include <string>

#include "Komorebi/Namespace.h"
#include "Komorebi/ASTBaseNode.h"
KOMOREBI_NS_BEGIN

class ASTCodeBlockNode : public ASTBaseNode
{
public:
    ASTCodeBlockNode();
    ~ASTCodeBlockNode() override;

    ASTValue getLValueInScope(const std::string& name);
    void appendLValueInScope(const std::string& name, const ASTValue& value);

private:
    ASTValue onCodeGen(llvm::IRBuilder<>& builder);

    std::map<std::string, ASTValue>     fScope;
};

KOMOREBI_NS_END
#endif //COCOA_ASTCODEBLOCKNODE_H
