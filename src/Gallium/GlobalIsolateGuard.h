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

#ifndef COCOA_GALLIUM_GLOBALISOLATEGUARD_H
#define COCOA_GALLIUM_GLOBALISOLATEGUARD_H

#include <list>
#include <map>

#include "include/v8.h"

#include "Gallium/Gallium.h"
GALLIUM_NS_BEGIN

class Runtime;

class GlobalIsolateGuard
{
public:
    explicit GlobalIsolateGuard(Runtime *rt);
    ~GlobalIsolateGuard();

    g_nodiscard inline v8::Isolate *getIsolate() const {
        return isolate_;
    }

    g_nodiscard inline Runtime *getRuntime() const {
        return runtime_;
    }

    void pushMaybeUnhandledRejectPromise(v8::Local<v8::Promise> promise, v8::Local<v8::Value> value);
    void removeMaybeUnhandledRejectPromise(v8::Local<v8::Promise> promise);
    void performUnhandledRejectPromiseCheck();
    void reportUncaughtExceptionFromCallback(const v8::TryCatch& caught);

private:
    struct PromiseWithValue
    {
        PromiseWithValue(v8::Isolate *isolate, v8::Local<v8::Promise> promise,
                         v8::Local<v8::Value> value)
            : promise(isolate, promise), value(isolate, value) {}
        PromiseWithValue(const PromiseWithValue&) = delete;
        PromiseWithValue(PromiseWithValue&& rhs) noexcept
            : promise(std::move(rhs.promise)) , value(std::move(rhs.value)) {}
        ~PromiseWithValue() {
            promise.Reset();
            value.Reset();
        }

        bool operator==(const PromiseWithValue& pv) const {
            return (promise == pv.promise);
        }
        bool operator==(const v8::Local<v8::Promise>& promise_) const {
            return (promise == promise_);
        }
        v8::Global<v8::Promise> promise;
        v8::Global<v8::Value> value;
    };

    using PromiseValueList = std::list<PromiseWithValue>;

    Runtime               *runtime_;
    v8::Isolate           *isolate_;
    PromiseValueList       reject_promises_;
};

GALLIUM_NS_END
#endif //COCOA_GALLIUM_GLOBALISOLATEGUARD_H
