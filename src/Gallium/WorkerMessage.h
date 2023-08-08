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

#ifndef COCOA_GALLIUM_WORKERMESSAGE_H
#define COCOA_GALLIUM_WORKERMESSAGE_H

#include "Gallium/Gallium.h"
GALLIUM_NS_BEGIN

class WorkerRuntimeThread;

struct WorkerMessage
{
    enum Type
    {
        kTransfer_Type,
        kTerminate_Type
    };

    static std::unique_ptr<WorkerMessage> Terminate(WorkerRuntimeThread *thread) {
        return std::make_unique<WorkerMessage>(kTerminate_Type, thread);
    }

    WorkerMessage(Type type_, WorkerRuntimeThread *thread_)
        : type(type_), thread(thread_) {}

    Type type;
    WorkerRuntimeThread *thread;
};

GALLIUM_NS_END
#endif //COCOA_GALLIUM_WORKERMESSAGE_H
