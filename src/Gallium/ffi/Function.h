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

#ifndef COCOA_GALLIUM_FFI_FUNCTION_H
#define COCOA_GALLIUM_FFI_FUNCTION_H

#include <functional>

#include "include/v8.h"
#include "include/v8-fast-api-calls.h"
#include "fmt/format.h"

#include "Gallium/Gallium.h"
#include "Gallium/RuntimeBase.h"
#include "Gallium/ffi/DataTypes.h"
#include "Gallium/ffi/ReturnValue.h"
#include "Gallium/ffi/JSObject.h"
#include "Gallium/ffi/Interface.h"
#include "Gallium/ffi/Class.h"
#include "Gallium/ffi/ArrayBuffer.h"
GALLIUM_FFI_NS_BEGIN

using v8::FunctionCallbackInfo;
using v8::PropertyCallbackInfo;

using DirectCallable = std::function<void(const FunctionCallbackInfo<Value>&)>;

/**
 * Wrap a `std::function` compatible callable object into a `v8::FunctionTemplate`.
 * The `std::function` object can be treated as a closure, which packs some data
 * with a function together.
 */
Local<FunctionTemplate> WrapFunction(Isolate *isolate, DirectCallable callable,
                                     v8::SideEffectType side_effect_type);

namespace fn {

bool CheckThisDisposeState(JSObject *this_, bool skip_disposing_check, bool skip_disposed_check);

template<typename T, size_t I, typename Enable = void>
struct ExtractFuncArg;

template<typename T, size_t I>
struct ExtractFuncArg<T, I, typename std::enable_if<!Cast<T>::kWrapperCast>::type>
{
    static T Invoke(Isolate *isolate, const FunctionCallbackInfo<Value>& info) {
        Opt<T> opt = Cast<T>::From(isolate, info[I]);
        if (!opt.has_value()) {
            throw std::runtime_error(fmt::format(
                    "Argument #{} of function call has a wrong type", I + 1));
        }
        return std::move(*opt);
    }
};

template<typename T, size_t I>
struct ExtractFuncArg<T, I, typename std::enable_if<Cast<T>::kWrapperCast>::type>
{
    static T Invoke(Isolate *isolate, const FunctionCallbackInfo<Value>& info) {
        Ret<T> result = Cast<T>::From(isolate, info[I]);
        if (result.HasError()) {
            throw std::runtime_error(fmt::format(
                    "Argument #{} of function call: {}", I + 1, result.GetError().message));
        }
        return std::move(result.Extract());
    }
};

template<typename TAdapter, size_t I>
struct ExtractFuncArg<TAdapter, I, typename std::enable_if<std::is_base_of_v<ArgAdapter, TAdapter>>::type>
{
    static TAdapter Invoke(Isolate *isolate, const FunctionCallbackInfo<Value>& info) {
        Ret<TAdapter> result = TAdapter::Cast(isolate, info[I]);
        if (result.HasError()) {
            throw std::runtime_error(fmt::format(
                    "Argument #{} of function call: {}", I + 1, result.GetError().message));
        }
        return std::move(result.Extract());
    }
};

template<typename T, size_t I>
struct ExtractFuncArg<Opt<T>, I>
{
    static Opt<T> Invoke(Isolate *isolate, const FunctionCallbackInfo<Value>& info) {
        Local<Value> value = info[I];
        if (value->IsNullOrUndefined()) {
            return std::nullopt;
        }
        return ExtractFuncArg<T, I>::Invoke(isolate, info);
    }
};

template<typename T, size_t I>
T ExtractFuncArgOrThrow(Isolate *isolate, const FunctionCallbackInfo<Value>& info) {
    return ExtractFuncArg<T, I>::Invoke(isolate, info);
}

template<typename RetT, typename...ArgsT, size_t...Index>
Return<RetT> CallAddressHelper(Isolate *isolate, Return<RetT>(*address)(ArgsT...),
                               const FunctionCallbackInfo<Value>& info,
                               std::index_sequence<Index...>)
{
    return address(ExtractFuncArgOrThrow<remove_cref<ArgsT>, Index>(isolate, info)...);
}

template<typename RetT, typename...ArgsT>
void CallFunctionAddress(const FunctionCallbackInfo<Value>& info)
{
    using FuncAddress = Return<RetT>(*)(ArgsT...);

    CHECK(!info.IsConstructCall());

    Isolate *isolate = info.GetIsolate();
    if (info.Length() != sizeof...(ArgsT)) {
        isolate->ThrowError("Number of arguments does not match the function definition");
        return;
    }

    v8::HandleScope handle_scope(isolate);

    CHECK(info.Data()->IsExternal());
    auto *func_address = reinterpret_cast<FuncAddress>(info.Data().As<v8::External>()->Value());
    CHECK(func_address);

    auto index_seq = std::make_index_sequence<sizeof...(ArgsT)>();
    try {
        Return<RetT> ret(CallAddressHelper(isolate, func_address, info, index_seq));
        if (ret.HasError()) {
            ret.GetError().Throw(isolate);
            return;
        }
        if constexpr (!std::is_void_v<RetT>) {
            ret.AssignTo(info.GetReturnValue());
        }
    } catch (const std::exception& error) {
        isolate->ThrowError(Cast<const char*>::ToChecked(isolate, error.what()));
        return;
    }
}

template<typename T, typename...ArgsT, size_t...Index>
T *CallConstructorHelper(Isolate *isolate, const FunctionCallbackInfo<Value>& info,
                         std::index_sequence<Index...>)
{
    return new T(ExtractFuncArgOrThrow<remove_cref<ArgsT>, Index>(isolate, info)...);
}

template<typename T, typename...ArgsT>
void CallConstructor(const FunctionCallbackInfo<Value>& info)
{
    Isolate *isolate = info.GetIsolate();
    if (!info.IsConstructCall()) {
        isolate->ThrowError("The function must be called as a constructor");
        return;
    }

    v8::HandleScope handle_scope(isolate);

    auto index_seq = std::make_index_sequence<sizeof...(ArgsT)>();
    T *object_ptr;
    try {
        object_ptr = CallConstructorHelper<T, ArgsT...>(isolate, info, index_seq);
    } catch (const std::exception& error) {
        isolate->ThrowError(Cast<const char*>::ToChecked(isolate, error.what()));
        return;
    }
    CHECK(object_ptr && "Memory allocation failed");
    info.GetReturnValue().Set(JSObject::Wrap(
            isolate, ClassTypeInfo::Get<T>(), object_ptr, object_ptr));
}

void CallNotConstructible(const FunctionCallbackInfo<Value>& info);

template<typename F>
struct MemberFuncStore
{
    F member_func;
    bool skip_disposed_check;
    bool skip_disposing_check;
};

template<typename T, typename RetT, typename...ArgsT, size_t...Index>
Return<RetT> CallMemberHelper(Isolate *isolate, T *this_, Return<RetT>(T::*member_func)(ArgsT...),
                              const FunctionCallbackInfo<Value>& info,
                              std::index_sequence<Index...>)
{
    return (this_->*member_func)(ExtractFuncArgOrThrow<remove_cref<ArgsT>, Index>(isolate, info)...);
}

template<typename T, typename RetT, typename...ArgsT>
void CallMemberFunction(const FunctionCallbackInfo<Value>& info)
{
    using MemberFunc = Return<RetT>(T::*)(ArgsT...);

    CHECK(!info.IsConstructCall());

    Isolate *isolate = info.GetIsolate();
    if (info.Length() != sizeof...(ArgsT)) {
        isolate->ThrowError("Number of arguments does not match the function definition");
        return;
    }

    if (!info.This()->IsObject()) {
        isolate->ThrowError("Member function must be called with a valid `this`");
        return;
    }

    v8::HandleScope handle_scope(isolate);

    CHECK(info.Data()->IsExternal());
    auto *store = static_cast<MemberFuncStore<MemberFunc>*>(
            info.Data().As<v8::External>()->Value());

    T *this_ = static_cast<T*>(JSObject::Unwrap(isolate, ClassTypeInfo::Get<T>(), info.This()));
    if (!this_) {
        isolate->ThrowError("Invalid `this` object to call the function");
        return;
    }

    if (!CheckThisDisposeState(this_, store->skip_disposing_check, store->skip_disposing_check))
        return;

    auto index_seq = std::make_index_sequence<sizeof...(ArgsT)>();
    try {
        Return<RetT> ret(CallMemberHelper(isolate, this_, store->member_func, info, index_seq));
        if (ret.HasError()) {
            ret.GetError().Throw(isolate);
            return;
        }
        if constexpr (!std::is_void_v<RetT>) {
            ret.AssignTo(info.GetReturnValue());
        }
    } catch (const std::exception& error) {
        isolate->ThrowError(Cast<const char*>::ToChecked(isolate, error.what()));
        return;
    }
}

} // namespace fn

