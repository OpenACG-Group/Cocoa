#ifndef COCOA_CORE_OUTOFLOOPTHREADPOOL_H
#define COCOA_CORE_OUTOFLOOPTHREADPOOL_H

#include <future>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <functional>
#include <queue>

#include "Core/Exception.h"
namespace cocoa {

class OutOfLoopThreadPool
{
public:
    using TaskRoutine = std::function<void()>;

    /**
     * An appropriate number depending on the number of CPU cores
     * will be used if `count` is 0.
     */
    explicit OutOfLoopThreadPool(const std::string_view& workerName, uint32_t count = 0);
    ~OutOfLoopThreadPool();

    template<typename F, class ...ArgsT>
    auto enqueue(F&& f, ArgsT&&... args)
        -> std::future<typename std::invoke_result<F, ArgsT...>::type>;

private:
    void workerEntrypoint(uint32_t number);

    std::string                     worker_base_name_;
    std::vector<std::thread>        threads_;
    std::queue<TaskRoutine>         task_queue_;
    std::mutex                      queue_lock_;
    std::condition_variable         queue_condition_var_;
    bool                            stop_;
};

template<typename F, typename...ArgsT>
auto OutOfLoopThreadPool::enqueue(F&& f, ArgsT&& ...args)
    -> std::future<typename std::invoke_result<F, ArgsT...>::type>
{
    using ReturnType = typename std::result_of<F(ArgsT...)>::type;

    auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<F>(f), std::forward<ArgsT>(args)...));

    std::future<ReturnType> result = task->get_future();
    {
        std::unique_lock<std::mutex> scopedLock(queue_lock_);
        if (stop_)
            throw RuntimeException(__func__, "Enqueue on stopped threadpool");
        task_queue_.emplace([task]() { (*task)(); });
    }

    queue_condition_var_.notify_one();
    return result;
}

} // namespace anonymous
#endif //COCOA_CORE_OUTOFLOOPTHREADPOOL_H
