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

#ifndef COCOA_CORE_EVENTLOOP_H
#define COCOA_CORE_EVENTLOOP_H

#include <functional>
#include <optional>
#include <memory>

#include <uv.h>

#include "Core/Project.h"
#include "Core/UniquePersistent.h"

namespace cocoa
{

class EventLoop : public ThreadLocalUniquePersistent<EventLoop>
{
public:
    EventLoop();
    ~EventLoop();
    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

    int run();

    /**
     * Spin the event loop. This method performs roughly following steps:
     * 1. Run the event loop until it exits normally
     * 2. Call `func` callback function
     * 3. If the loop is alive again (uv_loop_alive), go to step 1
     */
    void spin(const std::function<void(void)>& func);

    void walk(std::function<void(uv_handle_t*)> function);

    g_nodiscard inline uv_loop_t *handle() {
        return &loop_;
    }

    template<typename T>
    using TaskRoutine = std::function<T()>;
    using TaskRoutineVoid = TaskRoutine<void>;

    template<typename T>
    using PostTaskRoutine = std::function<void(T&&)>;
    using PostTaskRoutineVoid = std::function<void()>;

    /**
     * Submit an asynchronous task to execute in the thread pool.
     * Task will be queued and executed later. When a task is executed, `task` will be
     * called by a worker thread; when a task is finished, `post_task` will be called
     * by event loop in current thread.
     */
    template<typename T>
    void enqueueThreadPoolTask(const TaskRoutine<T>& task, const PostTaskRoutine<T>& post_task);

    void enqueueThreadPoolTrivialTask(const TaskRoutineVoid& task, const PostTaskRoutineVoid& post_task);

    void dispose();

private:
    uv_loop_t loop_;
};

template<typename T>
void EventLoop::enqueueThreadPoolTask(const TaskRoutine<T>& task, const PostTaskRoutine<T>& post_task)
{
    using Holder = std::optional<T>;

    auto ret_holder = std::make_shared<Holder>();
    enqueueThreadPoolTrivialTask([ret_holder, task]() -> void {
        *ret_holder = std::move(task());
    }, [ret_holder, post_task]() -> void {
        post_task(std::move(**ret_holder));
    });
}

namespace uv {

template<typename T>
class HandleBase
{
    CO_NONASSIGNABLE(HandleBase)
    CO_NONCOPYABLE(HandleBase)

public:
    HandleBase() : ref_count_(1), handle_(new T{}) {}

    HandleBase(HandleBase<T>&& rhs) noexcept
        : handle_(rhs.handle_), ref_count_(rhs.ref_count_) { rhs.handle_ = nullptr; }

    ~HandleBase() {
        Close();
    }

    void Close() {
        if (!handle_)
            return;
        uv_close(reinterpret_cast<uv_handle_t *>(handle_), [](uv_handle_t *p) {
            delete reinterpret_cast<T *>(p);
        });
        handle_ = nullptr;
    }

    g_nodiscard T *Get() const {
        return handle_;
    }

    void Unref() {
        uv_unref(reinterpret_cast<uv_handle_t *>(handle_));
    }

    void Ref() {
        uv_ref(reinterpret_cast<uv_handle_t *>(handle_));
    }

    void RefCounted() {
        ref_count_++;
        Ref();
    }

    void UnrefCounted() {
        CHECK(ref_count_ > 0 && "UnrefCount() without its counterpart RefCount()");
        if (ref_count_ == 1)
        {
            Unref();
            ref_count_ = 0;
            return;
        }
        ref_count_--;
    }

private:
    int32_t ref_count_;
    T *handle_;
};

#define START_STOP_HANDLE_IMPL(cl, name)                            \
    class cl : public HandleBase<uv_##name##_t>                     \
    {                                                               \
    public:                                                         \
        explicit cl(uv_loop_t *loop) {                              \
            uv_##name##_init(loop, Get());                          \
            Get()->data = this;                                     \
        }                                                           \
        void Start(std::function<void(void)> func) {                \
            func_ = std::move(func);                                \
            uv_##name##_start(Get(), [](uv_##name##_t *h) {         \
                static_cast<cl*>(h->data)->func_();                 \
            });                                                     \
        }                                                           \
        void Stop() {                                               \
            uv_##name##_stop(Get());                                \
        }                                                           \
    private:                                                        \
        std::function<void(void)> func_;                            \
    };

START_STOP_HANDLE_IMPL(CheckHandle, check)
START_STOP_HANDLE_IMPL(PrepareHandle, prepare)
START_STOP_HANDLE_IMPL(IdleHandle, idle)

#undef START_STOP_HANDLE_IMPL

class AsyncHandle : public HandleBase<uv_async_t>
{
public:
    AsyncHandle(uv_loop_t *loop, std::function<void(void)> func)
        : func_(std::move(func))
    {
        uv_async_init(loop, Get(), [](uv_async_t *h) {
            static_cast<AsyncHandle*>(h->data)->func_();
        });
        Get()->data = this;
    }

    void Send() {
        uv_async_send(Get());
    }

private:
    std::function<void(void)> func_;
};

class PollHandle : public HandleBase<uv_poll_t>
{
public:
    PollHandle(uv_loop_t *loop, int fd) {
        uv_poll_init(loop, Get(), fd);
        Get()->data = this;
    }

    void Start(int events, std::function<void(int, int)> func) {
        func_ = std::move(func);
        uv_poll_start(Get(), events, [](uv_poll_t *h, int status, int events) {
            PollHandle *self = static_cast<PollHandle*>(h->data);
            std::function<void(int, int)> func  = std::move(self->func_);
            func(status, events);
            if (!self->func_)
                self->func_ = std::move(func);
        });
    }

    void Stop() {
        uv_poll_stop(Get());
        func_ = {};
    };

private:
    std::function<void(int, int)> func_;
};

class TimerHandle : public HandleBase<uv_timer_t>
{
public:
    explicit TimerHandle(uv_loop_t *loop) {
        uv_timer_init(loop, Get());
        Get()->data = this;
    }

    void Start(int64_t timeout, int64_t repeat, std::function<void()> func) {
        CHECK(func && "callback must be provided");
        func_ = std::move(func);
        uv_timer_start(Get(), [](uv_timer_t *handle) {
            TimerHandle *self = static_cast<TimerHandle*>(handle->data);

            // The user may reset the callback function in `func()`, by calling `Start()`
            // to update the timer. If that happens, the function should be moved out and
            // stored in the stack temporarily to keep the lambda captures alive.
            std::function<void()> func = std::move(self->func_);
            func();

            // If the user does not reset the callback function, restore it.
            if (!self->func_)
                self->func_ = std::move(func);
        }, timeout, repeat);
    }

    void Stop() {
        uv_timer_stop(Get());
        func_ = {};
    }

    void Again() {
        uv_timer_again(Get());
    }

private:
    std::function<void()> func_;
};

} // namespace uv
} // namespace cocoa
#endif //COCOA_CORE_EVENTLOOP_H
