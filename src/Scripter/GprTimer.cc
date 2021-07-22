#include "Scripter/ScripterBase.h"
#include "Scripter/GprTimer.h"
#include "Scripter/Runtime.h"
SCRIPTER_NS_BEGIN

GprTimer::GprTimer(Runtime *runtime, v8::Local<v8::Function> callback)
    : TimerSource(runtime->eventLoop()),
      ResourceObject(runtime),
      fCallback(runtime->isolate(), callback)
{
}

GprTimer::~GprTimer()
{
    fCallback.Reset();
}

void GprTimer::release()
{
    TimerSource::stopTimer();
    fCallback.Reset();
}

KeepInLoop GprTimer::timerDispatch()
{
    v8::Isolate *isolate = runtime()->isolate();

    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope scope(isolate);
    v8::Context::Scope contextScope(runtime()->context());

    v8::Local<v8::Context> context = runtime()->context();
    v8::Local<v8::Function> func = fCallback.Get(isolate);

    v8::Local<v8::Value> args[] = {v8::Integer::New(isolate, ResourceObject::getRID())};

    func->Call(context, context->Global(), 1, args).ToLocalChecked();
    return KeepInLoop::kYes;
}

SCRIPTER_NS_END
