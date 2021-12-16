#include "Core/Errors.h"

#include "fmt/format.h"
#include "Vanilla/RenderKit/RenderMicroTask.h"
VANILLA_NS_BEGIN

RenderMicroTask::RenderMicroTask(uint32_t poolSize)
    : fDisposed(false)
{
    CHECK(poolSize > 0);
    for (uint32_t i = 0; i < poolSize; i++)
    {
        fThreads.emplace_back(&RenderMicroTask::routine, this, i);
    }
}

RenderMicroTask::~RenderMicroTask()
{
    this->dispose();
}

void RenderMicroTask::dispose()
{
    if (fDisposed)
        return;

    {
        std::scoped_lock<std::mutex> lock(fMutex);
        fDisposed = true;
    }
    fCond.notify_all();

    for (auto& thread : fThreads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
}

RenderMicroTask::MTask::Future RenderMicroTask::enqueue(const CallbackType& cb)
{
    MTask task(cb);
    MTask::Future future = task.promise.get_future();
    {
        std::scoped_lock<std::mutex> lock(fMutex);
        fQueue.emplace(std::move(task));
    }
    fCond.notify_one();
    return future;
}

void RenderMicroTask::routine(int number)
{
#ifdef __linux__
    {
        std::string threadName = fmt::format("RenderMTask#{}", number);
        pthread_setname_np(pthread_self(), threadName.c_str());
    }
#endif /* __linux__ */

    while (true)
    {
        std::unique_lock<std::mutex> lock(fMutex);
        fCond.wait(lock, [this] { return fDisposed || !fQueue.empty(); });
        if (fDisposed && fQueue.empty())
            break;
        MTask task(std::move(fQueue.front()));
        fQueue.pop();
        lock.unlock();
        lock.release();

        task.promise.set_value(task.callback());
    }
}

VANILLA_NS_END
