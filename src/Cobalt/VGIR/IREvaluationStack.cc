#include <sstream>

#include "fmt/format.h"
#include "Cobalt/VGIR/IREvaluationStack.h"

COBALT_NAMESPACE_BEGIN
namespace ir {

StackValue::StackValue(int32_t integer)
    : type(StackValueType::kFlags), packed{.flags = integer} {}

StackValue::StackValue(SkScalar scalar)
    : type(StackValueType::kScalar), packed{.scalar = scalar} {}

StackValue::StackValue(const SkV2& v)
    : type(StackValueType::kVec2), packed{.vec2 = v} {}

StackValue::StackValue(const SkV3& v)
    : type(StackValueType::kVec3), packed{.vec3 = v} {}

StackValue::StackValue(const SkV4& v)
    : type(StackValueType::kVec4), packed{.vec4 = v} {}

StackValue::StackValue(const SkM44& m)
    : type(StackValueType::kMat4x4), packed{.mat44 = m} {}

StackValue::StackValue()
    : type(StackValueType::kPlaceholder), packed{false} {}

StackValue::StackValue(const StackValue& other)
    : type(other.type), packed{false}
{
    switch (type)
    {
    case StackValueType::kFlags:
        packed.flags = other.packed.flags;
        break;
    case StackValueType::kScalar:
        packed.scalar = other.packed.scalar;
        break;
    case StackValueType::kVec2:
        new (&packed.vec2) SkV2(other.packed.vec2);
        break;
    case StackValueType::kVec3:
        new (&packed.vec3) SkV3(other.packed.vec3);
        break;
    case StackValueType::kVec4:
        new (&packed.vec4) SkV4(other.packed.vec4);
        break;
    case StackValueType::kMat4x4:
        new (&packed.mat44) SkM44(other.packed.mat44);
        break;
    case StackValueType::kPlaceholder:
        break;
    }
}

namespace {
template<typename T>
void destruct_at(T& v)
{
    v.~T();
}
} // namespace anonymous

StackValue::~StackValue()
{
    switch (type)
    {
    case StackValueType::kScalar:
    case StackValueType::kPlaceholder:
    case StackValueType::kFlags:
        break;
    case StackValueType::kVec2:
        destruct_at(packed.vec2);
        break;
    case StackValueType::kVec3:
        destruct_at(packed.vec3);
        break;
    case StackValueType::kVec4:
        destruct_at(packed.vec4);
        break;
    case StackValueType::kMat4x4:
        destruct_at(packed.mat44);
        break;
    }
}

std::string StackValue::ToString() const
{
    switch (type)
    {
    case StackValueType::kFlags:
        return fmt::format("@{}", packed.flags);
    case StackValueType::kScalar:
        return fmt::format("${}", packed.scalar);
    case StackValueType::kVec2:
        return fmt::format("({}, {})", packed.vec2.x, packed.vec2.y);
    case StackValueType::kVec3:
        return fmt::format("({}, {}, {})", packed.vec3.x, packed.vec3.y, packed.vec3.z);
    case StackValueType::kVec4:
        return fmt::format("({}, {}, {}, {})", packed.vec4.x, packed.vec4.y, packed.vec4.z, packed.vec4.w);
    case StackValueType::kMat4x4:
        return "<matrix 4x4>";
    case StackValueType::kPlaceholder:
        return "<null>";
    }
}

EvaluationStack::EvaluationStack()
    : stack_base_(new StackElement)
    , stack_top_(stack_base_)
{
}

EvaluationStack::~EvaluationStack()
{
    while (!IsEmpty())
        PopValue();
    delete stack_base_;
}

bool EvaluationStack::IsEmpty()
{
    return (stack_top_ == stack_base_);
}

void EvaluationStack::PopValue()
{
    if (IsEmpty())
        throw std::runtime_error("Empty stack");

    auto *newTop = stack_top_->prev_ptr;
    newTop->next_ptr = nullptr;
    delete stack_top_;
    stack_top_ = newTop;
}

void EvaluationStack::PushUnlinkedElement(StackElement *ptr)
{
    ptr->prev_ptr = stack_top_;
    stack_top_->next_ptr = ptr;
    stack_top_ = ptr;
}

EvaluationStack::StackElement *EvaluationStack::GetElement(int32_t idx)
{
    StackElement *ptr = stack_top_;
    if (idx < 0)
    {
        while (ptr && ++idx)
            ptr = ptr->prev_ptr;
    }
    else
    {
        ptr = stack_base_;
        while (ptr && idx--)
            ptr = ptr->next_ptr;
    }
    if (!ptr)
        throw std::runtime_error("Invalid index");

    return ptr;
}

void EvaluationStack::PushRedundantValue(int32_t srcIdx)
{
    StackElement *e = GetElement(srcIdx);
    PushUnlinkedElement(new StackElement{e->value, nullptr, nullptr});
}

void EvaluationStack::Exchange()
{
    StackElement *prev = stack_top_->prev_ptr;
    if (!prev || prev == stack_base_)
        throw std::runtime_error("Exchange stack less than 2 elements");

    StackElement *prevprev = prev->prev_ptr;
    prev->next_ptr = nullptr;
    prevprev->next_ptr = stack_top_;
    prev->prev_ptr = stack_top_;
    stack_top_->prev_ptr = prevprev;
    stack_top_->next_ptr = prev;
    stack_top_ = prev;
}

std::string EvaluationStack::ToString() const
{
    std::ostringstream oss;
    oss << '[';

    StackElement *e = stack_base_;
    while (e)
    {
        oss << e->value.ToString();
        if (e->next_ptr)
            oss << ", ";
        e = e->next_ptr;
    }
    oss << ']';

    return oss.str();
}

} // namespace ir
COBALT_NAMESPACE_END
