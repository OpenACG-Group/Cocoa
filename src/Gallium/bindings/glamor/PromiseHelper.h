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

#include "Gallium/bindings/glamor/Exports.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

/* A helper structure for asynchronous operations on RenderClient objects */
struct PromiseClosure
{
    using InfoConverter = std::function<v8::Local<v8::Value>(v8::Isolate*, gl::RenderHostCallbackInfo&)>;

    PromiseClosure(v8::Isolate *isolate, InfoConverter conv);
    ~PromiseClosure();

    static std::shared_ptr<PromiseClosure> New(v8::Isolate *isolate, const InfoConverter& converter);
    static void HostCallback(gl::RenderHostCallbackInfo& info);

    bool rejectIfEssential(gl::RenderHostCallbackInfo& info);

    v8::Local<v8::Promise> getPromise();

    template<typename Wrapper, typename T>
    static v8::Local<v8::Value> CreateObjectConverter(v8::Isolate *isolate,
                                                      gl::RenderHostCallbackInfo& info)
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
                                            const gl::Shared<gl::RenderClientObject>& client,
                                            v8::Local<v8::Function> callback,
                                            InfoAcceptor acceptor);

    ~SlotClosure();

    v8::Isolate *isolate_;
    v8::Global<v8::Function> callback_;
    gl::Shared<gl::RenderClientObject> client_;
    InfoAcceptor acceptor_;
    uint32_t slot_id_;
    int32_t signal_code_;
};

template<typename RealT, typename CastT, bool CreateObj = false>
struct InfoAcceptorCast
{
    using real_type = RealT;
    using cast_type = CastT;
    constexpr static bool create_obj = CreateObj;
};

template<typename T>
using NoCast = InfoAcceptorCast<T, T>;

template<typename T>
using AutoEnumCast = InfoAcceptorCast<T, typename std::underlying_type<T>::type>;

template<typename ParamT, typename ObjT>
using CreateObjCast = InfoAcceptorCast<ParamT, ObjT, true>;

namespace acceptor_traits {

template<typename T>
v8::Local<v8::Value> ConvertGeneric(v8::Isolate *isolate,
                                    gl::RenderHostSlotCallbackInfo& info,
                                    size_t index)
{
    if constexpr (T::create_obj)
    {
        return binder::Class<typename T::cast_type>::create_object(isolate,
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
InfoAcceptorResult GenericInfoAcceptor(v8::Isolate *isolate,
                                       gl::RenderHostSlotCallbackInfo& info)
{
    size_t i = 0;
    InfoAcceptorResult::value_type result{
        acceptor_traits::ConvertGeneric<ArgsT>(isolate, info, i++)...
    };

    return std::move(result);
}

GALLIUM_BINDINGS_GLAMOR_NS_END
#endif //COCOA_GALLIUM_BINDINGS_GLAMOR_PROMISEHELPER_H