template<typename RetT, typename...ArgsT>
Local<FunctionTemplate> WrapFunctionAddress(Isolate *isolate,
                                            v8::SideEffectType side_effect_type,
                                            Return<RetT>(*func_address)(ArgsT...))
{
    CHECK(func_address && "Invalid function address");
    return FunctionTemplate::New(
        isolate,
        fn::CallFunctionAddress<RetT, ArgsT...>,
        v8::External::New(isolate, reinterpret_cast<void*>(func_address)),
        /* signature= */  {},
        /* length= */  0,
        v8::ConstructorBehavior::kThrow,
        side_effect_type,
        /* c_function= */ nullptr,
        /* instance_type= */ 0
    );
}

template<typename T, typename...ArgsT>
Local<FunctionTemplate> WrapConstructor(Isolate *isolate,
                                        v8::SideEffectType side_effect_type)
{
    static_assert(std::is_base_of<JSObject, T>::value, "`T` must be derived from `JSObject`");
    return FunctionTemplate::New(
        isolate,
        fn::CallConstructor<T, ArgsT...>,
        /* data= */ {},
        /* signature= */ {},
        /* length= */ 0,
        v8::ConstructorBehavior::kAllow,
        side_effect_type,
        /* c_function= */ nullptr,
        /* instance_type= */ 0
    );
}

