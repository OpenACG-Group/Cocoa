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

#include "Core/Errors.h"
#include "Core/EventLoop.h"
#include "Gallium/Infrastructures.h"
#include "Gallium/Runtime.h"
#include "Gallium/binder/Convert.h"
#include "Gallium/binder/ThrowExcept.h"
GALLIUM_NS_BEGIN
namespace infra
{

namespace {
struct TimeoutPack
{
    ~TimeoutPack()
    {
        callback.Reset();
        for (auto& ref: args)
            ref.Reset();
    }

    v8::Isolate *isolate;
    v8::Global<v8::Function> callback;
    bool once;
    uint64_t id;
    std::vector<v8::Global<v8::Value>> args;
    uv_timer_t timer;
};

std::map<uint64_t, TimeoutPack *> timeout_callbacks_map_;
uint64_t timeout_id_counter_ = 1;

void clearTimer(TimeoutPack *pack)
{
    timeout_callbacks_map_.erase(pack->id);
    uv_close((uv_handle_t *) &pack->timer, [](uv_handle_t *handle) {
        /* Reinterpret as `TimeoutPack` to invoke destructor properly */
        auto *pack = reinterpret_cast<TimeoutPack*>(uv_handle_get_data(handle));
        delete pack;
    });
}

void setTimeoutCallback(uv_timer_t *timer)
{
    auto *pack = reinterpret_cast<TimeoutPack*>(uv_handle_get_data((uv_handle_t*) timer));
    CHECK(pack);

    Runtime *runtime = Runtime::GetBareFromIsolate(pack->isolate);
    CHECK(runtime);

    v8::HandleScope scope(pack->isolate);
    runtime->PerformTasksCheckpoint();

    v8::Local<v8::Function> func = pack->callback.Get(pack->isolate);
    std::vector<v8::Local<v8::Value>> args;
    for (v8::Global<v8::Value>& value : pack->args)
        args.emplace_back(value.Get(pack->isolate));

    v8::Local<v8::Context> context = pack->isolate->GetCurrentContext();
    v8::Local<v8::Object> receiver = context->Global();

    v8::Local<v8::Value> result;
    if (!func->Call(context, receiver, static_cast<int>(args.size()), args.data()).ToLocal(&result))
    {
        if (!pack->once)
            uv_timer_stop(timer);
        clearTimer(pack);
        return;
    }

    if (pack->once)
        clearTimer(pack);
}

void GlobalObjectGetter([[maybe_unused]] v8::Local<v8::Name> property,
                        const v8::PropertyCallbackInfo<v8::Value>& info)
{
    v8::HandleScope scope(info.GetIsolate());
    v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();
    info.GetReturnValue().Set(context->Global());
}

void GlobalObjectSetter([[maybe_unused]] v8::Local<v8::Name> property,
                        [[maybe_unused]] v8::Local<v8::Value> value,
                        [[maybe_unused]] const v8::PropertyCallbackInfo<void>& info)
{
    JS_THROW_IF(true, "Reassigning global object is not permitted", v8::Exception::Error);
}

} // namespace anonymous

void setTimeoutOrInterval(const v8::FunctionCallbackInfo<v8::Value>& info, bool repeat)
{
    v8::Isolate *isolate = info.GetIsolate();
    v8::HandleScope scope(isolate);

    JS_THROW_IF(info.Length() < 2, "At least 2 arguments required", v8::Exception::Error);
    JS_THROW_IF(!info[0]->IsFunction(), "Callback must be a Function", v8::Exception::TypeError);
    JS_THROW_IF(!info[1]->IsNumber(), "Timeout must be a number", v8::Exception::TypeError);

    int64_t timeout = binder::from_v8<decltype(timeout)>(isolate, info[1]);
    JS_THROW_IF(timeout < 0, "Timeout must be a non-negative integer", v8::Exception::RangeError);

    auto *pack = new TimeoutPack{
        isolate,
        v8::Global<v8::Function>(isolate, info[0].As<v8::Function>()),
        !repeat,
        timeout_id_counter_++
    };
    for (int32_t i = 2; i < info.Length(); i++)
        pack->args.emplace_back(isolate, info[i]);

    uv_loop_t *loop = Runtime::GetBareFromIsolate(isolate)->CheckSource::eventLoop()->handle();
    uv_timer_init(loop, &pack->timer);
    uv_handle_set_data((uv_handle_t*) &pack->timer, pack);
    uv_timer_start(&pack->timer, setTimeoutCallback, timeout, repeat ? timeout : 0);

    timeout_callbacks_map_[pack->id] = pack;

    // FIXME: Unsafe type cast from uint64_t to uint32_t
    info.GetReturnValue().Set(static_cast<uint32_t>(pack->id));
}

void JS_setTimeout(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    setTimeoutOrInterval(info, false);
}

void JS_setInterval(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    setTimeoutOrInterval(info, true);
}

void JS_clearTimer(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    v8::Isolate *isolate = info.GetIsolate();

    JS_THROW_IF(info.Length() != 1, "1 argument required", v8::Exception::Error);
    JS_THROW_IF(!info[0]->IsNumber(), "Timer ID must be a number", v8::Exception::TypeError);

    int64_t id = binder::from_v8<decltype(id)>(isolate, info[0]);
    JS_THROW_IF(!timeout_callbacks_map_.count(id), "Invalid timer ID", v8::Exception::Error);
    uv_timer_stop(&timeout_callbacks_map_[id]->timer);
    clearTimer(timeout_callbacks_map_[id]);
}

namespace {
struct timeval tv_start_{};
}

void JS_millisecondTimeCounter(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    struct timeval tv{};
    gettimeofday(&tv, nullptr);

    double val = static_cast<double>(tv.tv_sec - tv_start_.tv_sec) * 1e3
                 + static_cast<double>(tv.tv_usec - tv_start_.tv_usec) / 1e3;

    info.GetReturnValue().Set(val);
}

void InstallOnGlobalContext(v8::Isolate *isolate,
                            v8::Local<v8::Context> context)
{
    v8::HandleScope scope(isolate);
    v8::Local<v8::Object> global = context->Global();

    gettimeofday(&tv_start_, nullptr);

    /* This causes circular reference */
    global->SetAccessor(context, binder::to_v8(isolate, "global"),
                        GlobalObjectGetter, GlobalObjectSetter).Check();

    v8::Local<v8::ObjectTemplate> runtimeInfo = v8::ObjectTemplate::New(isolate);
    runtimeInfo->Set(isolate, "version", binder::to_v8(isolate, COCOA_VERSION));
    runtimeInfo->Set(isolate, "implementation", binder::to_v8(isolate, COCOA_NAME));
    runtimeInfo->Set(isolate, "platform", binder::to_v8(isolate, COCOA_PLATFORM));

    global->Set(context, binder::to_v8(isolate, "__runtime__"),
                runtimeInfo->NewInstance(context).ToLocalChecked()).Check();

#define EXPORT_FUNC(name, cb) \
    global->Set(context, binder::to_v8(isolate, name), CHECKED(v8::Function::New(context, cb))) \
    .Check()

    EXPORT_FUNC("setTimeout", JS_setTimeout);
    EXPORT_FUNC("setInterval", JS_setInterval);
    EXPORT_FUNC("clearTimeout", JS_clearTimer);
    EXPORT_FUNC("clearInterval", JS_clearTimer);
    EXPORT_FUNC("getMillisecondTimeCounter", JS_millisecondTimeCounter);

#undef EXPORT_FUNC
}

} // namespace infra
GALLIUM_NS_END
