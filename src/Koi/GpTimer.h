#ifndef COCOA_GPTIMER_H
#define COCOA_GPTIMER_H

#include "include/v8.h"

#include "Core/EventSource.h"
#include "Koi/KoiBase.h"
#include "Koi/ResourceObject.h"
KOI_NS_BEGIN

class GpTimer : public TimerSource,
                 public ResourceObject
{
    RESOURCE_OBJECT(GpTimer)

public:
    GpTimer(Runtime *runtime, v8::Local<v8::Function> callback, v8::Local<v8::Object> _this);
    ~GpTimer() override;

private:
    void release() override;
    KeepInLoop timerDispatch() override;

    v8::Global<v8::Function> fCallback;
    v8::Global<v8::Object>   fThis;
};

KOI_NS_END
#endif //COCOA_GPTIMER_H
