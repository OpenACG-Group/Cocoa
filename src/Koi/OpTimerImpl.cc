#include "Koi/Ops.h"
#include "Koi/Runtime.h"
#include "Koi/GpTimer.h"
KOI_NS_BEGIN

OpHandlerImpl(op_timer_create)
{
    using namespace js_literal;

    v8::Local<v8::Context> context = param.runtime()->context();
    if (param.get()->Has(context, "callback"_js).IsNothing())
        return -OP_EINVARG;
    auto callback = v8::Local<v8::Function>::Cast(param["callback"]);

    v8::Local<v8::Object> _this;
    if (!param.get()->Has(context, "_this"_js).IsNothing())
    {
        v8::Local<v8::Value> valueThis = param["_this"];
        if (valueThis->IsObject())
            _this = valueThis->ToObject(context).ToLocalChecked();
    }

    return param.runtime()->resourcePool().resourceGen<GpTimer>(param.runtime(),
                                                                 callback,
                                                                 _this)->getRID();
}

OpHandlerImpl(op_timer_ctl)
{
    using namespace js_literal;

    v8::Local<v8::Context> context = param.runtime()->context();
    v8::Local<v8::Object> args = param.get();

    RID rid = OpExtractRIDFromArgs(param);
    if (rid < 0)
        return rid;
    ResourceDescriptorPool::ScopedAcquire<GpTimer> timer(param.runtime()->resourcePool(), rid);
    if (!timer.valid())
        return -OP_EBADRID;

    if (args->Has(context, "verb"_js).IsNothing())
        return -OP_EINVARG;

    int32_t verb = param["verb"]->ToInt32(context).ToLocalChecked()->Value();
    if (verb >= static_cast<int8_t>(OpTimerCtlVerb::kMaxEnum))
        return -OP_EINVARG;

    int64_t interval, timeout;
    switch (static_cast<OpTimerCtlVerb>(verb))
    {
    case OpTimerCtlVerb::kSetTimeout:
        if (args->Has(context, "timeout"_js).IsNothing())
            return -OP_EINVARG;
        timeout = param["timeout"]->ToInteger(context).ToLocalChecked()->Value();
        timer->startTimer(timeout, 0);
        break;

    case OpTimerCtlVerb::kSetInterval:
        if (args->Has(context, "interval"_js).IsNothing())
            return -OP_EINVARG;
        interval = param["interval"]->ToInteger(context).ToLocalChecked()->Value();
        timer->startTimer(interval, interval);
        break;

    case OpTimerCtlVerb::kStop:
        timer->stopTimer();
        break;

    case OpTimerCtlVerb::kMaxEnum:
        return -OP_EINVARG;
    }
    return OP_SUCCESS;
}

KOI_NS_END
