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

#include "fmt/format.h"

#include "Gallium/bindings/EventEmitter.h"
#include "Gallium/RuntimeBase.h"
GALLIUM_BINDINGS_NS_BEGIN

EventEmitterBase::EventEmitterBase()
    : disposed_(false)
{
}

void EventEmitterBase::EmitterDefineEvent(const std::string& name,
                                          std::function<uint64_t(void)> on_set,
                                          std::function<void(uint64_t)> on_clear)
{
    CHECK(events_map_.count(name) == 0 && "Duplicated event definition");
    events_map_[name] = EventData{
        .on_listener_set = std::move(on_set),
        .on_listener_clear = std::move(on_clear),
        .on_listener_set_ret = 0
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
    EventData *event_data = &events_map_[name];

    return [isolate, g_self_sp, event_data](const ListenerArgsT& args) {
        CHECK(isolate == v8::Isolate::GetCurrent() && "Different v8::Isolate");
        v8::HandleScope handle_scope(isolate);
        v8::Local<v8::Object> self = g_self_sp->Get(isolate);
        EventEmitterBase::CallListeners(*event_data, self, args);
    };
}

void EventEmitterBase::CallListeners(EventData& event_data,
                                     v8::Local<v8::Value> recv,
                                     const ListenerArgsT& args)
{
    CHECK(args.size() < INT_MAX);

    int args_size = static_cast<int>(args.size());

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    RuntimeBase *rt = RuntimeBase::FromIsolate(isolate);

    ListenersList& listeners = event_data.listeners;
    auto itr = listeners.begin();
    while (itr != listeners.end())
    {
        CHECK(!itr->func.IsEmpty());
        v8::Local<v8::Function> func = itr->func.Get(isolate);
        // FIXME(sora): Is it a safe conversion (cast away `const` qualifier)?
        //              Why V8 does not make it a const parameter?
        auto *ptr_args = const_cast<v8::Local<v8::Value>*>(args.data());

        // User's callback function may call `removeListener()` or
        // `removeAllListeners()` on the listener list on which we are
        // iterating, and that will make the iterator invalid.
        // Setting `iterating = true` lets `removeListener()` and
        // `removeAllListeners()` know the iterator is being used and should
        // not be removed immediately. Instead, set `removing` flag.
        itr->iterating = true;

        v8::TryCatch try_catch(isolate);
        if (func->Call(ctx, recv, args_size, ptr_args).IsEmpty())
        {
            CHECK(try_catch.HasCaught());
            rt->ReportUncaughtExceptionInCallback(try_catch);
        }

        itr->iterating = false;

        if (itr->once || itr->removing)
            itr = listeners.erase(itr);
        else
            itr++;
    }

    if (listeners.empty() && event_data.on_listener_clear)
        event_data.on_listener_clear(event_data.on_listener_set_ret);
}

void EventEmitterBase::EmitterSetListener(const std::string &name,
                                          v8::Local<v8::Value> func,
                                          bool once)
{
    if (disposed_)
        g_throw(Error, "Event emitter has been disposed (closed)");

    if (events_map_.count(name) == 0)
        g_throw(Error, fmt::format("Undefined event name `{}`", name));

    if (!func->IsFunction())
        g_throw(TypeError, "Argument `func` must be a Function");

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    EventData& event_data = events_map_[name];
    event_data.listeners.emplace_back(once, isolate, func.As<v8::Function>());

    // Call the `on_listener_set` callback if it is the
    // first listener on this event.
    if (event_data.on_listener_set && event_data.listeners.size() == 1)
        event_data.on_listener_set_ret = event_data.on_listener_set();
}

void EventEmitterBase::EmitterDispose()
{
    disposed_ = true;
}

void EventEmitterBase::addListener(const std::string& name, v8::Local<v8::Value> func)
{
    EmitterSetListener(name, func, false);
}

void EventEmitterBase::addOnceListener(const std::string& name, v8::Local<v8::Value> func)
{
    EmitterSetListener(name, func, true);
}

auto EventEmitterBase::ListenerRemoveOrMarkRemoving(
        ListenersList& list, ListenersList::iterator itr) -> ListenersList::iterator
{
    if (itr->iterating)
    {
        itr->removing = true;
        itr++;
    }
    else
    {
        itr = list.erase(itr);
    }
    return itr;
}

bool EventEmitterBase::removeListener(const std::string& name, v8::Local<v8::Value> func)
{
    if (events_map_.count(name) == 0)
        g_throw(Error, fmt::format("Undefined event name `{}`", name));
    if (!func->IsFunction())
        g_throw(TypeError, "Argument `func` must be a Function");

    EventData& event_data = events_map_[name];
    if (event_data.listeners.empty())
        return false;

    bool found = false;
    auto itr = event_data.listeners.begin();
    while (itr != event_data.listeners.end())
    {
        if (itr->func == func)
        {
            found = true;
            itr = ListenerRemoveOrMarkRemoving(event_data.listeners, itr);
        }
        else
        {
            itr++;
        }
    }

    if (event_data.listeners.empty() && event_data.on_listener_clear)
        event_data.on_listener_clear(event_data.on_listener_set_ret);

    return found;
}

void EventEmitterBase::removeAllListeners(const std::string &name)
{
    if (events_map_.count(name) == 0)
        g_throw(Error, fmt::format("Undefined event name `{}`", name));

    EventData& event_data = events_map_[name];
    if (event_data.listeners.empty())
        return;

    auto itr = event_data.listeners.begin();
    while (itr != event_data.listeners.end())
        itr = ListenerRemoveOrMarkRemoving(event_data.listeners, itr);

    if (event_data.listeners.empty() && event_data.on_listener_clear)
        event_data.on_listener_clear(event_data.on_listener_set_ret);
}

void EventEmitterBase::RegisterClass(v8::Isolate *isolate)
{
    binder::Class<EventEmitterBase>(isolate)
        .set("addListener", &EventEmitterBase::addListener)
        .set("addOnceListener", &EventEmitterBase::addOnceListener)
        .set("removeListener", &EventEmitterBase::removeListener)
        .set("removeAllListeners", &EventEmitterBase::removeAllListeners);
}

GALLIUM_BINDINGS_NS_END
