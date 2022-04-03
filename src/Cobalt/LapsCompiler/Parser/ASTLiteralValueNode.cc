#include "fmt/format.h"

#include "Cobalt/LapsCompiler/Parser/ASTLiteralValueNode.h"
LAPS_COMPILER_BEGIN_NS

ASTLiteralValueNode::ASTLiteralValueNode(const location& loc,
                                         LiteralType type,
                                         const std::string& token)
    : ASTNodeBase(Type::kLiteralValue, loc)
    , literal_type_(type)
    , literal_string_value_(token)
{
    fmt::print("literal \"{}\"\n", token);
}

LAPS_COMPILER_END_NS
