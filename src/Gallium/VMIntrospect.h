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

#ifndef COCOA_GALLIUM_VMINTROSPECT_H
#define COCOA_GALLIUM_VMINTROSPECT_H

#include <map>
#include <queue>

#include "include/v8.h"

#include "Gallium/Gallium.h"
GALLIUM_NS_BEGIN

class VMIntrospect
{
public:
    explicit VMIntrospect(v8::Isolate *isolate);
    ~VMIntrospect();

    struct ScheduledTask
    {
        enum class Type
        {
            kInvalid,
            kEvalModuleUrl,
            kEvalScript
        };

        ScheduledTask() : type(Type::kInvalid) {}
        ScheduledTask(const ScheduledTask&) = delete;
        ScheduledTask(ScheduledTask&& rhs) noexcept
            : type(rhs.type)
            , callback(std::move(rhs.callback))
            , reject(std::move(rhs.reject))
            , param(std::move(rhs.param)) {}
        ~ScheduledTask() {
            callback.Reset();
            reject.Reset();
        }

        Type type;
        v8::Global<v8::Function> callback;
        v8::Global<v8::Function> reject;
        std::string param;
    };
    using TaskQueue = std::queue<ScheduledTask>;

    enum class CallbackSlot
    {
        kUncaughtException,
        kBeforeExit,
        kUnhandledPromiseRejection,
        kPromiseMultipleResolve
    };
    using CallbackMap = std::map<CallbackSlot, v8::Global<v8::Function>>;

    enum class PerformCheckpointResult
    {
        kThrow,
        kOk
    };

    enum class MultipleResolveAction
    {
        kResolve,
        kReject
    };

    /**
     * Install global 'introspect' object to current context.
     * @note `GetIsolate` must have an entered context scope.
     */
    static std::unique_ptr<VMIntrospect> InstallGlobal(v8::Isolate *isolate);

    g_nodiscard inline v8::Isolate *getIsolate() const {
        return fIsolate;
    }

    /**
     * These 'notify*' methods try calling corresponding JS callbacks,
     * and returns `true` if we do call them.
     */

    bool notifyUncaughtException(v8::Local<v8::Value> except);
    bool notifyBeforeExit();
    bool notifyUnhandledPromiseRejection(v8::Local<v8::Promise> promise, v8::Local<v8::Value> value);
    bool notifyPromiseMultipleResolve(v8::Local<v8::Promise> promise, MultipleResolveAction action);

    void setCallbackSlot(CallbackSlot slot, v8::Local<v8::Function> func);
    v8::MaybeLocal<v8::Function> getCallbackFromSlot(CallbackSlot slot);

    inline void scheduledTaskEnqueue(ScheduledTask task) {
        fScheduledTaskQueue.emplace(std::move(task));
    }
    PerformCheckpointResult performScheduledTasksCheckpoint();

private:
    CallbackMap     fCallbackMap;
    TaskQueue       fScheduledTaskQueue;
    v8::Isolate    *fIsolate;
};

GALLIUM_NS_END
#endif //COCOA_GALLIUM_VMINTROSPECT_H
