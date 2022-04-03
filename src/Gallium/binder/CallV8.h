#ifndef COCOA_GALLIUM_BINDER_CALLV8_H
#define COCOA_GALLIUM_BINDER_CALLV8_H

#include "Gallium/Gallium.h"
#include "Gallium/binder/Convert.h"

GALLIUM_BINDER_NS_BEGIN

template<typename ...Args>
v8::Local<v8::Value> Invoke(v8::Isolate *isolate, v8::Local<v8::Function> func,
                            v8::Local<v8::Value> recv, Args&& ... args)
{
    v8::EscapableHandleScope scope(isolate);

    int const arg_count = sizeof...(Args);
    // +1 to allocate array for arg_count == 0
    v8::Local<v8::Value> v8_args[arg_count + 1] = {
            to_v8(isolate, std::forward<Args>(args))...
    };

    v8::Local<v8::Value> result;
    bool const is_empty_result = func->Call(isolate->GetCurrentContext(),
                                            recv,
                                            arg_count,
                                            v8_args).ToLocal(&result);
    (void) is_empty_result;
    return scope.Escape(result);
}

template<typename...ArgsT>
v8::Local<v8::Value> InvokeMethod(v8::Isolate *isolate, v8::Local<v8::Object> object,
                                  const std::string& method, ArgsT&&... args)
{
    v8::EscapableHandleScope scope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    v8::Local<v8::Value> member = object->Get(context, to_v8(isolate, method))
                                  .FromMaybe(v8::Local<v8::Value>());
    if (!member->IsFunction())
        return {};
    v8::Local<v8::Value> ret = Invoke(isolate, member.As<v8::Function>(),
                                      object, std::forward<ArgsT>(args)...);
    return scope.Escape(ret);
}

GALLIUM_BINDER_NS_END
#endif //COCOA_GALLIUM_BINDER_CALLV8_H
