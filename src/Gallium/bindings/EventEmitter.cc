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

#include "Gallium/bindings/EventEmitter.h"
#include "Gallium/RuntimeBase.h"
GALLIUM_BINDINGS_NS_BEGIN

EventEmitterBase::EventEmitterBase()
    : disposed_(false)
{
}

void EventEmitterBase::EmitterDefineEvent(const std::string& name,
                                          std::function<void(void)> on_set,
                                          std::function<void(void)> on_clear)
{
    CHECK(events_map_.count(name) == 0 && "Duplicated event definition");
    events_map_[name] = EventData{
        .on_listener_set = std::move(on_set),
        .on_listener_clear = std::move(on_clear)
    };
}

std::function<void(const EventEmitterBase::ListenerArgsT&)>
EventEmitterBase::EmitterWrapAsCallable(const std::string &name)
{
    CHECK(events_map_.count(name) > 0 && "Undefined event name");

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Object> self = OnGetObjectSelf(isolate);
    CHECK(!self.IsEmpty());
    auto g_self_sp = std::make_shared<v8::Global<v8::Object>>(isolate, self);

    // Listeners only can be added to `events_map_`, the pointer is
    // valid during the whole lifetime of this object.
    ListenersList *listeners = &events_map_[name].listeners;

    return [isolate, g_self_sp, listeners](const ListenerArgsT& args) {
        CHECK(isolate == v8::Isolate::GetCurrent() && "Different v8::Isolate");
        v8::HandleScope handle_scope(isolate);
        v8::Local<v8::Object> self = g_self_sp->Get(isolate);
        EventEmitterBase::CallListeners(*listeners, self, args);
    };
}

void EventEmitterBase::CallListeners(ListenersList& listeners,
                                     v8::Local<v8::Value> recv,
                                     const ListenerArgsT& args)
{
    CHECK(args.size() < INT_MAX);

    int args_size = static_cast<int>(args.size());

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    RuntimeBase *rt = RuntimeBase::FromIsolate(isolate);

    for (const v8::Global<v8::Function>& g_func : listeners)
    {
        CHECK(!g_func.IsEmpty());
        v8::Local<v8::Function> func = g_func.Get(isolate);
        // FIXME(sora): Is it a safe conversion (cast away `const` qualifier)?
        //              Why V8 does not make it a const parameter?
        auto *ptr_args = const_cast<v8::Local<v8::Value>*>(args.data());

        v8::TryCatch try_catch(isolate);
        if (func->Call(ctx, recv, args_size, ptr_args).IsEmpty())
        {
            CHECK(try_catch.HasCaught());
            rt->ReportUncaughtExceptionInCallback(try_catch);
        }
    }
}

void EventEmitterBase::EmitterSetListener(const std::string &name,
                                          v8::Local<v8::Value> func)
{
    if (disposed_)
        g_throw(Error, "Event emitter has been disposed (closed)");

    if (events_map_.count(name) == 0)
        g_throw(Error, "Undefined event name");

    if (!func->IsFunction())
        g_throw(TypeError, "Argument `func` must be a Function");

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    EventData& event_data = events_map_[name];
    event_data.listeners.emplace_back(isolate, func.As<v8::Function>());

    if (event_data.on_listener_set)
        event_data.on_listener_set();

    if (event_data.listeners.size() == 1)
        OnActivateEventEmitter();
}

void EventEmitterBase::EmitterDispose()
{
    disposed_ = true;
}

GALLIUM_BINDINGS_NS_END
