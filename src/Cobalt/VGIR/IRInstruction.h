#ifndef COCOA_COBALT_VGIR_IRINSTRUCTION_H
#define COCOA_COBALT_VGIR_IRINSTRUCTION_H

#include <optional>

#include "Cobalt/Cobalt.h"
COBALT_NAMESPACE_BEGIN
namespace ir {

enum class Opcode : uint16_t
{
    /* Stack operations */
    kPush = 0x01,       // [] -> [x]
    kPop,               // [x] -> []
    kRedundant,         // [x] -> [x, x]
    kExchange,          // [x, y] -> [y, x]

    kLoadconst,         // [] -> [x]    (Load from constant pool)
    kLoad,              // [] -> [x]    (Load from heap)
    kStore,             // [x] -> [x]   (Store to heap)
    kLoadp,             // [] -> [x]    (Load from arguments)

    /* Arithmetic operations */
    kAdd,               // [x, y] -> [x + y]
    kSub,               // [x, y] -> [x - y]
    kMul,               // [x, y] -> [x * y]
    kDiv,               // [x, y] -> [x / y]
    kRem,               // [x, y] -> [x % y]     (Slow emulated operation)
    kInc,               // [x] -> [x + 1]
    kNeg,               // [x] -> [-x]
    kSqrt,              // [x] -> [sqrt(x)]
    kRsqrt,             // [x] -> [1 / sqrt(x)]  (Normal square root)
    kFastrsqrt,         // [x] -> [1 / sqrt(x)]  (Fast Inverse Square Root)
    kPow2,              // [x] -> [x ^ 2]
    kSin,               // [x] -> [sin(x)]
    kCos,               // [x] -> [cos(x)]
    kTan,               // [x] -> [tan(x)]
    kSincos,            // [x] -> [sin(x) * cos(x)]
    kSincosb,           // [x, y] -> [sin(x) * cos(y)]

    /* Vector operations */
    kReducevb,          // [x, y] -> [(x, y)]
    kReducevt,          // [x, y, z] -> [(x, y, z)]
    kReducevq,          // [x, y, z, w] -> [(x, y, z, w)]
    kAddv,              // [u, v] -> [u + v]
    kSubv,              // [u, v] -> [u - v]
    kMulv,              // [v, k] -> [k * v]
    kModv,              // [v] -> [|v|]
    kModsqv,            // [v] -> [|v| ^ 2]
    kNormv,             // [v] -> [normalized v]
    kDotv,              // [u, v] -> [dot product of u and v]
    kCrossv,            // [u, v] -> [cross product of u and v]
    kExtractv,          // [v] -> [a component of v]
    kSwizzlev,          // [v] -> [swizzled v]

    /* Matrix operations */
    kReducemq,          // [m1, m2, ..., m16] -> [[matrix]]
};

enum class OperandType : uint8_t
{
    kConstantPoolRef   = 0x01,
    kImm               = 0x02,
    kFlag              = 0x03
};

struct Operand
{
    using Integer = int32_t;
    using FloatImm = float;
    using ConstantRef = uint32_t;

    OperandType type;
    union
    {
        Integer      flag;
        FloatImm     imm;
        ConstantRef  ref;
    } value;
};

struct Instruction
{
    Opcode      opcode;
    uint8_t     operands_count;
    Operand     operands[];
};

std::optional<Opcode> GetOpcodeByName(const std::string_view& name);

#ifdef IR
#undef IR
#endif
#define IR cocoa::cobalt::ir

#define VGIR_INST_SIZE(p) \
    (sizeof(IR::Instruction) + sizeof(IR::Operand) * (p)->operands_count)

#undef IR

} // namespace ir
COBALT_NAMESPACE_END
#endif //COCOA_COBALT_VGIR_IRINSTRUCTION_H
