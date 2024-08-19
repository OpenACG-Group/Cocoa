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

#include <sys/time.h>

#include <map>

#include "fmt/format.h"

#include "Core/Errors.h"
#include "Core/Journal.h"
#include "Core/EventLoop.h"
#include "Core/MeasuredTable.h"
#include "Core/Utils.h"
#include "Gallium/UnixPathTools.h"
#include "Gallium/Infrastructures.h"
#include "Gallium/RuntimeBase.h"
#include "Gallium/ffi/ReturnValue.h"
#include "Gallium/ffi/Function.h"
GALLIUM_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Gallium.Infrastructures)

namespace infra
{

namespace {
struct TimeoutPack
{
    v8::Isolate *isolate;
    v8::Global<v8::Function> callback;
    bool once;
    uint64_t id;
    uv_timer_t timer;
};

thread_local std::map<uint64_t, TimeoutPack *> timeout_callbacks_map_;
thread_local uint64_t timeout_id_counter_ = 1;

void timer_dispose(TimeoutPack *pack)
{
    timeout_callbacks_map_.erase(pack->id);
    uv_timer_stop(&pack->timer);
    uv_close((uv_handle_t *) &pack->timer, [](uv_handle_t *handle) {
        /* Reinterpret as `TimeoutPack` to invoke destructor properly */
        auto *pack = reinterpret_cast<TimeoutPack*>(uv_handle_get_data(handle));
        delete pack;
    });
}

void timer_callback_trampoline(uv_timer_t *timer)
{
    auto *pack = reinterpret_cast<TimeoutPack*>(uv_handle_get_data((uv_handle_t*) timer));
    CHECK(pack);

    RuntimeBase *runtime = RuntimeBase::FromIsolate(pack->isolate);
    CHECK(runtime);

    v8::HandleScope scope(pack->isolate);
    runtime->PerformTasksCheckpoint();

    uint64_t timer_id = pack->id;
    v8::Local<v8::Function> func = pack->callback.Get(pack->isolate);
    (void) func->Call(pack->isolate->GetCurrentContext(), v8::Null(pack->isolate), 0, nullptr);

    // The first condition is to check whether user code has disposed the timer.
    if (timeout_callbacks_map_.find(timer_id) != timeout_callbacks_map_.end() && pack->once)
        timer_dispose(pack);
}

} // namespace anonymous

ffi::Ret<uint64_t> JS_setTimerCallback(int64_t timeout, bool repeat, v8::Local<v8::Function> func)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (timeout < 0)
        return ffi::Fail(ffi::kRangeErr, "invalid timeout value");

    auto *pack = new TimeoutPack{};
    pack->isolate = isolate;
    pack->callback.Reset(isolate, func);
    pack->once = !repeat;
    pack->id = timeout_id_counter_++;

    uv_loop_t *loop = RuntimeBase::FromIsolate(isolate)->GetEventLoop();
    uv_timer_init(loop, &pack->timer);
    uv_handle_set_data((uv_handle_t*) &pack->timer, pack);
    uv_timer_start(&pack->timer, timer_callback_trampoline, timeout, repeat ? timeout : 0);

    timeout_callbacks_map_[pack->id] = pack;
    return pack->id;
}

ffi::Ret<void> JS_clearTimerCallback(uint64_t id)
{
    auto itr = timeout_callbacks_map_.find(id);
    if (itr == timeout_callbacks_map_.end())
        return ffi::Fail(ffi::kErr, "invalid timer ID to clear");

    timer_dispose(itr->second);
    return {};
}

namespace {
thread_local struct timeval tv_start_{};
}

ffi::Ret<double> JS_millisecondTimeCounter()
{
    struct timeval tv{};
    gettimeofday(&tv, nullptr);

    double val = static_cast<double>(tv.tv_sec - tv_start_.tv_sec) * 1e3
                 + static_cast<double>(tv.tv_usec - tv_start_.tv_usec) / 1e3;

    return val;
}

