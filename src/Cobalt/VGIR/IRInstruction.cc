#include "Cobalt/VGIR/IRInstruction.h"
COBALT_NAMESPACE_BEGIN
namespace ir {

namespace {

struct
{
    Opcode opcode;
    const char *name;
} g_opcode_name_map[] = {
    { Opcode::kPush, "push" },
    { Opcode::kPop, "pop" },
    { Opcode::kRedundant, "redundant" },
    { Opcode::kExchange, "exchange" },
    { Opcode::kLoadp, "loadp" },
    { Opcode::kAdd, "add" },
    { Opcode::kSub, "sub" },
    { Opcode::kMul, "mul" },
    { Opcode::kDiv, "div" },
    { Opcode::kRem, "rem" },
    { Opcode::kInc, "inc" },
    { Opcode::kNeg, "neg" },
    { Opcode::kSqrt, "sqrt" },
    { Opcode::kRsqrt, "rsqrt" },
    { Opcode::kFastrsqrt, "fastrsqrt" },
    { Opcode::kPow2, "pow2" },
    { Opcode::kSin, "sin" },
    { Opcode::kCos, "cos" },
    { Opcode::kTan, "tan" },
    { Opcode::kSincos, "sincos" },
    { Opcode::kSincosb, "sincosb" },
    { Opcode::kReducevb, "reducevb" },
    { Opcode::kReducevt, "reducevt" },
    { Opcode::kReducevq, "reducevq" }
};

} // namespace anonymous

std::optional<Opcode> GetOpcodeByName(const std::string_view& name)
{
    for (const auto& pair : g_opcode_name_map)
    {
        if (name == pair.name)
            return pair.opcode;
    }
    return {};
}

} // namespace ir
COBALT_NAMESPACE_END
