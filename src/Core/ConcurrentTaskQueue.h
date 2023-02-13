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

#ifndef COCOA_CORE_CONCURRENTTASKQUEUE_H
#define COCOA_CORE_CONCURRENTTASKQUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>

#include "Core/Project.h"
namespace cocoa {

/**
 * A thread-safe queue which can be accessed concurrently.
 * Typically the queue can be used with an implementation which
 * uses the producer and consumer model, for example a thread pool.
 */

template<typename T>
class ConcurrentTaskQueue
{
    CO_NONCOPYABLE(ConcurrentTaskQueue)
    CO_NONASSIGNABLE(ConcurrentTaskQueue)

public:
    ConcurrentTaskQueue();
    ~ConcurrentTaskQueue() = default;

    /**
     * Push a task into the task queue. `value` must not be nullptr.
     * If there are more than one thread is waiting for tasks,
     * only one of them will be woken up randomly.
     * This operation increases the outstanding tasks counter.
     */
    void Push(std::unique_ptr<T> value);

    /**
     * Pop a task from the head of the queue.
     * Return nullptr if no task is available in the queue.
     * Note that though the `Push` operation increases the outstanding tasks counter,
     * the `Pop` does NOT decrease the counter. Caller should call
     * `NotifyOfCompletion` to decrease the counter explicitly.
     */
    std::unique_ptr<T> Pop();

    /**
     * Pop all the tasks from the queue.
     */
    std::queue<std::unique_ptr<T>> PopAll();

    /**
     * Wait until there are some tasks available in the queue.
     * If there are more than one thread is waiting for tasks,
     * only one of them will be woken up randomly.
     */
    std::unique_ptr<T> WaitPop();

    /**
     * Decrease the outstanding tasks counter.
     */
    void NotifyOfCompletion();

    /**
     * Wait until the outstanding tasks counter reaches zero.
     * If multiple threads are waiting for this, all of them will be notified.
     */
    void WaitDrain();

    /**
     * Dispose the task queue.
     * Wake up all the threads waiting for the tasks, and return
     * nullptr for pending `WaitPop` calls.
     */
    void Dispose();

private:
    bool                           disposed_;
    std::mutex                     task_queue_lock_;
    std::condition_variable        task_queue_cond_;
    std::condition_variable        tasks_drained_;
    std::queue<std::unique_ptr<T>> task_queue_;
    int32_t                        outstanding_tasks_;
};

template<typename T>
ConcurrentTaskQueue<T>::ConcurrentTaskQueue()
    : disposed_(false)
    , outstanding_tasks_(0)
{
}

template<typename T>
void ConcurrentTaskQueue<T>::Push(std::unique_ptr<T> value)
{
    std::scoped_lock<std::mutex> lock(task_queue_lock_);
    task_queue_.push(std::move(value));
    outstanding_tasks_++;
    task_queue_cond_.notify_one();
}

template<typename T>
std::unique_ptr<T> ConcurrentTaskQueue<T>::Pop()
{
    std::scoped_lock<std::mutex> lock(task_queue_lock_);
    if (task_queue_.empty())
        return nullptr;
    std::unique_ptr<T> result = std::move(task_queue_.front());
    task_queue_.pop();
    return result;
}

template<typename T>
std::queue<std::unique_ptr<T>> ConcurrentTaskQueue<T>::PopAll()
{
    std::scoped_lock<std::mutex> lock(task_queue_lock_);
    // Fastpath if the queue is empty we return an empty queue
    if (task_queue_.empty())
        return {};

    std::queue<std::unique_ptr<T>> queue;
    task_queue_.swap(queue);
    return std::move(queue);
}

template<typename T>
std::unique_ptr<T> ConcurrentTaskQueue<T>::WaitPop()
{
    std::unique_lock<std::mutex> lock(task_queue_lock_);
    task_queue_cond_.wait(lock, [this]() {
        return (this->disposed_ || !this->task_queue_.empty());
    });

    if (disposed_)
        return nullptr;

    std::unique_ptr<T> result = std::move(task_queue_.front());
    task_queue_.pop();
    return result;
}

template<typename T>
void ConcurrentTaskQueue<T>::NotifyOfCompletion()
{
    std::scoped_lock<std::mutex> lock(task_queue_lock_);
    if (--outstanding_tasks_ == 0)
    {
        tasks_drained_.notify_all();
    }
}

template<typename T>
void ConcurrentTaskQueue<T>::WaitDrain()
{
    std::unique_lock<std::mutex> lock(task_queue_lock_);
    while (outstanding_tasks_ > 0)
    {
        tasks_drained_.wait(lock);
    }
}

template<typename T>
void ConcurrentTaskQueue<T>::Dispose()
{
    std::scoped_lock<std::mutex> lock(task_queue_lock_);
    disposed_ = true;
    task_queue_cond_.notify_all();
}

} // namespace cocoa
#endif //COCOA_CORE_CONCURRENTTASKQUEUE_H
