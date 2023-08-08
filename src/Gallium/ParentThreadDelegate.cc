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

#include "Core/Errors.h"
#include "Gallium/ParentThreadDelegate.h"
#include "Gallium/RuntimeBase.h"
GALLIUM_NS_BEGIN

ParentThreadDelegate::ParentThreadDelegate(RuntimeBase *runtime)
    : runtime_(runtime)
{
}

ParentThreadDelegate::~ParentThreadDelegate() = default;

uv_loop_t *ParentThreadDelegate::GetEventLoop() const
{
    return runtime_->GetEventLoop();
}

void ParentThreadDelegate::NotifyNewWorkerThreadCreated()
{
    message_async_.emplace(runtime_->GetEventLoop(), [&]{
        OnReceiveMessage();
    });
}

void ParentThreadDelegate::PostMessageToMainThread(std::unique_ptr<WorkerMessage> message)
{
    CHECK(message_async_.has_value());
    CHECK(message && message->thread);
    std::scoped_lock<std::mutex> lock(message_queue_lock_);
    message_queue_.emplace(std::move(message));
    message_async_->Send();
}

GALLIUM_NS_END
