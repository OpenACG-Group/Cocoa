#include <thread>
#include <sstream>

#include "Scripter/ScripterBase.h"
#include "Scripter/AsyncWorker.h"

SCRIPTER_NS_BEGIN

#define SCOPED_LOCK(L) std::unique_lock<std::mutex> scopedLock(L);

AsyncWorkerPool::AsyncWorkerPool(uint32_t numThreads)
    : fStop(false)
{
    for (int32_t i = 0; i < numThreads; i++)
        fThreads.emplace_back(&AsyncWorkerPool::workerRoutine, this, i + 1);
}

AsyncWorkerPool::~AsyncWorkerPool()
{
    this->stopAll();
    for (auto& th : fThreads)
    {
        if (th.joinable())
            th.join();
    }
}

void AsyncWorkerPool::stopAll()
{
    {
        SCOPED_LOCK(fQueueMutex)
        fStop = true;
    }
    fQueueCond.notify_all();
}

void AsyncWorkerPool::submit(const std::shared_ptr<AsyncOpTask>& task)
{
    {
        SCOPED_LOCK(fQueueMutex)
        if (fStop)
            return;
        fTaskQueue.emplace(task);
    }
    fQueueCond.notify_one();
}

void AsyncWorkerPool::workerRoutine(uint32_t id)
{
#if defined(__linux__)
    {
        std::ostringstream iss;
        iss << "KWorker#" << id;
        std::string str(iss.str());
        pthread_setname_np(pthread_self(), str.c_str());
    }
#endif // __linux__

    while (true)
    {
        std::shared_ptr<AsyncOpTask> task;
        {
            SCOPED_LOCK(fQueueMutex)
            fQueueCond.wait(scopedLock, [this]() {
                return this->fStop || !this->fTaskQueue.empty();
            });

            if (fStop && fTaskQueue.empty())
                break;

            task = std::move(fTaskQueue.front());
            fTaskQueue.pop();
        }

        if (task == nullptr)
            continue;
        task->invokeByWorker();
        {
            SCOPED_LOCK(fInvokedQueueMutex)
            fInvokedQueue.push(task);
        }
    }
}

void AsyncWorkerPool::takeInvokedTasks(std::vector<std::shared_ptr<AsyncOpTask>>& vec)
{
    SCOPED_LOCK(fInvokedQueueMutex)
    while (!fInvokedQueue.empty())
    {
        vec.push_back(fInvokedQueue.front());
        fInvokedQueue.pop();
    }
}

SCRIPTER_NS_END