void InstallOnGlobalContext(v8::Isolate *isolate,
                            v8::Local<v8::Context> context,
                            bool is_worker_scope)
{
    v8::HandleScope scope(isolate);
    v8::Local<v8::Object> global = context->Global();

    gettimeofday(&tv_start_, nullptr);

#define JSSTR(s) v8::String::NewFromUtf8Literal(isolate, s)

    if (is_worker_scope)
        global->Set(context, JSSTR("self"), global).Check();
    else
        global->Set(context, JSSTR("global"), global).Check();

    global->Set(context, JSSTR("__runtime__"), ffi::Cast<ffi::ObjectLiteralMap>::ToChecked(isolate, {
        { "version", v8::String::NewFromUtf8Literal(isolate, COCOA_VERSION) },
        { "implementation", v8::String::NewFromUtf8Literal(isolate, COCOA_NAME) },
        { "platform", v8::String::NewFromUtf8Literal(isolate, COCOA_PLATFORM) },
        { "isWorkerGlobalScope", v8::Boolean::New(isolate, is_worker_scope) }
    })).Check();

    global->Set(context, JSSTR("setTimerCallback"),
                ffi::WrapFunctionAddress(isolate, v8::SideEffectType::kHasNoSideEffect, JS_setTimerCallback)->GetFunction(context).ToLocalChecked()).Check();

#define EXPORT_FUNC(name, cb) \
    global->Set(context, JSSTR(name), ffi::WrapFunctionAddress( \
    isolate, v8::SideEffectType::kHasNoSideEffect, cb)->GetFunction(context).ToLocalChecked()).Check()

    EXPORT_FUNC("setTimerCallback", JS_setTimerCallback);
    EXPORT_FUNC("clearTimerCallback", JS_clearTimerCallback);
    EXPORT_FUNC("getMillisecondTimeCounter", JS_millisecondTimeCounter);

#undef EXPORT_FUNC
}

void ReportUncaughtException(v8::Isolate *isolate, v8::Local<v8::Message> message,
                             v8::Local<v8::Value> except)
{
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    RuntimeBase *rt = RuntimeBase::FromIsolate(isolate);
    CHECK(rt);

    v8::Local<v8::String> string = except->ToString(ctx).ToLocalChecked();
    v8::String::Utf8Value string_u8v(isolate, string);
    QLOG(LOG_ERROR, "%fg<re>Uncaught exception: {}%reset", *string_u8v);

    if (message->GetStackTrace().IsEmpty())
        return;

    QLOG(LOG_ERROR, "  %fg<re>Stack traceback:%reset");
    v8::Local<v8::StackTrace> trace = message->GetStackTrace();
    MeasuredTable mt(/* minSpace */ 1);
    for (int32_t i = 0; i < trace->GetFrameCount(); i++)
    {
        v8::Local<v8::StackFrame> frame = trace->GetFrame(isolate, i);
        std::string funcName = "<unknown>", scriptName = "<unknown>";
        std::string funcNamePrefix;

        if (frame->IsConstructor())
            funcNamePrefix = "new ";

        if (!frame->GetScriptName().IsEmpty())
        {
            scriptName = ffi::Cast<std::string>::FromChecked(isolate, frame->GetScriptName());
            if (utils::StrStartsWith(scriptName, "file://"))
            {
                scriptName = "file://" + unixpath::SolveShortestPathRepresentation(scriptName.substr(7));
            }
        }
        if (!frame->GetFunctionName().IsEmpty())
            funcName = ffi::Cast<std::string>::FromChecked(isolate, frame->GetFunctionName());

        if (frame->GetLineNumber() != v8::Message::kNoLineNumberInfo)
            scriptName.append(fmt::format(":{}", frame->GetLineNumber()));
        if (frame->GetColumn() != v8::Message::kNoColumnInfo)
            scriptName.append(fmt::format(":{}", frame->GetColumn()));

        mt.append(fmt::format("%fg<bl>#{}%reset %italic%fg<ye>{}{}%reset", i, funcNamePrefix, funcName),
                  fmt::format("%fg<cy>(from {})%reset", scriptName));
    }
    mt.flush([](const std::string& line) {
        QLOG(LOG_ERROR, "    {}", line);
    });
}

} // namespace infra
GALLIUM_NS_END
