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

#ifndef COCOA_GALLIUM_BINDER_CALLFROMV8_H
#define COCOA_GALLIUM_BINDER_CALLFROMV8_H

#include <functional>

#include "include/v8.h"
#include "Gallium/binder/Convert.h"
#include "Gallium/binder/Utility.h"

GALLIUM_BINDER_NS_BEGIN

namespace detail {

template<typename F, size_t Offset = 0>
struct call_from_v8_traits
{
    static bool const is_mem_fun = std::is_member_function_pointer<F>::value;
    using arguments = typename function_traits<F>::arguments;

    static size_t const arg_count =
            std::tuple_size<arguments>::value - is_mem_fun - Offset;

    template<size_t Index, bool>
    struct tuple_element
    {
        using type = typename std::tuple_element<Index, arguments>::type;
    };

    template<size_t Index>
    struct tuple_element<Index, false>
    {
        using type = void;
    };

    template<typename Arg, typename Traits,
            typename T = typename std::remove_reference<Arg>::type,
            typename U = typename std::remove_pointer<T>::type
    >
    using arg_converter = typename std::conditional<
            is_wrapped_class<typename std::remove_cv<U>::type>::value,
            typename std::conditional<
                    std::is_pointer<T>::value,
                    typename Traits::template convert_ptr<U>,
                    typename Traits::template convert_ref<U>
            >::type,
            convert<typename std::remove_cv<T>::type>
    >::type;

    template<size_t Index>
    using arg_type = typename tuple_element<Index + is_mem_fun,
            Index < (arg_count + Offset)>::type;

    template<size_t Index, typename Traits>
    using arg_convert = arg_converter<arg_type<Index>, Traits>;

    template<size_t Index, typename Traits>
    static decltype(arg_convert<Index, Traits>::from_v8(std::declval<v8::Isolate *>(),
                                                        std::declval<v8::Local<v8::Value>>()))
    arg_from_v8(v8::FunctionCallbackInfo<v8::Value> const& args)
    {
        return arg_convert<Index, Traits>::from_v8(args.GetIsolate(), args[Index - Offset]);
    }

    static void check(v8::FunctionCallbackInfo<v8::Value> const& args)
    {
        if (args.Length() != arg_count)
        {
            throw std::runtime_error("argument count does not match function definition");
        }
    }
};

template<typename F>
using isolate_arg_call_traits = call_from_v8_traits<F, 1>;

template<typename F, size_t Offset = 0>
struct v8_args_call_traits : call_from_v8_traits<F, Offset>
{
    template<size_t Index, typename Traits>
    static v8::FunctionCallbackInfo<v8::Value> const&
    arg_from_v8(v8::FunctionCallbackInfo<v8::Value> const& args)
    {
        return args;
    }

    static void check(v8::FunctionCallbackInfo<v8::Value> const&)
    {
    }
};

template<typename F>
using isolate_v8_args_call_traits = v8_args_call_traits<F, 1>;

template<typename F, size_t Offset>
using is_direct_args = std::integral_constant<bool,
        call_from_v8_traits<F>::arg_count == (Offset + 1) &&
        std::is_same<typename call_from_v8_traits<F>::template arg_type<Offset>,
                v8::FunctionCallbackInfo<v8::Value> const&>::value>;

template<typename F>
using is_first_arg_isolate = std::integral_constant<bool,
        call_from_v8_traits<F>::arg_count != 0 &&
        std::is_same<typename call_from_v8_traits<F>::template arg_type<0>,
                v8::Isolate *>::value>;

template<typename F>
using select_call_traits = typename std::conditional<is_first_arg_isolate<F>::value,
        typename std::conditional<is_direct_args<F, 1>::value,
                isolate_v8_args_call_traits<F>,
                isolate_arg_call_traits<F>>::type,
        typename std::conditional<is_direct_args<F, 0>::value,
                v8_args_call_traits<F>,
                call_from_v8_traits<F>>::type
>::type;

template<typename Traits, typename F, typename CallTraits, size_t ...Indices>
typename function_traits<F>::return_type
call_from_v8_impl(F&& func, v8::FunctionCallbackInfo<v8::Value> const& args,
                  CallTraits, std::index_sequence<Indices...>)
{
    return func(CallTraits::template arg_from_v8<Indices, Traits>(args)...);
}

template<typename Traits, typename T, typename F, typename CallTraits, size_t ...Indices>
typename function_traits<F>::return_type
call_from_v8_impl(T& obj, F&& func, v8::FunctionCallbackInfo<v8::Value> const& args,
                  CallTraits, std::index_sequence<Indices...>)
{
    return (obj.*func)(CallTraits::template arg_from_v8<Indices, Traits>(args)...);
}

template<typename Traits, typename F, size_t ...Indices>
typename function_traits<F>::return_type
call_from_v8_impl(F&& func, v8::FunctionCallbackInfo<v8::Value> const& args,
                  isolate_arg_call_traits<F>, std::index_sequence<Indices...>)
{
    return func(args.GetIsolate(),
                isolate_arg_call_traits<F>::template arg_from_v8<Indices + 1, Traits>(args)...);
}

template<typename Traits, typename T, typename F, size_t ...Indices>
typename function_traits<F>::return_type
call_from_v8_impl(T& obj, F&& func, v8::FunctionCallbackInfo<v8::Value> const& args,
                  isolate_arg_call_traits<F>, std::index_sequence<Indices...>)
{
    return (obj.*func)(args.GetIsolate(),
                       isolate_arg_call_traits<F>::template arg_from_v8<Indices + 1, Traits>(args)...);
}

template<typename Traits, typename F, size_t ...Indices>
typename function_traits<F>::return_type
call_from_v8_impl(F&& func, v8::FunctionCallbackInfo<v8::Value> const& args,
                  isolate_v8_args_call_traits<F>, std::index_sequence<Indices...>)
{
    return func(args.GetIsolate(), args);
}

template<typename Traits, typename T, typename F, size_t ...Indices>
typename function_traits<F>::return_type
call_from_v8_impl(T& obj, F&& func, v8::FunctionCallbackInfo<v8::Value> const& args,
                  isolate_v8_args_call_traits<F>, std::index_sequence<Indices...>)
{
    return (obj.*func)(args.GetIsolate(), args);
}

template<typename Traits, typename F>
typename function_traits<F>::return_type
call_from_v8(F&& func, v8::FunctionCallbackInfo<v8::Value> const& args)
{
    using call_traits = select_call_traits<F>;
    call_traits::check(args);
    return call_from_v8_impl<Traits>(std::forward<F>(func),
                                     args,
                                     call_traits(),
                                     std::make_index_sequence<call_traits::arg_count>());
}

template<typename Traits, typename T, typename F>
typename function_traits<F>::return_type
call_from_v8(T& obj, F&& func, v8::FunctionCallbackInfo<v8::Value> const& args)
{
    using call_traits = select_call_traits<F>;
    call_traits::check(args);
    return call_from_v8_impl<Traits>(obj,
                                     std::forward<F>(func), args,
                                     call_traits(),
                                     std::make_index_sequence<call_traits::arg_count>());
}

} // namespace detail

GALLIUM_BINDER_NS_END
#endif //COCOA_GALLIUM_BINDER_CALLFROMV8_H
