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

class EventLoop : public UniquePersistent<EventLoop>
{
public:
    EventLoop();
    ~EventLoop();
    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

    int run();

    void walk(std::function<void(uv_handle_t*)> function);

    g_nodiscard inline uv_loop_t *handle() {
        return fLoop;
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
    uv_loop_t   *fLoop;
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

} // namespace cocoa
#endif //COCOA_CORE_EVENTLOOP_H
