#include "include/v8.h"

#include "Koi/binder/Convert.h"
#include "Koi/bindings/Base.h"
#include "Koi/bindings/core/Exports.h"
#include "Koi/bindings/core/FdRandomize.h"

KOI_BINDINGS_NS_BEGIN

CoreBinding::CoreBinding()
        : BindingBase("core",
                      "Basic language features for Cocoa JavaScript")
{
    FDLRInitialize();
}

CoreBinding::~CoreBinding()
{
    FDLRCollectAndSweep();
}

void CoreBinding::setInstanceProperties(v8::Local<v8::Object> instance)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    instance->Set(context, binder::to_v8(isolate, "args"), GetEscapableArgs()).Check();
}

KOI_BINDINGS_NS_END
