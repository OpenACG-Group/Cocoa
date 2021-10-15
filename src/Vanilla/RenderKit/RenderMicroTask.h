#ifndef COCOA_RENDERMICROTASKEXECUTOR_H
#define COCOA_RENDERMICROTASKEXECUTOR_H

#include <utility>
#include <vector>
#include <thread>
#include <queue>
#include <condition_variable>
#include <mutex>
#include <functional>
#include <future>

#include "Vanilla/Base.h"
VANILLA_NS_BEGIN

class RenderMicroTask
{
public:
    using CallbackType = std::function<bool()>;
    struct MTask
    {
        MTask() = default;
        explicit MTask(CallbackType cb)
            : callback(std::move(cb)) {}
        MTask(const MTask&) = delete;
        MTask(MTask&& rhs) noexcept
            : callback(std::move(rhs.callback))
            , promise(std::move(rhs.promise)) {}

        using Future = std::future<bool>;

        CallbackType callback;
        std::promise<bool> promise;
    };

    explicit RenderMicroTask(uint32_t poolSize);
    ~RenderMicroTask();

    MTask::Future enqueue(const CallbackType& cb);
    void dispose();

private:
    void routine(int number);

    std::vector<std::thread>    fThreads;
    std::mutex                  fMutex;
    std::condition_variable     fCond;
    std::queue<MTask>           fQueue;
    bool                        fDisposed;
};

VANILLA_NS_END
#endif //COCOA_RENDERMICROTASKEXECUTOR_H
