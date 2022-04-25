#include "Cobalt/VGIR/IRInstruction.h"
#include "Cobalt/VGIR/IRExecutionEngine.h"
#include "Cobalt/VGIR/IREvaluationStack.h"
COBALT_NAMESPACE_BEGIN
namespace ir {

ExecutionEngine::ExecutionEngine(co_unique<InstructionDecoder> decoder)
    : decoder_(std::move(decoder))
    , persistent_heap_(std::make_unique<PersistentHeap>())
{
}

ExecutionEngine::~ExecutionEngine() = default;

#define IMPL(p) case Opcode::k##p:
#define DECL_OPCODE_NAME(name) static std::string __opcode_name__ = name

#define CHECK_OPERANDS_NUM(n)                                                    \
    if (inst->operands_count != (n)) {                                           \
        throw std::runtime_error(                                                \
            "Instruction " + __opcode_name__ + ": invalid number of operands");  \
    }

#define CHECK_OPERAND_TYPE(idx, type_)                                      \
    if (inst->operands[idx].type != OperandType::k##type_) {                \
        throw std::runtime_error(                                           \
            "Instruction " + __opcode_name__ + ": bad type of operand");    \
    }

#define CHECK_VALUE_TYPE(v, type_)                                                \
    if ((v).type != (StackValueType::k##type_)) {                                 \
        throw std::runtime_error(                                                 \
            "Instruction " + __opcode_name__ + ": invalid value type in stack");  \
    }

#define GET_CHECKED_VALUE(idx, v, type_)                \
    StackValue& v = stack.GetElement(idx)->value;       \
    CHECK_VALUE_TYPE(v, type_)

#define GET_XY_BINARY_OPERAND_CHECKED() \
    GET_CHECKED_VALUE(-2, xop, Scalar)    \
    GET_CHECKED_VALUE(-1, yop, Scalar)

#define ST_POP  stack.PopValue();
#define ST_POP2 ST_POP ST_POP
#define ST_POP3 ST_POP2 ST_POP
#define ST_POP4 ST_POP3 ST_POP

#define GET_AB_BINARY_SCALAR_AND_POP                    \
    GET_XY_BINARY_OPERAND_CHECKED()                     \
    float a = xop.packed.scalar, b = yop.packed.scalar;     \
    ST_POP2

#define GET_X_UNARY_SCALAR_AND_POP      \
    GET_CHECKED_VALUE(-1, xop, Scalar)  \
    float x = xop.packed.scalar;        \
    ST_POP

namespace {

template<typename T>
inline T force_interpret_ptr_as(void *ptr)
{
    return *reinterpret_cast<T*>(ptr);
}

/**
 * "Fast Inverse Square Root" algorithm.
 * For more details: https://www.lomont.org/papers/2003/InvSqrt.pdf
 */
float fast_inverse_sqrt(float x)
{
    float half = 0.5f * x;
    int i = force_interpret_ptr_as<int>(&x);
    i = 0x5f3759df - (i >> 1);
    x = force_interpret_ptr_as<float>(&i);
    x = x * (1.5f - half * x * x);
    return x;
}

} // namespace anonymous

sk_sp<SkPicture> ExecutionEngine::Perform(const ExecutionArgsPack& args)
{
    decoder_->Reset();
    EvaluationStack stack;

    Instruction *inst = decoder_->FetchNext();
    while (inst)
    {
        if (execution_tracer_)
            execution_tracer_->PreExecution(inst, stack);

        switch (inst->opcode)
        {
        IMPL(Push)
        {
            DECL_OPCODE_NAME("push");
            CHECK_OPERANDS_NUM(1)
            CHECK_OPERAND_TYPE(0, Imm)
            stack.PushValue(inst->operands[0].value.imm);
            break;
        }
        IMPL(Pop)
        {
            DECL_OPCODE_NAME("pop");
            stack.PopValue();
            break;
        }
        IMPL(Redundant)
        {
            DECL_OPCODE_NAME("redundant");
            stack.PushRedundantValue(-1);
            break;
        }
        IMPL(Exchange)
        {
            DECL_OPCODE_NAME("exchange");
            stack.Exchange();
            break;
        }
        IMPL(Load)
        {
            DECL_OPCODE_NAME("load");
            break;
        }
        IMPL(Store)
        {
            DECL_OPCODE_NAME("store");
            break;
        }
        IMPL(Loadconst)
        {
            DECL_OPCODE_NAME("loadconst");
            break;
        }
        IMPL(Loadp)
        {
            DECL_OPCODE_NAME("loadp");
            CHECK_OPERANDS_NUM(1)
            CHECK_OPERAND_TYPE(0, Flag)
            stack.PushValue(args.Get(inst->operands[0].value.flag));
            break;
        }
        IMPL(Reducevb)
        {
            DECL_OPCODE_NAME("reducevb");
            GET_CHECKED_VALUE(-2, x, Scalar)
            GET_CHECKED_VALUE(-1, y, Scalar)
            SkV2 v{x.packed.scalar, y.packed.scalar};
            ST_POP2
            stack.PushValue(v);
            break;
        }
        IMPL(Reducevt)
        {
            DECL_OPCODE_NAME("reducevt");
            GET_CHECKED_VALUE(-3, x, Scalar)
            GET_CHECKED_VALUE(-2, y, Scalar)
            GET_CHECKED_VALUE(-1, z, Scalar)
            SkV3 v{x.packed.scalar, y.packed.scalar, z.packed.scalar};
            ST_POP3
            stack.PushValue(v);
            break;
        }
        IMPL(Reducevq)
        {
            DECL_OPCODE_NAME("reducevq");
            GET_CHECKED_VALUE(-4, x, Scalar)
            GET_CHECKED_VALUE(-3, y, Scalar)
            GET_CHECKED_VALUE(-2, z, Scalar)
            GET_CHECKED_VALUE(-1, w, Scalar)
            SkV4 v{x.packed.scalar, y.packed.scalar, z.packed.scalar, w.packed.scalar};
            ST_POP4
            stack.PushValue(v);
            break;
        }
        IMPL(Add)
        {
            DECL_OPCODE_NAME("add");
            GET_AB_BINARY_SCALAR_AND_POP
            stack.PushValue(a + b);
            break;
        }
        IMPL(Sub)
        {
            DECL_OPCODE_NAME("sub");
            GET_AB_BINARY_SCALAR_AND_POP
            stack.PushValue(a - b);
            break;
        }
        IMPL(Mul)
        {
            DECL_OPCODE_NAME("mul");
            GET_AB_BINARY_SCALAR_AND_POP
            stack.PushValue(a * b);
            break;
        }
        IMPL(Div)
        {
            DECL_OPCODE_NAME("div");
            GET_AB_BINARY_SCALAR_AND_POP
            if (b == 0)
                throw std::runtime_error("Instruction div: dividing by zero");
            stack.PushValue(a / b);
            break;
        }
        IMPL(Rem)
        {
            DECL_OPCODE_NAME("rem");
            GET_AB_BINARY_SCALAR_AND_POP
            if (b == 0)
                throw std::runtime_error("Instruction rem: dividing by zero");
            stack.PushValue(std::remainder(a, b));
            break;
        }
        IMPL(Inc)
        {
            DECL_OPCODE_NAME("inc");
            GET_CHECKED_VALUE(-1, xop, Scalar)
            ++xop.packed.scalar;
            break;
        }
        IMPL(Neg)
        {
            DECL_OPCODE_NAME("neg");
            GET_CHECKED_VALUE(-1, xop, Scalar)
            xop.packed.scalar = -xop.packed.scalar;
            break;
        }
        IMPL(Sqrt)
        {
            DECL_OPCODE_NAME("sqrt");
            GET_CHECKED_VALUE(-1, xop, Scalar)
            xop.packed.scalar = std::sqrt(xop.packed.scalar);
            break;
        }
        IMPL(Rsqrt)
        {
            DECL_OPCODE_NAME("rsqrt");
            GET_CHECKED_VALUE(-1, xop, Scalar)
            xop.packed.scalar = 1.0f / std::sqrt(xop.packed.scalar);
            break;
        }
        IMPL(Fastrsqrt)
        {
            DECL_OPCODE_NAME("fastrsqrt");
            GET_CHECKED_VALUE(-1, xop, Scalar)
            xop.packed.scalar = fast_inverse_sqrt(xop.packed.scalar);
            break;
        }
        IMPL(Pow2)
        {
            DECL_OPCODE_NAME("pow2");
            GET_CHECKED_VALUE(-1, xop, Scalar)
            xop.packed.scalar = xop.packed.scalar * xop.packed.scalar;
            break;
        }
        IMPL(Sin)
        {
            DECL_OPCODE_NAME("sin");
            GET_CHECKED_VALUE(-1, xop, Scalar)
            xop.packed.scalar = std::sin(xop.packed.scalar);
            break;
        }
        IMPL(Cos)
        {
            DECL_OPCODE_NAME("cos");
            GET_CHECKED_VALUE(-1, xop, Scalar)
            xop.packed.scalar = std::cos(xop.packed.scalar);
            break;
        }
        IMPL(Tan)
        {
            DECL_OPCODE_NAME("tan");
            GET_CHECKED_VALUE(-1, xop, Scalar)
            xop.packed.scalar = std::tan(xop.packed.scalar);
            break;
        }
        IMPL(Sincos)
        {
            DECL_OPCODE_NAME("sincos");
            GET_CHECKED_VALUE(-1, xop, Scalar)
            xop.packed.scalar = std::sin(xop.packed.scalar) * std::cos(xop.packed.scalar);
            break;
        }
        IMPL(Sincosb)
        {
            DECL_OPCODE_NAME("sincosb");
            GET_AB_BINARY_SCALAR_AND_POP
            stack.PushValue(std::sin(a) * std::cos(b));
            break;
        }
        default:
            break;
        }

        if (execution_tracer_)
            execution_tracer_->PostExecution(inst, stack);

        inst = decoder_->FetchNext();
    }

    return nullptr;
}

} // namespace ir
COBALT_NAMESPACE_END
