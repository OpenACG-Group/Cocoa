/**
 * This file is part of Cocoa.
 *
 * Cocoa is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * Cocoa is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cocoa. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef COCOA_GALLIUM_BINDINGS_GLAMOR_PROMISEHELPER_H
#define COCOA_GALLIUM_BINDINGS_GLAMOR_PROMISEHELPER_H

#include <functional>

#include "Core/EnumClassBitfield.h"
#include "Glamor/PresentThread.h"
#include "Gallium/bindings/glamor/Exports.h"
#include "Gallium/bindings/EventEmitter.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

using SignalArgsVector = std::vector<v8::Local<v8::Value>>;
using SignalArgsConverter = std::function<SignalArgsVector(v8::Isolate*, gl::PresentSignalArgs&)>;

using InvokeResultConverter = std::function<v8::Local<v8::Value>(
        v8::Isolate*, gl::PresentRemoteCallReturn&)>;

template<typename RealT, typename CastT, bool CreateObj = false, bool ExtractValue = false>
struct SignalArgsCast
{
    using real_type = RealT;
    using cast_type = CastT;
    constexpr static bool create_obj = CreateObj;
    constexpr static bool extract_value = ExtractValue;
};

template<typename T>
using NoCast = SignalArgsCast<T, T>;

template<typename T>
using AutoEnumCast = SignalArgsCast<T, typename std::underlying_type<T>::type>;

template<typename ObjT, typename WrapperT>
using CreateObjCast = SignalArgsCast<ObjT, WrapperT, true>;

template<typename Enum, typename Target = typename Bitfield<Enum>::T>
using EnumBitfieldCast = SignalArgsCast<Bitfield<Enum>, Target, false, true>;

namespace acceptor_traits {

template<typename T>
v8::Local<v8::Value> ConvertGeneric(v8::Isolate *isolate,
                                    gl::PresentSignalArgs& info,
                                    size_t index)
{
    if constexpr (T::extract_value)
    {
        static_assert(T::real_type::kHasExtractValue,
                "Class must have signature CO_CLASS_HAS_EXTRACT_VALUE");

        static_assert(!T::create_obj, "Illegal usage of SignalArgsCast<...>");

        return binder::to_v8(isolate, static_cast<typename T::cast_type>(
                    info.Get<typename T::real_type>(index).value()));
    }
    else if constexpr (T::create_obj)
    {
        return binder::NewObject<typename T::cast_type>(isolate,
            info.Get<typename T::real_type>(index));
    }
    else if constexpr (std::is_same<typename T::real_type, typename T::cast_type>::value)
    {
        return binder::to_v8(isolate, info.Get<typename T::real_type>(index));
    }
    else
    {
        return binder::to_v8(isolate,
            static_cast<typename T::cast_type>(info.Get<typename T::real_type>(index)));
    }
}

} // namespace acceptor_traits

template<typename...ArgsT>
SignalArgsVector GenericSignalArgsConverter(v8::Isolate *isolate,
                                       gl::PresentSignalArgs& info)
{
    size_t i = 0;
    SignalArgsVector result{
        acceptor_traits::ConvertGeneric<ArgsT>(isolate, info, i++)...
    };

    return result;
}

struct SignalEventInfo
{
    const char *name;
    uint32_t signum;
    SignalArgsConverter args_converter;
};
void DefineSignalEventsOnEventEmitter(EventEmitterBase *this_,
                                      const std::shared_ptr<gl::PresentRemoteHandle>& handle,
                                      const std::vector<SignalEventInfo>& info_vec);


struct PromisifiedRemoteCall
{
    static void ResultCallback(gl::PresentRemoteCallReturn& info);

    template<typename...ArgsT>
    static v8::Local<v8::Promise> Call(v8::Isolate *isolate,
                                       const std::shared_ptr<gl::PresentRemoteHandle>& handle,
                                       InvokeResultConverter result_converter,
                                       gl::PresentRemoteHandle::OpCode opcode,
                                       ArgsT&&...args);

    template<typename T>
    static v8::Local<v8::Value> GenericConvert(v8::Isolate *isolate,
                                               gl::PresentRemoteCallReturn& info);

    v8::Isolate                      *isolate;
    InvokeResultConverter             result_converter;
    v8::Global<v8::Promise::Resolver> resolver;
};

template<typename...ArgsT>
v8::Local<v8::Promise>
PromisifiedRemoteCall::Call(v8::Isolate *isolate,
                            const std::shared_ptr<gl::PresentRemoteHandle>& handle,
                            InvokeResultConverter result_converter,
                            gl::PresentRemoteHandle::OpCode opcode,
                            ArgsT&&...args)
{
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    auto resolver = v8::Promise::Resolver::New(context).ToLocalChecked();

    auto closure = std::make_shared<PromisifiedRemoteCall>();
    closure->isolate = isolate;
    closure->result_converter = std::move(result_converter);
    closure->resolver.Reset(isolate, resolver);

    handle->Invoke(opcode, closure, ResultCallback, std::forward<ArgsT>(args)...);
    return resolver->GetPromise();
}

template<typename T>
v8::Local<v8::Value>
PromisifiedRemoteCall::GenericConvert(v8::Isolate *isolate,
                                      gl::PresentRemoteCallReturn& info)
{
    if constexpr (T::extract_value)
    {
        static_assert(T::real_type::kHasExtractValue,
                      "Class must have signature CO_CLASS_HAS_EXTRACT_VALUE");
        static_assert(!T::create_obj, "Illegal usage of SignalArgsCast<...>");
        return binder::to_v8(isolate, static_cast<typename T::cast_type>(
                info.GetReturnValue<typename T::real_type>().value()));
    }
    else if constexpr (T::create_obj)
    {
        return binder::NewObject<typename T::cast_type>(
                isolate, info.GetReturnValue<typename T::real_type>());
    }
    else if constexpr (std::is_same<typename T::real_type, typename T::cast_type>::value)
    {
        return binder::to_v8(
                isolate, info.GetReturnValue<typename T::real_type>());
    }
    else
    {
        return binder::to_v8(
                isolate, static_cast<typename T::cast_type>(
                        info.GetReturnValue<typename T::real_type>()));
    }
}

struct PromisifiedRemoteTask
{
    template<typename Ret>
    using TaskF = std::function<Ret()>;
    template<typename Value>
    using ReturnConverterF = std::function<v8::Local<v8::Value>(Value)>;

    using TaskNoRetF = std::function<void()>;

    template<typename Ret>
    static v8::Local<v8::Promise> Submit(v8::Isolate *isolate, TaskF<Ret> task_func,
                                         ReturnConverterF<Ret> return_converter);

    static v8::Local<v8::Promise> SubmitNoRet(v8::Isolate *isolate, TaskNoRetF task_func);

    v8::Isolate *isolate;
    v8::Global<v8::Promise::Resolver> resolver;
};

template<typename Ret>
v8::Local<v8::Promise>
PromisifiedRemoteTask::Submit(v8::Isolate *isolate,
                              TaskF<Ret> task_func,
                              ReturnConverterF<Ret> return_converter)
{
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    auto resolver = v8::Promise::Resolver::New(context).ToLocalChecked();

    auto closure = std::make_shared<PromisifiedRemoteTask>();
    closure->isolate = isolate;
    closure->resolver.Reset(isolate, resolver);

    auto *thread = gl::GlobalScope::Ref().GetPresentThread();
    thread->SubmitTask<Ret>(std::move(task_func), [closure, return_converter](Ret value) {
        v8::HandleScope scope(closure->isolate);
        v8::Local<v8::Value> h = return_converter(std::move(value));
        auto r = closure->resolver.Get(closure->isolate);
        r->Resolve(closure->isolate->GetCurrentContext(), h).ToChecked();
    }, [closure](std::string error) {
        v8::HandleScope scope(closure->isolate);
        auto r = closure->resolver.Get(closure->isolate);
        r->Reject(closure->isolate->GetCurrentContext(),
                  v8::String::NewFromUtf8(closure->isolate,
                                          error.c_str()).ToLocalChecked()
                 ).ToChecked();
    });

    return resolver->GetPromise();
}

GALLIUM_BINDINGS_GLAMOR_NS_END
#endif //COCOA_GALLIUM_BINDINGS_GLAMOR_PROMISEHELPER_H
