#ifndef COCOA_CALLV8_H
#define COCOA_CALLV8_H

#include "Koi/KoiBase.h"
#include "Koi/binder/Convert.h"

KOI_BINDER_NS_BEGIN

template<typename ...Args>
v8::Local<v8::Value> call_v8(v8::Isolate *isolate, v8::Local<v8::Function> func,
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

KOI_BINDER_NS_END
#endif //COCOA_CALLV8_H
