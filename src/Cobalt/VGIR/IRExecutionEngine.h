#ifndef COCOA_COBALT_VGIR_IREXECUTIONENGINE_H
#define COCOA_COBALT_VGIR_IREXECUTIONENGINE_H

#include "include/core/SkPicture.h"

#include "Core/Data.h"
#include "Core/Errors.h"
#include "Cobalt/Cobalt.h"
#include "Cobalt/VGIR/IRInstruction.h"
#include "Cobalt/VGIR/IREvaluationStack.h"
#include "Cobalt/VGIR/IRPersistentHeap.h"
COBALT_NAMESPACE_BEGIN
namespace ir {

class InstructionDecoder
{
public:
    virtual ~InstructionDecoder() = default;

    virtual void Reset() = 0;
    virtual void SeekToInstruction(uint64_t offset) = 0;
    virtual Instruction *FetchNext() = 0;
};

class ExecutionTracer
{
public:
    virtual ~ExecutionTracer() = default;

    virtual void PreExecution(const Instruction *inst, const EvaluationStack& stack) {}
    virtual void PostExecution(const Instruction *inst, const EvaluationStack& stack) {}
};

class ExecutionArgsPack
{
public:
    ExecutionArgsPack() = default;
    ~ExecutionArgsPack() = default;

    g_nodiscard g_inline const StackValue& Get(int32_t idx) const {
        CHECK(idx >= 0 && idx < args_.size());
        return args_[idx];
    }

    template<typename T>
    g_inline ExecutionArgsPack& EmplaceBack(T&& v) {
        args_.emplace_back(std::forward<T>(v));
        return *this;
    }

    g_nodiscard g_inline size_t Length() const {
        return args_.size();
    }

private:
    std::vector<StackValue> args_;
};

class ExecutionEngine
{
public:
    explicit ExecutionEngine(co_unique<InstructionDecoder> decoder);
    ~ExecutionEngine();

    g_nodiscard g_inline const co_unique<InstructionDecoder>& GetDecoder() {
        return decoder_;
    }

    g_inline void SetExecutionTracer(const co_sp<ExecutionTracer>& tracer) {
        execution_tracer_ = tracer;
    }

    sk_sp<SkPicture> Perform(const ExecutionArgsPack& args);

private:
    co_sp<ExecutionTracer>          execution_tracer_;
    co_unique<InstructionDecoder>   decoder_;
    co_unique<PersistentHeap>       persistent_heap_;
};

} // namespace ir
COBALT_NAMESPACE_END
#endif //COCOA_COBALT_VGIR_IREXECUTIONENGINE_H
