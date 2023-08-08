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

#ifndef COCOA_GALLIUM_WORKER_RUNTIME_H
#define COCOA_GALLIUM_WORKER_RUNTIME_H

#include "uv.h"

#include "Gallium/Gallium.h"
#include "Gallium/RuntimeBase.h"
#include "Gallium/ParentThreadDelegate.h"
#include "Gallium/WorkerRuntimeThread.h"
GALLIUM_NS_BEGIN

class WorkerRuntime : public RuntimeBase
{
public:
    WorkerRuntime(uint32_t thread_id, uv_loop_t *event_loop,
                  std::shared_ptr<Platform> platform,
                  ParentThreadDelegate *parent_thread_delegate);
    ~WorkerRuntime() override = default;

    void ReceiveHostMessage(WorkerMessage& message);

private:
    void OnInitialize(v8::Isolate *isolate, v8::Local<v8::Context> context) override;

    ParentThreadDelegate      *parent_thread_delegate_;
};

GALLIUM_NS_END
#endif // COCOA_GALLIUM_WORKER_RUNTIME_H
