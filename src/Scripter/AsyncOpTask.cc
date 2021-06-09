#include "include/v8.h"
#include "Scripter/ScripterBase.h"
#include "Scripter/AsyncOpTask.h"

#include <utility>
SCRIPTER_NS_BEGIN

void AsyncOpTask::DefaultFinalizer(TaskHandle& handle)
{
    // Do nothing in default finalizer.
}

AsyncOpTask::AsyncOpTask(const OpEntry *entry,
                         std::shared_ptr<OpParameterInfo> parameterInfo,
                         v8::Isolate *isolate,
                         v8::Local<v8::Promise::Resolver> resolver,
                         Finalizer finalizer)
    : fEntry(entry),
      fParamPackOwner(std::move(parameterInfo)),
      fIsolate(isolate),
      fResolver(isolate, resolver),
      fFinalizer(std::move(finalizer)),
      fOpRet(0),
      fState(State::kQueued)
{
}

bool AsyncOpTask::isInvoked()
{
    return fState == State::kInvoked || fState == State::kFinalized;
}

bool AsyncOpTask::isFinalized()
{
    return fState == State::kFinalized;
}

std::any AsyncOpTask::profiler(VmcProfilerOption option)
{
    switch (option)
    {
    case VmcProfilerOption::kInvocationStartTime:
        return fProfileTimers[0];
    case VmcProfilerOption::kInvocationEndTime:
        return fProfileTimers[1];
    case VmcProfilerOption::kFinalizationStartTime:
        return fProfileTimers[2];
    case VmcProfilerOption::kFinalizationEndTime:
        return fProfileTimers[3];
    }
}

void AsyncOpTask::invokeByWorker()
{
    fProfileTimers[0] = std::chrono::steady_clock::now();
    assert(fEntry->pfn);
    fOpRet = fEntry->pfn(*fParamPackOwner);
    fState = State::kInvoked;
    fProfileTimers[1] = std::chrono::steady_clock::now();
}

void AsyncOpTask::finalizeAtLocal()
{
    fProfileTimers[2] = std::chrono::steady_clock::now();
    assert(fState == State::kInvoked);
    auto ret = v8::Integer::New(fIsolate, fOpRet);
    auto resolver = fResolver.Get(fIsolate);
    auto context = fIsolate->GetCurrentContext();
    if (fOpRet < 0)
        resolver->Reject(context, ret).Check();
    else
        resolver->Resolve(context, ret).Check();

    TaskHandle handle(this);
    fFinalizer(handle);
    fState = State::kFinalized;
    fProfileTimers[3] = std::chrono::steady_clock::now();
}

SCRIPTER_NS_END
