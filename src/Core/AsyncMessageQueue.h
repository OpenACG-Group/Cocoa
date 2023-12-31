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
#include <condition_variable>

#include "Core/Project.h"
#include "Core/EventLoop.h"
COCOA_BEGIN_NAMESPACE

template<typename T, typename UniquePtr = std::unique_ptr<T>>
class AsyncMessageQueue
{
public:
    using Message = UniquePtr;
    using HandlerF = std::function<void(Message, AsyncMessageQueue<T, UniquePtr> *self)>;

    class MessageListener
    {
    public:
        virtual void OnMessage(Message message, AsyncMessageQueue<T, UniquePtr> *self) = 0;
    };

    AsyncMessageQueue(uv_loop_t *event_loop, HandlerF message_handler)
        : message_handler_(std::move(message_handler))
        , message_listener_(nullptr)
        , notifier_(event_loop, [this] { OnMessageComing(); })
        , non_blocking_(false) {}

    AsyncMessageQueue(uv_loop_t *event_loop, MessageListener *listener)
        : message_listener_(listener)
        , notifier_(event_loop, [this] { OnMessageComing(); })
        , non_blocking_(false) {}

    AsyncMessageQueue(AsyncMessageQueue<T, UniquePtr>&& rhs) noexcept
        : message_handler_(std::move(rhs.message_handler_))
        , message_listener_(rhs.message_listener_)
        , queue_(std::move(rhs.queue_))
        , notifier_(std::move(rhs.notifier_))
        , non_blocking_(rhs.non_blocking_) {}

    void Enqueue(Message message, std::function<void(const Message&)> finish_enqueue = {});
    void SetNonBlocking(bool non_blocking);

    /**
     * If the event loop running, the message handler/listener will
     * be called when there are messages enqueued. You can only set
     * one of the handler or listener.
     * Note that if a message is consumed by `WaitOnce()`, the message
     * handler/listener will not be fired.
     */
    void SetMessageHandler(HandlerF handler);
    void SetMessageListener(MessageListener *listener);

    /**
     * Block the current thread to wait for the next message enqueued
     * into the message queue, and return that message.
     * Note that messages consumed by `WaitOnce()` will NOT be handled
     * by the message handler/listener.
     */
    Message WaitOnce();

private:
    void OnMessageComing();

private:
    HandlerF                     message_handler_;
    MessageListener             *message_listener_;
    std::mutex                   queue_lock_;
    std::queue<Message>          queue_;
    std::condition_variable      queue_cond_;
    uv::AsyncHandle              notifier_;
    bool                         non_blocking_;
};

template<typename T, typename U>
void AsyncMessageQueue<T, U>::SetMessageHandler(HandlerF handler)
{
    message_handler_ = std::move(handler);
    message_listener_ = nullptr;
}

template<typename T, typename U>
void AsyncMessageQueue<T, U>::SetMessageListener(MessageListener *listener)
{
    CHECK(listener);
    message_listener_ = listener;
    message_handler_ = nullptr;
}

template<typename T, typename U>
void AsyncMessageQueue<T, U>::Enqueue(
        Message message,
        std::function<void(const Message&)> finish_enqueue)
{
    std::scoped_lock<std::mutex> lock(queue_lock_);
    queue_.emplace(std::move(message));
    if (finish_enqueue)
        finish_enqueue(queue_.back());
    notifier_.Send();
    queue_cond_.notify_one();
}

template<typename T, typename U>
void AsyncMessageQueue<T, U>::SetNonBlocking(bool non_blocking)
{
    if (non_blocking == non_blocking_)
        return;
    non_blocking_ = non_blocking;
    if (non_blocking_)
        notifier_.Unref();
    else
        notifier_.Ref();
}

template<typename T, typename U>
void AsyncMessageQueue<T, U>::OnMessageComing()
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

    if (!message_handler_ && !message_listener_)
        return;
    CHECK(!message_handler_ || !message_listener_);

    if (message_handler_)
    {
        for (Message &message : messages)
            message_handler_(std::move(message), this);
    }
    else
    {
        for (Message &message : messages)
            message_listener_->OnMessage(std::move(message), this);
    }
}

template<typename T, typename U>
typename AsyncMessageQueue<T, U>::Message AsyncMessageQueue<T, U>::WaitOnce()
{
    std::unique_lock<std::mutex> lock(queue_lock_);
    queue_cond_.wait(lock, [this]() { return !this->queue_.empty(); });
    auto result = std::move(queue_.front());
    queue_.pop();
    return result;
}

COCOA_END_NAMESPACE
#endif //COCOA_CORE_ASYNCMESSAGEQUEUE_H
