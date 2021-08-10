#include "Core/Journal.h"
#include "Koi/KoiBase.h"
#include "Koi/GpTimer.h"
#include "Koi/Runtime.h"
KOI_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Koi)

GpTimer::GpTimer(Runtime *runtime, v8::Local<v8::Function> callback, v8::Local<v8::Object> _this)
    : TimerSource(runtime->eventLoop()),
      ResourceObject(runtime),
      fCallback(runtime->isolate(), callback),
      fThis(runtime->isolate(), _this)
{
    if (fThis.IsEmpty())
        fThis = v8::Global<v8::Object>(runtime->isolate(), runtime->context()->Global());
}

GpTimer::~GpTimer()
{
    fCallback.Reset();
}

void GpTimer::release()
{
    TimerSource::stopTimer();
    fCallback.Reset();
}

KeepInLoop GpTimer::timerDispatch()
{
    v8::Isolate *isolate = runtime()->isolate();
    v8::HandleScope scope(isolate);

    v8::Local<v8::Context> context = runtime()->context();
    v8::Local<v8::Function> func = fCallback.Get(isolate);

    v8::Local<v8::Value> args[] = {v8::Integer::New(isolate, ResourceObject::getRID())};
    v8::Local<v8::Value> retValue;
    if (!func->Call(context, fThis.Get(isolate), 1, args).ToLocal(&retValue))
    {
        LOGF(LOG_DEBUG, "Failed in calling timer handler (RID:{})", getRID())
        return KeepInLoop::kNo;
    }
    return KeepInLoop::kYes;
}

KOI_NS_END
