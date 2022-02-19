#ifndef COCOA_GLOBALISOLATEGUARD_H
#define COCOA_GLOBALISOLATEGUARD_H

#include <list>
#include <map>

#include "include/v8.h"

#include "Koi/KoiBase.h"
KOI_NS_BEGIN

class Runtime;

class GlobalIsolateGuard
{
public:
    explicit GlobalIsolateGuard(const std::shared_ptr<Runtime>& rt);
    ~GlobalIsolateGuard();

    koi_nodiscard inline v8::Isolate *getIsolate() const {
        return fIsolate;
    }

    koi_nodiscard inline std::shared_ptr<Runtime> getRuntime() const {
        return fRuntime.lock();
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

    std::weak_ptr<Runtime> fRuntime;
    v8::Isolate           *fIsolate;
    PromiseValueList       fRejectPromises;
};

KOI_NS_END
#endif //COCOA_GLOBALISOLATEGUARD_H
