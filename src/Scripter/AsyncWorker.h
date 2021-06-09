#ifndef COCOA_ASYNCWORKER_H
#define COCOA_ASYNCWORKER_H

#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <future>

#include "Scripter/ScripterBase.h"
#include "Scripter/AsyncOpTask.h"
SCRIPTER_NS_BEGIN

class AsyncWorkerPool
{
public:
    explicit AsyncWorkerPool(uint32_t numThreads);
    ~AsyncWorkerPool();

    void submit(const std::shared_ptr<AsyncOpTask>& task);
    void stopAll();

    void takeInvokedTasks(std::vector<std::shared_ptr<AsyncOpTask>>& vec);

private:
    void workerRoutine(uint32_t id);

    std::vector<std::thread>    fThreads;
    std::queue<std::shared_ptr<AsyncOpTask>>
                                fTaskQueue;
    std::mutex                  fQueueMutex;
    std::condition_variable     fQueueCond;
    std::queue<std::shared_ptr<AsyncOpTask>>
                                fInvokedQueue;
    std::mutex                  fInvokedQueueMutex;
    bool                        fStop;
};

SCRIPTER_NS_END
#endif //COCOA_ASYNCWORKER_H
