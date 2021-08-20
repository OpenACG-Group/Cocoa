#include "Koi/binder/ThrowExcept.h"
#include "Koi/binder/Convert.h"
KOI_BINDER_NS_BEGIN

v8::Local<v8::Value> throw_(v8::Isolate* isolate, std::string_view str)
{
    return isolate->ThrowException(to_v8(isolate, str));
}

v8::Local<v8::Value> throw_(v8::Isolate* isolate, std::string_view str,
                            v8::Local<v8::Value> (*builder)(v8::Local<v8::String>))
{
    return isolate->ThrowException(builder(to_v8(isolate, str)));
}

KOI_BINDER_NS_END
