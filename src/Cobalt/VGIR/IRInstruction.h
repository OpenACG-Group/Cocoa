#ifndef COCOA_COBALT_VGIR_VGIRINSTRUCTION_H
#define COCOA_COBALT_VGIR_VGIRINSTRUCTION_H

#include "Cobalt/Cobalt.h"
namespace cocoa::cobalt::vgir {

enum class Opcode : uint16_t
{
    /* Stack operations */
    kPush = 0x01,       // [] -> [x]
    kPop,               // [x] -> []
    kRedundant,         // [x] -> [x, x]
    kReducevb,          // [x, y] -> [(x, y)]
    kReducevt,          // [x, y, z] -> [(x, y, z)]
    kReducevq,          // [x, y, z, w] -> [(x, y, z, w)]
    kStore,
    kLoad,

    /* Arithmetic operations */
    kAdd,               // [x, y] -> [x + y]
    kSub,               // [x, y] -> [x - y]
    kMul,               // [x, y] -> [x * y]
    kDiv,               // [x, y] -> [x / y]
    kRem,               // [x, y] -> [x % y]
    kSqrt,              // [x] -> [sqrt(x)]
    kRSqrt,             // [x] -> [1 / sqrt(x)]
    kPow2,              // [x] -> [x ^ 2]
    kPow3,              // [x] -> [x ^ 3]
    kCubic,             // [x] -> [cubic(x)]
    kRCubic,            // [x] -> [1 / cubic(x)]

    /* Vector operations */
    kAddv,              // [u, v] -> [u + v]
    kSubv,              // [u, v] -> [u - v]
    kMulv,              // [v, k] -> [k * v]
    kModv,              // [v] -> [|v|]
    kMovsqv,            // [v] -> [|v| ^ 2]
    kNormv,             // [v] -> [normalized v]
    kDotv,              // [u, v] -> [dot product of u and v]
    kCrossv,            // [u, v] -> [cross product of u and v]
};

enum class OperandType : uint8_t
{
    kRefId,
    kImm
};

struct Operand
{
    using FloatImm = float;
    using ReferenceId = uint32_t;

    OperandType type;
    union
    {
        FloatImm     imm;
        ReferenceId  ref;
    } value;
};

struct Instruction
{
    Opcode      opcode;
    Operand     operands[];
};

#define VGIR_INST_SIZE(nop)     (sizeof(Instruction) + sizeof(Operand) * (nop))

} // namespace cocoa::cobalt::vgir
#endif //COCOA_COBALT_VGIR_VGIRINSTRUCTION_H
