#include "Core/EventSource.h"
#include "Core/EventLoop.h"
#include "Core/Journal.h"
#include "Koi/binder/CallV8.h"
#include "Koi/bindings/core/Exports.h"

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Koi.bindings)

KOI_BINDINGS_NS_BEGIN

class DelayTimer : public TimerSource
{
public:
    DelayTimer(uint64_t timeout,
               v8::Isolate *isolate,
               v8::Local<v8::Promise::Resolver> resolver)
            : TimerSource(EventLoop::Instance()),
              fIsolate(isolate),
              fResolve(fIsolate, resolver)
    {
        startTimer(timeout);
    }
    ~DelayTimer() override
    {
        fResolve.Reset();
    }

private:
    KeepInLoop timerDispatch() override
    {
        v8::Local<v8::Promise::Resolver> resolve = fResolve.Get(fIsolate);
        resolve->Resolve(fIsolate->GetCurrentContext(), v8::Null(fIsolate))
                .Check();
        delete this;
        return KeepInLoop::kDeleted;
    }

    v8::Isolate *fIsolate;
    v8::Global<v8::Promise::Resolver>   fResolve;
};

v8::Local<v8::Value> coreDelay(uint64_t timeout)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto resolver = v8::Promise::Resolver::New(isolate->GetCurrentContext()).ToLocalChecked();

    /* The object will be deleted in DelayTimer::timerDispatch(). */
    new DelayTimer(timeout, isolate, resolver);
    return resolver->GetPromise();
}

class CoreTimer : public TimerSource
{
public:
    enum class State
    {
        kIdle,
        kRepeat,
        kWaitTimeout
    };

    CoreTimer()
            : TimerSource(EventLoop::Instance())
            , fState(State::kIdle)
            , fGarbageCollected(false) { }
    ~CoreTimer() override
    {
        QLOG(LOG_DEBUG, "CoreTimer@{} was destructed", fmt::ptr(this));
        fCallback.Reset();
    }

    void setInterval(uint64_t timeout, int64_t repeat, v8::Local<v8::Value> cbValue)
    {
        v8::Isolate *isolate = v8::Isolate::GetCurrent();
        if (!cbValue->IsFunction())
        {
            binder::throw_(isolate, "Callback should be a function", v8::Exception::TypeError);
            return;
        }
        if (!fCallback.IsEmpty())
            fCallback.Reset();
        fCallback = v8::Global<v8::Function>(isolate, v8::Local<v8::Function>::Cast(cbValue));
        TimerSource::startTimer(timeout, repeat);
        fState = repeat > 0 ? State::kRepeat : State::kWaitTimeout;
    }

    void stop()
    {
        fState = State::kIdle;
        TimerSource::stopTimer();
        fCallback.Reset();
    }

    void markGarbageCollected()
    {
        fGarbageCollected = true;
    }

    koi_nodiscard State inline getState() const
    {
        return fState;
    }

private:
    KeepInLoop timerDispatch() override
    {
        if (fCallback.IsEmpty())
        {
            QLOG(LOG_WARNING, "A timer was dispatched but the callback function is empty");
            return KeepInLoop::kNo;
        }
        v8::Isolate *isolate = v8::Isolate::GetCurrent();
        v8::Local<v8::Value> result = binder::Invoke(isolate,
                                                     fCallback.Get(isolate),
                                                     isolate->GetCurrentContext()->Global());

        if (fState == State::kWaitTimeout)
        {
            if (fGarbageCollected)
            {
                delete this;
                return KeepInLoop::kDeleted;
            }
            fState = State::kIdle;
            return KeepInLoop::kNo;
        }

        KeepInLoop keep = KeepInLoop::kYes;
        if (result.IsEmpty())
            keep = KeepInLoop::kNo;
        else if (result->IsBoolean())
            keep = result->ToBoolean(isolate)->Value() ? KeepInLoop::kYes : KeepInLoop::kNo;
        if (keep == KeepInLoop::kNo && fGarbageCollected)
        {
            delete this;
            return KeepInLoop::kDeleted;
        }
        return keep;
    }

    State fState;
    bool fGarbageCollected;
    v8::Global<v8::Function> fCallback;
};


CoreTimerProxy::CoreTimerProxy() : fAttachedTimer(new CoreTimer()) {}
CoreTimerProxy::~CoreTimerProxy()
{
    QLOG(LOG_DEBUG, "TimerProxy@{} was destructed", fmt::ptr(this));
    switch (fAttachedTimer->getState())
    {
    case CoreTimer::State::kIdle:
        delete fAttachedTimer;
        break;
    case CoreTimer::State::kWaitTimeout:
    case CoreTimer::State::kRepeat:
        fAttachedTimer->markGarbageCollected();
        break;
    }
}

binder::Class<CoreTimerProxy> CoreTimerProxy::GetClass()
{
    return binder::Class<CoreTimerProxy>(v8::Isolate::GetCurrent())
            .constructor<>()
            .set("setInterval", &CoreTimerProxy::setInterval)
            .set("setTimeout", &CoreTimerProxy::setTimeout)
            .set("stop", &CoreTimerProxy::stop);
}

void CoreTimerProxy::setInterval(uint64_t timeout, int64_t repeat, v8::Local<v8::Value> cbValue)
{
    fAttachedTimer->setInterval(timeout, repeat, cbValue);
}

void CoreTimerProxy::setTimeout(uint64_t timeout, v8::Local<v8::Value> cbValue)
{
    fAttachedTimer->setInterval(timeout, 0, cbValue);
}

void CoreTimerProxy::stop()
{
    fAttachedTimer->stop();
}

KOI_BINDINGS_NS_END
