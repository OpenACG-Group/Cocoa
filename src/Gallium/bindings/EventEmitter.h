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
 *     class NotifierWrap : public EventEmitterBase
 *     {
 *     public:
 *       NotifierWrap() {
 *         EmitterDefineEvent("happen", [this] {
 *           auto emit = EmitterWrapAsCallable("happen");
 *           notifier_.SetHappenCallback([emit](int value) {
 *             v8::Isolate *isolate = v8::Isolate::GetCurrent();
 *             v8::HandleScope handle_scope(isolate);
 *             emit({ v8::Number::New(isolate, value) });
 *           });
 *           return 0;
 *         }, [this](uint64_t id) {
 *           notifier_.ClearHappenCallback();
 *         });
 *       }
 *
 *     private:
 *       Notifier notifier_;
 *     }
 * @endcode
 *
 * `NotifierWrap` is exported into the JavaScript with name
 * `Notifier`, and has one event. Supposing it is used in
 * JavaScript like:
 * @code
 *     const emitter = new Notifier();
 *     emitter.addListener('happen', (value) => { ... });
 * @endcode
 *
 * 'addListener()' registers a handler function (anonymous) for the
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
     * `on_set` callback will be called when the number of listeners
     * becomes > 0, and `on_clear` will be called when the number of
     * listeners becomes 0 (all the event listeners are removed).
     * The value of `uint64_t` argument of `on_clear` is the same to
     * the `uint64_t` number returned by the latest `on_set` call.
     * If `on_set` is not present, 0 is the default value.
     */
    void EmitterDefineEvent(const std::string& name,
                            std::function<uint64_t(void)> on_set = {},
                            std::function<void(uint64_t)> on_clear = {});

    /**
     * A callable object is returned. The corresponding event will be emitted
     * when that callable object is called.
     * Returned callable object implicitly retains a `v8::Global` handle
     * (strong reference) of the object itself (`this`). Although `v8::Global`
     * is noncopyable, the returned callable object is copyable, and caller
     * should treat it as a shared reference of `v8::Global`.
     * It does not cache the result; each time `EmitterWrapAsCallable` is called,
     * a new callable object is created.
     */
    std::function<void(const ListenerArgsT&)> EmitterWrapAsCallable(const std::string& name);

    void EmitterSetListener(const std::string& name, v8::Local<v8::Value> func, bool once = false);
    void EmitterDispose();

    //! TSDecl: function addListener(name: string, func: Function): void
    void addListener(const std::string& name, v8::Local<v8::Value> func);

    //! TSDecl: function addOnceListener(name: string, func: Function): void
    void addOnceListener(const std::string& name, v8::Local<v8::Value> func);

    //! TSDecl: function removeListener(name: string, func: Function): boolean
    bool removeListener(const std::string& name, v8::Local<v8::Value> func);

    //! TSDecl: function removeAllListeners(name: string): void
    void removeAllListeners(const std::string& name);

    /**
     * This class will be exported to the JavaScript land as a base class
     * of other event emitters (like `MessagePort`).
     * Set `inherit="EventEmitterBase"` property on the `<class>` element
     * in `Module.xml` file to export this base class's methods.
     *
     * `RegisterClass()` should be called when a new isolate is created by
     * `BindingManager::NotifyIsolateHasCreated()`, which will be called by
     * `RuntimeBase::Initialize()` when the initialization of Isolate is
     * finished.
     */
    static void RegisterClass(v8::Isolate *isolate);

protected:
    /**
     * Implementors return a `v8::Object` which is the corresponding
     * JavaScript object of this object itself (`this`).
     * Usually implementors also inherit `ExportableObjectBase` class,
     * and, in that case, `ExportableObjectBase::GetObjectWeakReference()`
     * could be used to implement this method:
     * @code
     *     v8::Local<v8::Object> impl::OnGetObjectSelf(v8::Isolate *isolate) {
     *       return GetObjectWeakReference().Get(isolate);
     *     }
     * @endcode
     */
    virtual v8::Local<v8::Object> OnGetObjectSelf(v8::Isolate *isolate) = 0;

private:
    struct ListenerData
    {
        ListenerData(bool once_, v8::Isolate *i, v8::Local<v8::Function> f)
            : once(once_), func(i, f), iterating(false), removing(false) {}

        bool once;
        v8::Global<v8::Function> func;
        bool iterating;
        bool removing;
    };
    using ListenersList = std::list<ListenerData>;

    struct EventData
    {
        ListenersList listeners;
        std::function<uint64_t(void)> on_listener_set;
        std::function<void(uint64_t)> on_listener_clear;
        uint64_t on_listener_set_ret;
    };
    using EventsMap = std::unordered_map<std::string, EventData>;

    static ListenersList::iterator ListenerRemoveOrMarkRemoving(
            ListenersList& list, ListenersList::iterator itr);

    static void CallListeners(EventData& event_data, v8::Local<v8::Value> recv,
                              const ListenerArgsT& args);

    bool                    disposed_;
    EventsMap               events_map_;
};

GALLIUM_BINDINGS_NS_END
#endif //COCOA_GALLIUM_BINDINGS_EVENTEMITTER_H
