#ifndef COCOA_GALLIUM_BINDINGS_GLAMOR_PROMISEHELPER_H
#define COCOA_GALLIUM_BINDINGS_GLAMOR_PROMISEHELPER_H

#include <functional>

#include "Gallium/bindings/glamor/Exports.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

namespace i = ::cocoa::glamor;

/* A helper structure for asynchronous operations on RenderClient objects */
struct PromiseClosure
{
    using InfoConverter = std::function<v8::Local<v8::Value>(v8::Isolate*, i::RenderHostCallbackInfo&)>;

    PromiseClosure(v8::Isolate *isolate, InfoConverter conv);
    ~PromiseClosure();

    static std::shared_ptr<PromiseClosure> New(v8::Isolate *isolate, const InfoConverter& converter);
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
                                            const i::Shared<i::RenderClientObject>& client,
                                            v8::Local<v8::Function> callback,
                                            InfoAcceptor acceptor);

    ~SlotClosure();

    v8::Isolate *isolate_;
    v8::Global<v8::Function> callback_;
    i::Shared<i::RenderClientObject> client_;
    InfoAcceptor acceptor_;
    uint32_t slot_id_;
    int32_t signal_code_;
};

GALLIUM_BINDINGS_GLAMOR_NS_END
#endif //COCOA_GALLIUM_BINDINGS_GLAMOR_PROMISEHELPER_H
