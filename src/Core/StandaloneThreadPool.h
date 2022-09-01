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

#ifndef COCOA_CORE_STANDALONETHREADPOOL_H
#define COCOA_CORE_STANDALONETHREADPOOL_H

#include <future>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <functional>
#include <queue>

#include "uv.h"

#include "Core/Exception.h"
namespace cocoa {

class StandaloneThreadPool
{
public:
    using TaskRoutine = std::function<void()>;

    /**
     * An appropriate number depending on the number of CPU cores
     * will be used if `count` is 0.
     */
    explicit StandaloneThreadPool(const std::string_view& workerName, uint32_t count = 0);
    ~StandaloneThreadPool();

    template<typename F, typename ...ArgsT>
    auto enqueue(F&& f, ArgsT&&... args)
        -> std::future<typename std::invoke_result<F, ArgsT...>::type>;

    void enqueueTrivial(TaskRoutine routine);

private:
    void workerEntrypoint(uint32_t number);

    uv_barrier_t                    ready_barrier_;
    std::string                     worker_base_name_;
    std::vector<std::thread>        threads_;
    std::queue<TaskRoutine>         task_queue_;
    std::mutex                      queue_lock_;
    std::condition_variable         queue_condition_var_;
    bool                            stop_;
};

template<typename F, typename...ArgsT>
auto StandaloneThreadPool::enqueue(F&& f, ArgsT&& ...args)
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
#endif //COCOA_CORE_STANDALONETHREADPOOL_H