Local<FunctionTemplate> WrapNotConstructibleConstructor(Isolate *isolate);

template<typename T, typename RetT, typename...ArgsT>
Local<FunctionTemplate> WrapMemberFunction(Isolate *isolate,
                                           v8::SideEffectType side_effect_type,
                                           Return<RetT>(T::*member_func)(ArgsT...),
                                           bool skip_disposing_check = false,
                                           bool skip_disposed_check = false)
{
    static_assert(std::is_base_of<JSObject, T>::value, "`T` must be derived from `JSObject`");

    using MemberFuncT = Return<RetT>(T::*)(ArgsT...);
    auto *store = new fn::MemberFuncStore<MemberFuncT>;
    store->member_func = member_func;
    store->skip_disposing_check = skip_disposing_check;
    store->skip_disposed_check = skip_disposed_check;

    RuntimeBase::FromIsolate(isolate)->AddExternalCallback(
        RuntimeBase::ExternalCallbackType::kAfterRuntimeDispose,
        [store]() {
            delete store;
            return RuntimeBase::ExternalCallbackAfterCall::kRemove;
        }
    );

    return FunctionTemplate::New(
        isolate,
        fn::CallMemberFunction<T, RetT, ArgsT...>,
        v8::External::New(isolate, store),
        /* signature= */ {},
        /* length= */ 0,
        v8::ConstructorBehavior::kThrow,
        side_effect_type,
        /* c_function= */ nullptr,
        /* instance_type= */ 0
    );
}

namespace fn {

template<typename T, typename PropT>
struct PropertyAccessorStore
{
    using Getter = Return<PropT>(T::*)();
    using Setter = Return<void>(T::*)(PropT);

    Getter getter_mfunc;
    Setter setter_mfunc;

