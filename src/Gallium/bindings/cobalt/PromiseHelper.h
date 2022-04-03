#ifndef COCOA_GALLIUM_BINDINGS_COBALT_PROMISEHELPER_H
#define COCOA_GALLIUM_BINDINGS_COBALT_PROMISEHELPER_H

#include "Gallium/bindings/cobalt/Exports.h"
GALLIUM_BINDINGS_COBALT_NS_BEGIN

namespace i = ::cocoa::cobalt;

/* A helper structure for asynchronous operations on RenderClient objects */
struct PromiseClosure
{
    using InfoConverter = v8::Local<v8::Value>(*)(v8::Isolate*,
                                                  i::RenderHostCallbackInfo&);

    PromiseClosure(v8::Isolate *isolate, InfoConverter conv);
    ~PromiseClosure();

    static std::shared_ptr<PromiseClosure> New(v8::Isolate *isolate, InfoConverter converter);
    static void HostCallback(i::RenderHostCallbackInfo& info);

    bool rejectIfEssential(i::RenderHostCallbackInfo& info);

    v8::Local<v8::Promise> getPromise();

    template<typename Wrapper, typename T>
    static v8::Local<v8::Value> CreateObjectConverter(v8::Isolate *isolate,
                                                      i::RenderHostCallbackInfo& info)
    {
        return binder::Class<Wrapper>::create_object(isolate, info.GetReturnValue<T>());
    }

    v8::Isolate *isolate_;
    v8::Global<v8::Promise::Resolver> resolver_;
    InfoConverter info_converter_;
};

struct SlotClosure
{
    static std::unique_ptr<SlotClosure> New(v8::Isolate *isolate,
                                            int32_t signal,
                                            const i::co_sp<i::RenderClientObject>& client,
                                            v8::Local<v8::Function> callback,
                                            InfoAcceptor acceptor);

    ~SlotClosure();

    v8::Isolate *isolate_;
    v8::Global<v8::Function> callback_;
    i::co_sp<i::RenderClientObject> client_;
    InfoAcceptor acceptor_;
    uint32_t slot_id_;
};

GALLIUM_BINDINGS_COBALT_NS_END
#endif //COCOA_GALLIUM_BINDINGS_COBALT_PROMISEHELPER_H
