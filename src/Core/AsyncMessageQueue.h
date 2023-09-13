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

#ifndef COCOA_CORE_ASYNCMESSAGEQUEUE_H
#define COCOA_CORE_ASYNCMESSAGEQUEUE_H

#include <queue>
#include <mutex>
#include <functional>

#include "Core/Project.h"
#include "Core/EventLoop.h"
COCOA_BEGIN_NAMESPACE

template<typename T>
class AsyncMessageQueue
{
public:
    using Message = std::unique_ptr<T>;
    using HandlerF = std::function<void(Message, AsyncMessageQueue<T> *self)>;

    explicit AsyncMessageQueue(uv_loop_t *event_loop,
                               HandlerF message_handler)
        : message_handler_(std::move(message_handler))
        , notifier_(event_loop, [this] { OnMessageComing(); })
        , non_blocking_(false) {}

    AsyncMessageQueue(AsyncMessageQueue<T>&& rhs) noexcept
        : message_handler_(std::move(rhs.message_handler_))
        , queue_(std::move(rhs.queue_))
        , notifier_(std::move(rhs.notifier_))
        , non_blocking_(rhs.non_blocking_) {}

    void Enqueue(Message message, std::function<void(const Message&)> finish_enqueue = {});
    void SetNonBlocking(bool non_blocking);
    void SetMessageHandler(HandlerF handler);

private:
    void OnMessageComing();

private:
    HandlerF                message_handler_;
    std::mutex              queue_lock_;
    std::queue<Message>     queue_;
    uv::AsyncHandle         notifier_;
    bool                    non_blocking_;
};

template<typename T>
void AsyncMessageQueue<T>::SetMessageHandler(HandlerF handler)
{
    message_handler_ = std::move(handler);
}

template<typename T>
void AsyncMessageQueue<T>::Enqueue(Message message,
                                   std::function<void(const Message&)> finish_enqueue)
{
    std::scoped_lock<std::mutex> lock(queue_lock_);
    queue_.emplace(std::move(message));
    if (finish_enqueue)
        finish_enqueue(queue_.back());
    notifier_.Send();
}

template<typename T>
void AsyncMessageQueue<T>::SetNonBlocking(bool non_blocking)
{
    if (non_blocking == non_blocking_)
        return;
    non_blocking_ = non_blocking;
    if (non_blocking_)
        notifier_.Unref();
    else
        notifier_.Ref();
}

template<typename T>
void AsyncMessageQueue<T>::OnMessageComing()
{
    queue_lock_.lock();
    std::vector<Message> messages;
    messages.reserve(queue_.size());
    while (!queue_.empty())
    {
        messages.emplace_back(std::move(queue_.front()));
        queue_.pop();
    }
    queue_lock_.unlock();

    if (!message_handler_)
        return;

    for (Message& message : messages)
        message_handler_(std::move(message), this);
}

COCOA_END_NAMESPACE
#endif //COCOA_CORE_ASYNCMESSAGEQUEUE_H