    bool skip_disposing_check;
    bool skip_disposed_check;
};

template<typename T, typename PropT>
void CallPropertyGetter(const FunctionCallbackInfo<Value>& info)
{
    Isolate *isolate = info.GetIsolate();
    if (!info.This()->IsObject()) {
        isolate->ThrowError("Property accessor must be called with a valid `this`");
        return;
    }

    v8::HandleScope handle_scope(isolate);
    CHECK(info.Data()->IsExternal());
    auto *store = static_cast<PropertyAccessorStore<T, PropT>*>(
            info.Data().As<v8::External>()->Value());
    CHECK(store && store->getter_mfunc);

    T *this_ = static_cast<T*>(JSObject::Unwrap(isolate, ClassTypeInfo::Get<T>(), info.This()));
    if (!this_) {
        isolate->ThrowError("Invalid `this` object to call the accessor");
        return;
    }

    if (!CheckThisDisposeState(this_, store->skip_disposing_check, store->skip_disposed_check))
        return;

    auto mfunc = store->getter_mfunc;
    Return<PropT> ret((this_->*mfunc)());
    if (ret.HasError()) {
        ret.GetError().Throw(isolate);
        return;
    }
    ret.AssignTo(info.GetReturnValue());
}

template<typename T, typename PropT>
void CallPropertySetter(const FunctionCallbackInfo<Value>& info)
{
    Isolate *isolate = info.GetIsolate();
    if (!info.This()->IsObject()) {
        isolate->ThrowError("Property accessor must be called with a valid `this`");
        return;
    }

    if (info.Length() != 1) {
        isolate->ThrowError("Property accessor (setter) only accepts one argument");
        return;
    }

    v8::HandleScope handle_scope(isolate);
    CHECK(info.Data()->IsExternal());
    auto *store = static_cast<PropertyAccessorStore<T, PropT>*>(
            info.Data().As<v8::External>()->Value());
    CHECK(store && store->setter_mfunc);

    T *this_ = static_cast<T*>(JSObject::Unwrap(isolate, ClassTypeInfo::Get<T>(), info.This()));
    if (!this_) {
        isolate->ThrowError("Invalid `this` object to call the accessor");
        return;
    }

    if (!CheckThisDisposeState(this_, store->skip_disposing_check, store->skip_disposed_check))
        return;

    auto mfunc = store->setter_mfunc;
    if constexpr (Cast<PropT>::kWrapperCast) {
        Ret<PropT> ret_value(Cast<PropT>::From(isolate, info[0]));
        if (ret_value.HasError()) {
            ret_value.GetError().Throw(isolate);
            return;
        }
        Return<void> err((this_->*mfunc)(std::move(ret_value.Extract())));
        if (err.HasError())
            err.GetError().Throw(isolate);
    } else {
        Opt<PropT> opt_value = Cast<PropT>::From(isolate, info[0]);
        if (!opt_value) {
            isolate->ThrowError("The value provided for setter has a wrong type");
            return;
        }
        Return<void> err((this_->*mfunc)(std::move(*opt_value)));
        if (err.HasError())
            err.GetError().Throw(isolate);
    }
}

} // namespace fn

template<typename T, typename PropT>
void WrapPropertyAccessor(Isolate *isolate,
                         Local<String> property,
                         Local<ObjectTemplate> target,
                         Return<PropT>(T::*getter_member_func)(),
                         Return<void>(T::*setter_member_func)(PropT),
                         v8::SideEffectType getter_side_effect,
                         v8::SideEffectType setter_side_effect,
                         bool skip_disposing_check = false,
                         bool skip_disposed_check = false)
{
    static_assert(std::is_base_of<JSObject, T>::value, "`T` must be derived from `JSObject`");
    CHECK(getter_member_func && "Getter is absent");

    auto *store = new fn::PropertyAccessorStore<T, PropT>;
    store->getter_mfunc = getter_member_func;
    store->setter_mfunc = setter_member_func;
    store->skip_disposing_check = skip_disposing_check;
    store->skip_disposed_check = skip_disposed_check;

    RuntimeBase::FromIsolate(isolate)->AddExternalCallback(
        RuntimeBase::ExternalCallbackType::kAfterRuntimeDispose,
        [store]() {
            delete store;
            return RuntimeBase::ExternalCallbackAfterCall::kRemove;
        }
    );

    uint32_t attrs = v8::PropertyAttribute::DontDelete;
    if (!setter_member_func)
        attrs |= v8::PropertyAttribute::ReadOnly;

    Local<v8::External> external_data = v8::External::New(isolate, store);

    Local<FunctionTemplate> ft_getter;
    ft_getter = FunctionTemplate::New(
        isolate,
        fn::CallPropertyGetter<T, PropT>,
        external_data,
        /* signature= */  {},
        /* length= */ 0,
        v8::ConstructorBehavior::kThrow,
        getter_side_effect,
        /* c_function= */ nullptr,
        /* instance_type= */ 0
    );

    Local<FunctionTemplate> ft_setter;
    if (setter_member_func)
    {
        ft_setter = FunctionTemplate::New(
            isolate,
            fn::CallPropertySetter<T, PropT>,
            external_data,
            /* signature= */  {},
            /* length= */ 1,
            v8::ConstructorBehavior::kThrow,
            setter_side_effect,
            /* c_function= */ nullptr,
            /* instance_type= */ 0
        );
    }

    target->SetAccessorProperty(
            property, ft_getter, ft_setter, static_cast<v8::PropertyAttribute>(attrs));
}

GALLIUM_FFI_NS_END
#endif //COCOA_GALLIUM_FFI_FUNCTION_H
