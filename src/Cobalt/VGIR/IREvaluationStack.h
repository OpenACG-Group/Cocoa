#ifndef COCOA_VGIR_IREVALUTATIONSTACK_H
#define COCOA_VGIR_IREVALUTATIONSTACK_H

#include "include/core/SkM44.h"

#include "Cobalt/Cobalt.h"
#include "Cobalt/VGIR/IRInstruction.h"
COBALT_NAMESPACE_BEGIN
namespace ir {

class EvaluationStack
{
public:
    enum class StackValueType : uint8_t
    {
        kScalar,
        kVec2,
        kVec3,
        kVec4,
        kMat4x4
    };

    struct StackValue
    {
        explicit StackValue(SkScalar scalar);
        explicit StackValue(const SkV2& vec2);
        explicit StackValue(const SkV3& vec3);
        explicit StackValue(const SkV4& vec4);
        explicit StackValue(const SkM44& matrix44);
        ~StackValue();

        StackValueType      type;
        union
        {
            SkScalar scalar;
            SkV2 vec2;
            SkV3 vec3;
            SkV4 vec4;
            SkM44 mat44;
        } packed;
    };

    struct StackElement
    {
        StackValue          value;
        StackElement       *prev_ptr;
        StackElement       *next_ptr;
    };

    EvaluationStack();
    ~EvaluationStack();

    void PushValue();

    void PopValue();

private:
    StackElement        *stack_;
};

} // namespace ir
COBALT_NAMESPACE_END
#endif //COCOA_VGIR_IREVALUTATIONSTACK_H
