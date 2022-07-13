#include "fmt/format.h"

#include "Core/OutOfLoopThreadPool.h"
#include "Core/Utils.h"
namespace cocoa {

OutOfLoopThreadPool::OutOfLoopThreadPool(const std::string_view& workerName, uint32_t count)
    : worker_base_name_(workerName)
    , stop_(false)
{
    if (count == 0)
        count = std::thread::hardware_concurrency();

    for (int32_t i = 1; i <= count; i++)
    {
        threads_.emplace_back(&OutOfLoopThreadPool::workerEntrypoint, this, i);
    }
}

OutOfLoopThreadPool::~OutOfLoopThreadPool()
{
    {
        std::unique_lock<std::mutex> scopedLock(queue_lock_);
        stop_ = true;
    }

    queue_condition_var_.notify_all();
    for (std::thread& worker : threads_)
        worker.join();
}

void OutOfLoopThreadPool::workerEntrypoint(uint32_t number)
{
    utils::SetThreadName(fmt::format("{}#{}", worker_base_name_, number).c_str());
    while (true)
    {
        TaskRoutine task;
        {
            std::unique_lock<std::mutex> scopedLock(queue_lock_);
            queue_condition_var_.wait(scopedLock, [this] {
                return this->stop_ || !this->task_queue_.empty();
            });

            if (stop_ && task_queue_.empty())
                break;

            task = std::move(task_queue_.front());
            task_queue_.pop();
        }

        task();
    }
}

} // namespace cocoa
