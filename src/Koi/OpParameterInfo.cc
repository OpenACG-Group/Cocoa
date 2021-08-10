#include "Koi/Ops.h"
#include "Koi/Runtime.h"
KOI_NS_BEGIN

OpParameterInfo::OpParameterInfo(v8::Isolate *isolate,
                                         v8::Local<v8::Object> object,
                                         StorageType type)
    : fIsolate(isolate),
      fRuntime(reinterpret_cast<Runtime*>(fIsolate->GetData(ISOLATE_DATA_SLOT_RUNTIME_PTR))),
      fType(type)
{
    if (fType == StorageType::kLocal)
        fLocal = object;
    else
        fPersistent.Reset(fIsolate, object);
}

OpParameterInfo::OpParameterInfo(v8::Isolate *isolate, v8::Local<v8::Object> object,
                                 v8::Local<v8::Promise::Resolver> promise, StorageType type)
    : fIsolate(isolate),
      fRuntime(reinterpret_cast<Runtime*>(fIsolate->GetData(ISOLATE_DATA_SLOT_RUNTIME_PTR))),
      fType(type),
      fPromise(fIsolate, promise)
{
    if (fType == StorageType::kLocal)
        fLocal = object;
    else
        fPersistent.Reset(fIsolate, object);
}

OpParameterInfo::~OpParameterInfo()
{
    fPersistent.Reset();
}

v8::Local<v8::Object> OpParameterInfo::get()
{
    if (fType == StorageType::kLocal)
        return fLocal;
    else
        return fPersistent.Get(fIsolate);
}

v8::Local<v8::Value> OpParameterInfo::operator[](const std::string& member)
{
    auto key = v8::String::NewFromUtf8(fIsolate, member.c_str()).ToLocalChecked();
    return this->operator[](key);
}

v8::Local<v8::Value> OpParameterInfo::operator[](v8::Local<v8::String> key)
{
    auto object = this->get();
    auto context = this->context();
    return object->Get(context, key).ToLocalChecked();
}

KOI_NS_END
