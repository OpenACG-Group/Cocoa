#ifndef COCOA_ASYNCOPTASK_H
#define COCOA_ASYNCOPTASK_H

#include <chrono>
#include <any>
#include <functional>

#include "include/v8.h"
#include "Scripter/ScripterBase.h"
#include "Scripter/Ops.h"
SCRIPTER_NS_BEGIN

enum class VmcProfilerOption : int32_t
{
    kInvocationStartTime    = 0,
    kInvocationEndTime      = 1,
    kFinalizationStartTime  = 2,
    kFinalizationEndTime    = 3
};

/**
 * An AsyncOpTask object packs a resolver of Promise,
 * a VCTEntry with arguments and a finalizer. The workers
 * thread pool receives some AsyncOpTask objects and execute
 * them asynchronously. But we don't want to operate V8 objects
 * in multiple threads at the same time, so the execution of a
 * AsyncOpTask is divided into two stages.
 * The first stage is called "Invocation", which is completed
 * by worker thread. In this stage, the main part of vmcall
 * will be executed and get a return value. Then the return value
 * will be stored for next stage.
 * The second stage is called "Finalization", which is completed
 * by Scripter thread. In this stage, the finalizer will be
 * called and the Promise will be resolved or rejected.
 * The finalizer can do something necessary that we can't do in
 * the first stage, such as writing V8's ArrayBuffer.
 *
 * Besides, you can get some information about performance by
 * profiler that AsyncOpTask provides. For an example, you can
 * get when the task is invoked and finalized.
 */
class AsyncOpTask
{
public:
    class TaskHandle
    {
    public:
        explicit TaskHandle(AsyncOpTask *pTask)
            : fpTask(pTask) {}
        ~TaskHandle() = default;

        inline void resolvePromise(v8::Local<v8::Value> value)
        {
            auto resolver = fpTask->fResolver.Get(fpTask->fIsolate);
            assert(!resolver.IsEmpty());
            resolver->Resolve(fpTask->fIsolate->GetCurrentContext(), value).Check();
        }

        inline void rejectPromise(v8::Local<v8::Value> value)
        {
            auto resolver = fpTask->fResolver.Get(fpTask->fIsolate);
            assert(!resolver.IsEmpty());
            resolver->Reject(fpTask->fIsolate->GetCurrentContext(), value).Check();
        }

        inline std::shared_ptr<OpParameterInfo> args()
        { return fpTask->fParamPackOwner; }

        inline OpRet returnValue()
        { return fpTask->fOpRet; }

        inline v8::Isolate *isolate()
        { return fpTask->fIsolate; }

    private:
        AsyncOpTask     *fpTask;
    };
    friend class TaskHandle;

    enum class State
    {
        kQueued,
        kInvoked,
        kFinalized
    };

    using Timepoint = std::chrono::steady_clock::time_point;
    using Finalizer = std::function<void(TaskHandle&)>;

    AsyncOpTask(const OpEntry *entry,
                std::shared_ptr<OpParameterInfo> parameterInfo,
                v8::Isolate *isolate,
                v8::Local<v8::Promise::Resolver> resolver,
                Finalizer finalizer = DefaultFinalizer);
    ~AsyncOpTask() = default;

    static void DefaultFinalizer(TaskHandle&);

    bool isInvoked();
    bool isFinalized();
    std::any profiler(VmcProfilerOption option);

    void invokeByWorker();
    void finalizeAtLocal();

private:
    const OpEntry                          *fEntry;
    std::shared_ptr<OpParameterInfo>        fParamPackOwner;
    v8::Isolate                            *fIsolate;
    v8::Global<v8::Promise::Resolver>       fResolver;
    Finalizer                               fFinalizer;
    OpRet                                   fOpRet;
    State                                   fState;
    std::chrono::steady_clock::time_point   fProfileTimers[10];
};

SCRIPTER_NS_END
#endif //COCOA_ASYNCOPTASK_H
