#ifndef COCOA_COBALT_LAPSCOMPILER_PARSER_ASTLITERALVALUENODE_H
#define COCOA_COBALT_LAPSCOMPILER_PARSER_ASTLITERALVALUENODE_H

#include <optional>

#include "Cobalt/LapsCompiler/LapsCompiler.h"
#include "Cobalt/LapsCompiler/Parser/ASTNodeBase.h"
LAPS_COMPILER_BEGIN_NS

class ASTLiteralValueNode : public ASTNodeBase
{
public:
    enum class LiteralType
    {
        kDecInteger,
        kHexInteger,
        kFloat,
        kString
    };

    template<typename T>
    using Maybe = std::optional<T>;

    ASTLiteralValueNode(const location& loc, LiteralType type, const std::string& token);
    ~ASTLiteralValueNode() override = default;

    g_nodiscard g_inline LiteralType GetLiteralType() const {
        return literal_type_;
    }

    g_nodiscard g_inline Maybe<int64_t> ToInt64Safe() const {
        return value_i64_;
    }
    g_nodiscard g_inline Maybe<uint64_t> ToUInt64Safe() const {
        return value_u64_;
    }
    g_nodiscard g_inline Maybe<float> ToFloatSafe() const {
        return value_float_;
    }

    g_nodiscard g_inline const std::string& GetStringValue() const {
        return literal_string_value_;
    }

private:
    LiteralType         literal_type_;
    std::string         literal_string_value_;
    Maybe<int64_t>      value_i64_;
    Maybe<uint64_t>     value_u64_;
    Maybe<float>        value_float_;
};

LAPS_COMPILER_END_NS
#endif //COCOA_COBALT_LAPSCOMPILER_PARSER_ASTLITERALVALUENODE_H
