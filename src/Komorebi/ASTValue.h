#ifndef COCOA_ASTVALUE_H
#define COCOA_ASTVALUE_H

#include "llvm/IR/Value.h"
#include "Komorebi/Namespace.h"
KOMOREBI_NS_BEGIN

class ASTValue
{
public:
    class TypeInfo
    {
    public:
        enum class Type
        {
            kUnknown,
            kScalar,
            kVec2,
            kVec3,
            kVec4,
            kMat2,
            kMat3,
            kMat4,
            kBool,
            kObjectRef
        };
        enum Attributes
        {
            kAttrNone       = 0,
            kAttrConst      = (1 << 1),
            kAttrUniform    = (1 << 2),
        };

        explicit TypeInfo(Type type, uint32_t attributes = kAttrNone)
            : fType(type), fAttrs(attributes) {}
        TypeInfo()
            : fType(Type::kUnknown), fAttrs(0) {}
        ~TypeInfo() = default;

        inline Type type() const
        { return fType; }

        inline bool isConst() const
        { return (fAttrs & kAttrConst); }

        inline bool isUniform() const
        { return (fAttrs & kAttrUniform); }

    private:
        Type        fType;
        uint32_t    fAttrs;
    };

    ASTValue(bool rvalue, TypeInfo typeInfo, llvm::Value *pValue)
        : fIsRValue(rvalue), fTypeInfo(typeInfo), fValue(pValue) {}
    ASTValue()
        : fIsRValue(false), fTypeInfo(), fValue(nullptr) {}
    ~ASTValue() = default;

    inline bool isRValue() const
    { return fIsRValue; }

    inline bool isLValue() const
    { return !fIsRValue; }

    inline TypeInfo& typeInfo()
    { return fTypeInfo; }

    inline llvm::Value *value()
    { return fValue; }

    inline operator bool() const
    { return fValue != nullptr; }

private:
    bool                fIsRValue;
    TypeInfo            fTypeInfo;
    llvm::Value        *fValue;
};

KOMOREBI_NS_END
#endif //COCOA_ASTVALUE_H
