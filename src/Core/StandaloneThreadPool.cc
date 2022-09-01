/**
 * This file is part of Cocoa.
 *
 * Cocoa is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * Cocoa is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cocoa. If not, see <https://www.gnu.org/licenses/>.
 */

#include "fmt/format.h"

#include "Core/StandaloneThreadPool.h"
#include "Core/Utils.h"
#include "Core/Journal.h"
namespace cocoa {

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Core.StandaloneThreadPool)

StandaloneThreadPool::StandaloneThreadPool(const std::string_view& workerName, uint32_t count)
    : ready_barrier_{}
    , worker_base_name_(workerName)
    , stop_(false)
{
    if (count == 0)
        count = std::thread::hardware_concurrency();

    uv_barrier_init(&ready_barrier_, count + 1);

    QLOG(LOG_DEBUG, "Creating thread pool {}, concurrency is {}", fmt::ptr(this), count);
    for (int32_t i = 1; i <= count; i++)
    {
        threads_.emplace_back(&StandaloneThreadPool::workerEntrypoint, this, i);
    }

    // Wait until all the worker threads are prepared
    uv_barrier_wait(&ready_barrier_);
    uv_barrier_destroy(&ready_barrier_);

    QLOG(LOG_DEBUG, "Thread pool {} finished initializations", fmt::ptr(this));
}

StandaloneThreadPool::~StandaloneThreadPool()
{
    {
        std::unique_lock<std::mutex> scopedLock(queue_lock_);
        stop_ = true;
    }

    queue_condition_var_.notify_all();
    for (std::thread& worker : threads_)
        worker.join();
}

void StandaloneThreadPool::workerEntrypoint(uint32_t number)
{
    auto thread_name = fmt::format("{}#{}", worker_base_name_, number);
    utils::SetThreadName(thread_name.c_str());
    QLOG(LOG_DEBUG, "Thread %fg<gr,hl>\"{}\"%reset is started from thread pool {}",
         thread_name, fmt::ptr(this));

    uv_barrier_wait(&ready_barrier_);

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

void StandaloneThreadPool::enqueueTrivial(TaskRoutine routine)
{
    {
        std::scoped_lock<std::mutex> lock(queue_lock_);
        if (stop_)
            throw RuntimeException(__func__, "Enqueue on stopped threadpool");
        task_queue_.emplace(std::move(routine));
    }
    queue_condition_var_.notify_one();
}


} // namespace cocoa
