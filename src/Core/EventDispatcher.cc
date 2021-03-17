#include <sys/eventfd.h>
#include <unistd.h>
#include <fcntl.h>

#include "Core/BaseEventProcessor.h"
#include "Core/EventDispatcher.h"
#include "Core/Journal.h"
#include "Core/Exception.h"
namespace cocoa {

EventDispatcher::EventDispatcher()
    : fDispose(false)
{
    fEventFd = eventfd(0, EFD_SEMAPHORE | EFD_CLOEXEC);
    if (fEventFd < 0)
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("Failed to create an EventDispatcher, eventfd is not available")
                .make<RuntimeException>();
    }
}

EventDispatcher::~EventDispatcher()
{
    if (!fProcessorQueue.empty())
    {
        log_write(LOG_WARNING) << "The event dispatcher will be destroyed with "
                               << fProcessorQueue.size() << " event processor(s) in queue"
                               << log_endl;
    }
    close(fEventFd);
}

void EventDispatcher::wakeup(BaseEventProcessor *processor)
{
    std::scoped_lock<std::mutex> lock(fQueueMutex);
    fProcessorQueue.push(processor);
    constexpr int64_t inc = 1;
    write(fEventFd, &inc, sizeof(int64_t));
}

int32_t EventDispatcher::wait() const
{
    if (fDispose)
        return 0;

    int64_t num;
    ssize_t ret;
    do
    {
        ret = read(fEventFd, &num, sizeof(int64_t));
    } while (ret == EINTR);

    if (ret < 0)
    {
        log_write(LOG_ERROR) << "Failed to read event counter" << log_endl;
        return 0;
    }
    return num;
}

void EventDispatcher::handleEvents(int num)
{
    for (int32_t i = 0; i < num; i++)
    {
        fQueueMutex.lock();
        if (fProcessorQueue.empty())
        {
            fQueueMutex.unlock();
            break;
        }

        BaseEventProcessor *processor = fProcessorQueue.front();
        fProcessorQueue.pop();
        fQueueMutex.unlock();

        processor->processEvent();
    }
}

void EventDispatcher::dispose()
{
    fDispose = true;
}

} // namespace cocoa
