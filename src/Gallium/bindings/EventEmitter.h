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

#ifndef COCOA_GALLIUM_BINDINGS_EVENTEMITTER_H
#define COCOA_GALLIUM_BINDINGS_EVENTEMITTER_H

#include <unordered_map>
#include <list>

#include "include/v8.h"

#include "Core/Errors.h"
#include "Gallium/Gallium.h"
#include "Gallium/binder/Class.h"
#include "Gallium/bindings/ExportableObjectBase.h"
GALLIUM_BINDINGS_NS_BEGIN

#define EVENT_EMITTER_LISTENER_SETTER(name) \
    void on##name(v8::Local<v8::Value> func) { EmitterSetListener(#name, func); }

/**
 * Base class of classes that waits asynchronous events from event
 * loop and calls the corresponding JavaScript functions to handle
 * coming events.
 *
 * Supposing `NotifierWrap` is a C++ exported class which can emit
 * events, it is declared like:
 * @code
 *     class NotifierWrap : public EventEmitter<Notifier>
 *     {
 *     public:
 *         EVENT_EMITTER_LISTENER_SETTER(Error)
 *         EVENT_EMITTER_LISTENER_SETTER(Happen)
 *     }
 * @endcode
 *
 * `NotifierWrap` is exported into the JavaScript with name
 * `Notifier`, and has 2 events. Supposing it is used in JavaScript
 * like:
 * @code
 *     const emitter = new Notifier();
 *     emitter.onHappen(() => { ... });
 *     emitter.onError(() => { ... });
 * @endcode
 *
 * `onHappen()` and `onError()` register 2 handler functions for the
 * corresponding event, which are called event listeners. Implementors
 * (subclasses) should make sure the following principles:
 *
 * 1. The event loop should NOT exit if there are any event listeners,
 *    unless the EventEmitter will NOT emit events in the future (e.g.
 *    it is disposed or closed).
 *
 * 2. If an EventEmitter have no event listeners, and the coming of events
 *    does NOT make any differences to JavaScript execution, it is a "phantom
 *    event emitter". Those EventEmitters should NOT block the event loop
 *    (implementors may use `uv_ref()` and `uv_unref()` to implement this).
 */
class EventEmitterBase
{
public:
    using ListenerArgsT = std::vector<v8::Local<v8::Value>>;

    explicit EventEmitterBase();
    virtual ~EventEmitterBase() = default;

    /**
     * Implementors should call this to define (register) its events.
     */
    void EmitterDefineEvent(const std::string& name,
                            std::function<void(void)> on_set = {},
                            std::function<void(void)> on_clear = {});

    /**
     * A callable object is returned. The corresponding event will be emitted
     * when that callable object is called.
     * Returned callable object implicitly retains a `v8::Global<>` handle
     * (strong reference) of the object itself (`this`). Although `v8::Global<>`
     * is noncopyable, the returned callable object is copyable, and caller
     * should treat it as a shared reference of `v8::Global<>`.
     * It does not cache the result; each time `EmitterWrapAsCallable` is called,
     * a new callable object is created.
     */
    std::function<void(const ListenerArgsT&)> EmitterWrapAsCallable(const std::string& name);

    void EmitterSetListener(const std::string& name, v8::Local<v8::Value> func);
    void EmitterDispose();

protected:
    // Called when the first event listener appears
    virtual void OnActivateEventEmitter() {}

    // Called when there is no any event listeners
    virtual void OnDeactivateEventEmitter() {}

    virtual v8::Local<v8::Object> OnGetObjectSelf(v8::Isolate *isolate) = 0;

private:
    using ListenersList = std::list<v8::Global<v8::Function>>;
    struct EventData
    {
        ListenersList listeners;
        std::function<void(void)> on_listener_set;
        std::function<void(void)> on_listener_clear;
    };
    using EventsMap = std::unordered_map<std::string, EventData>;

    static void CallListeners(ListenersList& listeners, v8::Local<v8::Value> recv,
                              const ListenerArgsT& args);

    bool                    disposed_;
    EventsMap               events_map_;
};

template<typename T>
class EventEmitter : public EventEmitterBase
{
public:
    EventEmitter() = default;
    ~EventEmitter() override = default;

private:
};

GALLIUM_BINDINGS_NS_END
#endif //COCOA_GALLIUM_BINDINGS_EVENTEMITTER_H
