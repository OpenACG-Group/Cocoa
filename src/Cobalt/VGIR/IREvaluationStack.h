#ifndef COCOA_VGIR_IREVALUTATIONSTACK_H
#define COCOA_VGIR_IREVALUTATIONSTACK_H

#include "include/core/SkM44.h"

#include "Cobalt/Cobalt.h"
#include "Cobalt/VGIR/IRInstruction.h"
COBALT_NAMESPACE_BEGIN
namespace ir {

enum class StackValueType : uint8_t
{
    kFlags,
    kScalar,
    kVec2,
    kVec3,
    kVec4,
    kMat4x4,
    kPlaceholder
};

struct StackValue
{
    explicit StackValue(int32_t flags);
    explicit StackValue(SkScalar scalar);
    explicit StackValue(const SkV2& v);
    explicit StackValue(const SkV3& v);
    explicit StackValue(const SkV4& v);
    explicit StackValue(const SkM44& m);
    StackValue(const StackValue& other);
    // StackValue(StackValue&& other) noexcept;
    StackValue();
    ~StackValue();

    g_nodiscard std::string ToString() const;

    StackValueType      type;
    union
    {
        bool _placeholder;

        int32_t flags;
        SkScalar scalar;
        SkV2 vec2;
        SkV3 vec3;
        SkV4 vec4;
        SkM44 mat44;
    } packed;
};

class EvaluationStack
{
public:
    struct StackElement
    {
        StackValue          value;
        StackElement       *prev_ptr;
        StackElement       *next_ptr;
    };

    EvaluationStack();
    ~EvaluationStack();

    template<typename T>
    void PushValue(const T& v) {
        auto *ptr = new StackElement{ StackValue(v), nullptr, nullptr };
        PushUnlinkedElement(ptr);
    }

    void PushRedundantValue(int32_t srcIdx);
    void PopValue();

    void Exchange();

    g_nodiscard bool IsEmpty();
    g_nodiscard StackElement *GetElement(int32_t idx);
    g_nodiscard std::string ToString() const;

private:
    void PushUnlinkedElement(StackElement *ptr);

    StackElement        *stack_base_;
    StackElement        *stack_top_;
};

} // namespace ir
COBALT_NAMESPACE_END
#endif //COCOA_VGIR_IREVALUTATIONSTACK_H
