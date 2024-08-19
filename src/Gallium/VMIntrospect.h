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

#include "Core/TraceEvent.h"
#include "Gallium/Gallium.h"
#include "Gallium/ffi/ReturnValue.h"
#include "Gallium/ffi/JSObject.h"
#include "Gallium/ffi/Interface.h"

GALLIUM_NS_BEGIN

//! TSDecl: @interface TracingConfig
struct TracingConfig
{
    //! TSDecl: @property bufferSizeKb: u32
    size_t buffer_size_kb;

    //! TSDecl: @property enables: @array(string)
    std::vector<std::string> enables;

    //! TSDecl: @property largeTrace: boolean
    bool large_trace;

    //! TSDecl: @property writeToFile: string
    std::string write_to_file;
};
//! TSDecl: @end

//! TSDecl: @interface StacktraceFrame
//! TSDecl: @property line: i32
//! TSDecl: @property column: i32
//! TSDecl: @property scriptName: string
//! TSDecl: @property functionName: string
//! TSDecl: @property isEval: boolean
//! TSDecl: @property isConstructor: boolean
//! TSDecl: @property isWasm: boolean
//! TSDecl: @property isUserJavaScript: boolean
//! TSDecl: @end

//! TSDecl: @class @nonconstructible VMIntrospect
class VMIntrospect : public ffi::JSObject
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    explicit VMIntrospect(v8::Isolate *isolate);
    ~VMIntrospect() override;

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
            , resolver(std::move(rhs.resolver))
            , param(std::move(rhs.param)) {}

        Type type;
        v8::Global<v8::Promise::Resolver> resolver;
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
     * @note `isolate` must have an entered context scope.
     */
    static std::unique_ptr<VMIntrospect> InstallGlobal(v8::Isolate *isolate);

    g_nodiscard inline v8::Isolate *GetIsolate() const {
        return isolate_;
    }

    /**
     * These 'notify*' methods try calling corresponding JS callbacks,
     * and returns `true` if we do call them.
     */

    bool notifyUncaughtException(v8::Local<v8::Value> except);
    bool notifyBeforeExit();
    bool notifyUnhandledPromiseRejection(v8::Local<v8::Promise> promise, v8::Local<v8::Value> value);
    bool notifyPromiseMultipleResolve(v8::Local<v8::Promise> promise, MultipleResolveAction action);

    v8::MaybeLocal<v8::Function> GetCallbackFromSlot(CallbackSlot slot);

    PerformCheckpointResult performScheduledTasksCheckpoint();

    //! TSDecl: @method setUncaughtExceptionHandler(handler: @fn(void, except: any)): void
    ffi::Ret<void> setUncaughtExceptionHandler(v8::Local<v8::Value> handler);

    // TSDecl: @method setBeforeExitHandler(handler: @fn(void)): void
    ffi::Ret<void> setBeforeExitHandler(v8::Local<v8::Value> handler);

    //! TSDecl: @method setUnhandledPromiseRejectionHandler(handler: @fn(void, promise: Promise, value: any)): void
    ffi::Ret<void> setUnhandledPromiseRejectionHandler(v8::Local<v8::Value> handler);

    //! TSDecl: @method setPromiseMultipleResolveHandler(handler: @fn(void, promise: Promise, action: string)): void
    ffi::Ret<void> setPromiseMultipleResolveHandler(v8::Local<v8::Value> handler);

    //! TSDecl: @method scheduleScriptEval(source: string): @promise(void)
    ffi::RetLocal<v8::Value> scheduleScriptEval(const std::string& source);

    //! TSDecl: @method scheduleModuleUrlEval(url: string): @promise(void)
    ffi::RetLocal<v8::Value> scheduleModuleUrlEval(const std::string& url);

    //! TSDecl: @method print(str: string): void
    ffi::Ret<void> print(const std::string& str);

    //! TSDecl: @method hasNativeModule(name: string): void
    ffi::Ret<bool> hasNativeModule(const std::string& name);

    //! TSDecl: @method rewind(frameLimit: i32): @array(StacktraceFrame)
    ffi::RetLocal<v8::Value> rewind(int frame_limit);

    //! TSDecl: @method startProcessTracing(config: TracingConfig): void
    ffi::Ret<void> startProcessTracing(const ffi::IFace<TracingConfig>& config);

    //! TSDecl: @method finishProcessTracing(): void
    ffi::Ret<void> finishProcessTracing();

private:
    ffi::Ret<void> SetCallbackSlot(CallbackSlot slot, v8::Local<v8::Value> func);

    struct TracingSession
    {
        std::unique_ptr<perfetto::TracingSession> session;
        bool is_large_trace;
        std::string write_to_file;
        int32_t large_trace_fd;
    };

    CallbackMap                        callback_map_;
    TaskQueue                          scheduled_task_queue_;
    v8::Isolate                       *isolate_;
    std::unique_ptr<TracingSession>    current_tracing_session_;
};
//! TSDecl: @end

GALLIUM_NS_END
#endif //COCOA_GALLIUM_VMINTROSPECT_H
