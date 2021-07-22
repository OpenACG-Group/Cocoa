#ifndef COCOA_GPRTIMER_H
#define COCOA_GPRTIMER_H

#include "include/v8.h"

#include "Core/EventSource.h"
#include "Scripter/ScripterBase.h"
#include "Scripter/ResourceObject.h"
SCRIPTER_NS_BEGIN

class GprTimer : public TimerSource,
                 public ResourceObject
{
    RESOURCE_OBJECT(GprTimer)

public:
    GprTimer(Runtime *runtime, v8::Local<v8::Function> callback);
    ~GprTimer() override;

private:
    void release() override;
    KeepInLoop timerDispatch() override;

    v8::Global<v8::Function> fCallback;
};

SCRIPTER_NS_END
#endif //COCOA_GPRTIMER_H
