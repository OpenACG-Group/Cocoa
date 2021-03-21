#ifndef COCOA_REACTOR_H
#define COCOA_REACTOR_H

#include <cstdint>
#include <vector>

#define REACTOR_NS_BEGIN    namespace cocoa::reactor {
#define REACTOR_NS_END      }

REACTOR_NS_BEGIN

using ImmByteT = uint8_t;
using ImmWordT = uint16_t;
using ImmDWordT = uint32_t;
using ImmQWordT = uint64_t;
using ImmFloatT = double;

enum class IROpcode : uint8_t
{
    kOpcode_Move       = 0x20,
    kOpcode_Load,
    kOpcode_Store,
    kOpcode_Add,
    kOpcode_Sub,
    kOpcode_Mul,
    kOpcode_Div,
    kOpcode_And,
    kOpcode_Or,
    kOpcode_Xor,
    kOpcode_Not,
    kOpcode_RShift,
    kOpcode_LShift,
    kOpcode_Compare,
    kOpcode_Jump,
    kOpcode_Call,
    kOpcode_Ret,
    kOpcode_Prologue,
    kOpcode_Push,
    kOpcode_Pop
};

enum class OperandType : uint8_t
{
    kOperand_Reg,
    kOperand_Mem,
    kOperand_ImmByte,
    kOperand_ImmWord,
    kOperand_ImmDWord,
    kOperand_ImmQWord,
    kOperand_ImmFloat
};

enum class RegisterType : uint8_t
{
    kReg_Gpb0,
    kReg_Gpb1,
    kReg_Gpb2,
    kReg_Gpb3,
    kReG_Gpb4,
    kReg_Gpb5,
    kReg_Gpb6,
    kReg_Gpb7,
    kReg_Gpw0,
    kReg_Gpw1,
    kReg_Gpw2,
    kReg_Gpw3,
    kReg_Gpw4,
    kReg_Gpw5,
    kReg_Gpw6,
    kReg_Gpw7,
    kReg_Gpd0,
    kReg_Gpd1,
    kReg_Gpd2,
    kReg_Gpd3,
    kReg_Gpd4,
    kReg_Gpd5,
    kReg_Gpd6,
    kReg_Gpd7,
    kReg_Gpq0,
    kReg_Gpq1,
    kReg_Gpq2,
    kReg_Gpq3,
    kReg_Gpq4,
    kReg_Gpq5,
    kReg_Gpq6,
    kReg_Gpq7,
    kReg_Gpf0,
    kReg_Gpf1,
    kReg_Gpf2,
    kReg_Gpf3,
    kReg_Gpf4,
    kReg_Gpf5,
    kReg_Gpf6,
    kReg_Gpf7,

    kReg_Cmpr
};

class Compiler;
class BaseOperand;
class CodeResourceManager;
class CodeEmitter;
template<typename T>
class Local
{
public:
    template<typename...ArgsT>
    explicit Local(CodeResourceManager *emitter, ArgsT&&... args)
            : fObject(std::forward<ArgsT>(args)...),
              fManager(emitter) {}

    Local(Local<T>&& rhs) noexcept
            : fObject(std::move(rhs.fObject)),
              fManager(rhs.fManager)
    {
        rhs.fManager = nullptr;
    }

    ~Local();

    inline operator T() {
        return fObject;
    }

private:
    T                        fObject;
    CodeResourceManager     *fManager;
};

class CodeEmitter
{
public:
    explicit CodeEmitter(Compiler *compiler);

#define OP(n)   const BaseOperand& op##n
#define EMIT(name, args...) void name(args);

    EMIT(Move, OP(1), OP(2))
    EMIT(Load, OP(1), OP(2))
    EMIT(Store, OP(1), OP(2))
    EMIT(Add, OP(1), OP(2))
    EMIT(Sub, OP(1), OP(2))
    EMIT(Mul, OP(1), OP(2))
    EMIT(Div, OP(1), OP(2))
    EMIT(And, OP(1), OP(2))
    EMIT(Or, OP(1), OP(2))
    EMIT(Xor, OP(1), OP(2))
    EMIT(Not, OP(1))
    EMIT(RShift, OP(1), OP(2))
    EMIT(LShift, OP(1), OP(2))
    EMIT(Compare, OP(1), OP(2))
    EMIT(Jump, OP(1))
    EMIT(Call, OP(1), OP(2))
    EMIT(Ret)
    EMIT(Prologue)
    EMIT(Push, OP(1))
    EMIT(Pop, OP(1))

#undef EMIT
#undef OP

private:
    Compiler    *fCompiler;
};

class BaseOperand
{
public:
    explicit BaseOperand(OperandType type)
        : fType(type) {}
    virtual ~BaseOperand() = default;

    inline OperandType type()       { return fType; }

    inline ImmByteT toImmByte()     { return byteValue(); }
    inline ImmWordT toImmWord()     { return wordValue(); }
    inline ImmDWordT toImmDWord()   { return dwordValue(); }
    inline ImmQWordT toImmQWord()   { return qwordValue(); }
    inline ImmFloatT toImmFloat()   { return floatValue(); }

protected:
    inline virtual ImmByteT byteValue()     { throwBadValueType(); return 0; }
    inline virtual ImmWordT wordValue()     { throwBadValueType(); return 0; }
    inline virtual ImmDWordT dwordValue()   { throwBadValueType(); return 0; }
    inline virtual ImmQWordT qwordValue()   { throwBadValueType(); return 0; }
    inline virtual ImmFloatT floatValue()   { throwBadValueType(); return 0; }

private:
    void throwBadValueType();

    OperandType     fType;
};

template<typename T, OperandType OprType>
class ImmOperandT : public BaseOperand
{
public:
    explicit OperandT(T&& value)
        : BaseOperand(OprType),
          fValue(value) {}

    ~OperandT() override = default;

private:
    inline ImmByteT byteValue() override    { return fValue; }
    inline ImmWordT wordValue() override    { return fValue; }
    inline ImmDWordT dwordValue() override  { return fValue; }
    inline ImmQWordT qwordValue() override  { return fValue; }
    inline ImmFloatT floatValue() override  { return fValue; }

    T   fValue;
};

using OperandImmByte = ImmOperandT<ImmByteT, OperandType::kOperand_ImmByte>;
using OperandImmWord = ImmOperandT<ImmWordT, OperandType::kOperand_ImmWord>;
using OperandImmDWord = ImmOperandT<ImmDWordT, OperandType::kOperand_ImmDWord>;
using OperandImmQWord = ImmOperandT<ImmQWordT, OperandType::kOperand_ImmQWord>;
using OperandImmFloat = ImmOperandT<ImmFloatT, OperandType::kOperand_ImmFloat>;

class OperandReg
{
public:
    explicit OperandReg(RegisterType reg)
        : fRegType(reg) {}
    ~OperandReg() override = default;

    inline RegisterType regType()   { return fRegType; }

private:
    RegisterType        fRegType;
};

class Compiler
{
public:
    Local<BaseOperand> newImm(uint8_t val);
    Local<BaseOperand> newImm(int8_t val);
    Local<BaseOperand> newImm(uint16_t val);
    Local<BaseOperand> newImm(int16_t val);
    Local<BaseOperand> newImm(uint32_t val);
    Local<BaseOperand> newImm(int32_t val);
    Local<BaseOperand> newImm(uint64_t val);
    Local<BaseOperand> newImm(int64_t val);
    Local<BaseOperand> newImm(double val);
    Local<BaseOperand> newGp(RegisterType type);
};

REACTOR_NS_END
#endif //COCOA_REACTOR_H
