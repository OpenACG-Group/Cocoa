#include "Core/Journal.h"
#include "Koi/GlobalIsolateGuard.h"
#include "Koi/Runtime.h"
#include "Koi/binder/Convert.h"
KOI_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Koi)

namespace {

void perIsolateMessageListener(v8::Local<v8::Message> message,
                               v8::Local<v8::Value> except)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    Runtime *rt = Runtime::GetBareFromIsolate(isolate);
    CHECK(rt);

    switch (message->ErrorLevel())
    {
    case v8::Isolate::MessageErrorLevel::kMessageWarning:
    {
        auto script = binder::from_v8<std::string>(isolate, message->GetScriptResourceName());
        auto content = binder::from_v8<std::string>(isolate, message->Get());
        QLOG(LOG_WARNING, "%fg<hl>(Isolate)%reset Warning from script {} line {}:", script,
             message->GetLineNumber(rt->context()).FromMaybe(-1));
        QLOG(LOG_WARNING, "  {}", content);
        break;
    }
    case v8::Isolate::MessageErrorLevel::kMessageError:
    {
        v8::Local<v8::String> string = except->ToString(ctx).ToLocalChecked();
        QLOG(LOG_ERROR, "An uncaught exception was raised from JavaScript land:");
        QLOG(LOG_ERROR, "  Exception: {}", binder::from_v8<std::string>(isolate, string));
        if (rt->getIntrospect())
            rt->getIntrospect()->notifyUncaughtException(except);
        break;
    }
    }
}

} // namespace anonymous

GlobalIsolateGuard::GlobalIsolateGuard(const std::shared_ptr<Runtime>& rt)
    : fRuntime(rt)
    , fIsolate(rt->isolate())
{
    fIsolate->AddMessageListenerWithErrorLevel(perIsolateMessageListener,
                                               v8::Isolate::MessageErrorLevel::kMessageError |
                                               v8::Isolate::MessageErrorLevel::kMessageWarning);
}

GlobalIsolateGuard::~GlobalIsolateGuard()
{
}

KOI_NS_END
