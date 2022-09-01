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

#ifndef COCOA_GALLIUM_BINDER_PROPERTY_H
#define COCOA_GALLIUM_BINDER_PROPERTY_H

#include "Core/Errors.h"

#include "Gallium/binder/Convert.h"
#include "Gallium/binder/Function.h"

GALLIUM_BINDER_NS_BEGIN

template<typename Get, typename Set>
struct PropertyObj;

namespace detail
{

struct getter_tag {};
struct direct_getter_tag {};
struct isolate_getter_tag {};
struct setter_tag {};
struct direct_setter_tag {};
struct isolate_setter_tag {};

template<typename F>
using is_getter = std::integral_constant<bool,
        call_from_v8_traits<F>::arg_count == 0 && !is_void_return<F>::value>;

template<typename F>
using is_direct_getter = std::integral_constant<bool,
        call_from_v8_traits<F>::arg_count == 2 &&
        std::is_same<typename call_from_v8_traits<F>::template arg_type<0>,
                v8::Local<v8::String>>::value &&
        std::is_same<typename call_from_v8_traits<F>::template arg_type<1>,
                v8::PropertyCallbackInfo<v8::Value> const&>::value &&
        is_void_return<F>::value
>;

template<typename F>
using is_isolate_getter = std::integral_constant<bool,
        call_from_v8_traits<F>::arg_count == 1 &&
        is_first_arg_isolate<F>::value &&
        !is_void_return<F>::value>;

template<typename F>
using is_setter = std::integral_constant<bool,
        call_from_v8_traits<F>::arg_count == 1 && is_void_return<F>::value>;

template<typename F>
using is_direct_setter = std::integral_constant<bool,
        call_from_v8_traits<F>::arg_count == 3 &&
        std::is_same<typename call_from_v8_traits<F>::template arg_type<0>,
                v8::Local<v8::String>>::value &&
        std::is_same<typename call_from_v8_traits<F>::template arg_type<1>,
                v8::Local<v8::Value>>::value &&
        std::is_same<typename call_from_v8_traits<F>::template arg_type<2>,
                v8::PropertyCallbackInfo<void> const&>::value &&
        is_void_return<F>::value
>;

template<typename F>
using is_isolate_setter = std::integral_constant<bool,
        call_from_v8_traits<F>::arg_count == 2 &&
        is_first_arg_isolate<F>::value &&
        is_void_return<F>::value>;

template<typename F>
using select_getter_tag = typename std::conditional<is_direct_getter<F>::value,
        direct_getter_tag,
        typename std::conditional<is_isolate_getter<F>::value,
                isolate_getter_tag, getter_tag>::type
>::type;

template<typename F>
using select_setter_tag = typename std::conditional<is_direct_setter<F>::value,
        direct_setter_tag,
        typename std::conditional<is_isolate_setter<F>::value,
                isolate_setter_tag, setter_tag>::type
>::type;

template<typename Get, typename Set, bool get_is_mem_fun>
struct r_property_impl;

template<typename Get, typename Set, bool set_is_mem_fun>
struct rw_property_impl;

template<typename Get, typename Set>
struct r_property_impl<Get, Set, true>
{
    using property_type = PropertyObj<Get, Set>;

    using class_type = typename std::decay<typename std::tuple_element<0,
            typename function_traits<Get>::arguments>::type>::type;

    static_assert(is_getter<Get>::value
                  || is_direct_getter<Get>::value
                  || is_isolate_getter<Get>::value,
                  "property get function must be either `T ()` or \
                  `void (v8::Local<v8::String> name, v8::PropertyCallbackInfo<v8::Value> const& info)` or \
                  `T (v8::Isolate*)`");

    static void get_impl(class_type& obj, Get get, v8::Local<v8::String>,
                         v8::PropertyCallbackInfo<v8::Value> const& info, getter_tag)
    {
        info.GetReturnValue().Set(to_v8(info.GetIsolate(), (obj.*get)()));
    }

    static void get_impl(class_type& obj, Get get,
                         v8::Local<v8::String> name, v8::PropertyCallbackInfo<v8::Value> const& info,
                         direct_getter_tag)
    {
        (obj.*get)(name, info);
    }

    static void get_impl(class_type& obj, Get get, v8::Local<v8::String>,
                         v8::PropertyCallbackInfo<v8::Value> const& info, isolate_getter_tag)
    {
        v8::Isolate *isolate = info.GetIsolate();

        info.GetReturnValue().Set(to_v8(isolate, (obj.*get)(isolate)));
    }

    template<typename Traits>
    static void get(v8::Local<v8::String> name,
                    v8::PropertyCallbackInfo<v8::Value> const& info)
    try
    {
        auto obj = Class<class_type, Traits>::unwrap_object(info.GetIsolate(), info.This());
        CHECK(obj);

        property_type const& prop = detail::external_data::get<property_type>(info.Data());
        CHECK(prop.getter);

        if (obj && prop.getter)
        {
            get_impl(*obj, prop.getter, name, info, select_getter_tag<Get>());
        }
    }
    catch (const JSException& ex)
    {
        info.GetReturnValue().Set(JSException::TakeOver(ex));
    }
    catch (std::exception const& ex)
    {
        info.GetReturnValue().Set(throw_(info.GetIsolate(), ex.what()));
    }

    template<typename Traits>
    static void set(v8::Local<v8::String> name, v8::Local<v8::Value>,
                    v8::PropertyCallbackInfo<void> const& info)
    {
        CHECK(false && "read-only property");
        info.GetReturnValue().Set(throw_(info.GetIsolate(),
                                           "read-only property " + from_v8<std::string>(info.GetIsolate(), name)));
    }
};

template<typename Get, typename Set>
struct r_property_impl<Get, Set, false>
{
    using property_type = PropertyObj<Get, Set>;

    static void get_impl(Get get, v8::Local<v8::String>,
                         v8::PropertyCallbackInfo<v8::Value> const& info, getter_tag)
    {
        info.GetReturnValue().Set(to_v8(info.GetIsolate(), get()));
    }

    static void get_impl(Get get, v8::Local<v8::String> name,
                         v8::PropertyCallbackInfo<v8::Value> const& info, direct_getter_tag)
    {
        get(name, info);
    }

    static void get_impl(Get get, v8::Local<v8::String>,
                         v8::PropertyCallbackInfo<v8::Value> const& info, isolate_getter_tag)
    {
        v8::Isolate *isolate = info.GetIsolate();

        info.GetReturnValue().Set(to_v8(isolate, (get)(isolate)));
    }

    static void get(v8::Local<v8::String> name,
                    v8::PropertyCallbackInfo<v8::Value> const& info)
    try
    {
        property_type const& prop = detail::external_data::get<property_type>(info.Data());
        CHECK(prop.getter);

        if (prop.getter)
        {
            get_impl(prop.getter, name, info, select_getter_tag<Get>());
        }
    }
    catch (const JSException& ex)
    {
        info.GetReturnValue().Set(JSException::TakeOver(ex));
    }
    catch (std::exception const& ex)
    {
        info.GetReturnValue().Set(throw_(info.GetIsolate(), ex.what()));
    }

    static void set(v8::Local<v8::String> name, v8::Local<v8::Value>,
                    v8::PropertyCallbackInfo<void> const& info)
    {
        CHECK(false && "read-only property");
        info.GetReturnValue().Set(throw_(info.GetIsolate(),
                                         "read-only property " + from_v8<std::string>(info.GetIsolate(), name)));
    }
};

template<typename Get, typename Set>
struct rw_property_impl<Get, Set, true>
        : r_property_impl<Get, Set, std::is_member_function_pointer<Get>::value>
{
    using property_type = PropertyObj<Get, Set>;

    using class_type = typename std::decay<typename std::tuple_element<0,
            typename function_traits<Set>::arguments>::type>::type;

    static void set_impl(class_type& obj, Set set, v8::Local<v8::String>,
                         v8::Local<v8::Value> value, v8::PropertyCallbackInfo<void> const& info,
                         setter_tag)
    {
        using value_type = typename call_from_v8_traits<Set>::template arg_type<0>;

        (obj.*set)(from_v8<value_type>(info.GetIsolate(), value));
    }

    static void set_impl(class_type& obj, Set set, v8::Local<v8::String> name,
                         v8::Local<v8::Value> value, v8::PropertyCallbackInfo<void> const& info,
                         direct_setter_tag)
    {
        (obj.*set)(name, value, info);
    }

    static void set_impl(class_type& obj, Set set, v8::Local<v8::String>,
                         v8::Local<v8::Value> value, v8::PropertyCallbackInfo<void> const& info,
                         isolate_setter_tag)
    {
        using value_type = typename call_from_v8_traits<Set>::template arg_type<1>;

        v8::Isolate *isolate = info.GetIsolate();

        (obj.*set)(isolate, from_v8<value_type>(isolate, value));
    }

    template<typename Traits>
    static void set(v8::Local<v8::String> name, v8::Local<v8::Value> value,
                    v8::PropertyCallbackInfo<void> const& info)
    try
    {
        auto obj = Class<class_type, Traits>::unwrap_object(info.GetIsolate(), info.This());
        CHECK(obj);

        property_type const& prop = detail::external_data::get<property_type>(info.Data());
        CHECK(prop.setter);

        if (obj && prop.setter)
        {
            set_impl(*obj, prop.setter, name, value, info, select_setter_tag<Set>());
        }
    }
    catch (const JSException& ex)
    {
        info.GetReturnValue().Set(JSException::TakeOver(ex));
    }
    catch (std::exception const& ex)
    {
        info.GetReturnValue().Set(throw_(info.GetIsolate(), ex.what()));
    }
};

template<typename Get, typename Set>
struct rw_property_impl<Get, Set, false>
        : r_property_impl<Get, Set, std::is_member_function_pointer<Get>::value>
{
    using property_type = PropertyObj<Get, Set>;

    static void set_impl(Set set, v8::Local<v8::String>,
                         v8::Local<v8::Value> value, v8::PropertyCallbackInfo<void> const& info,
                         setter_tag)
    {
        using value_type = typename call_from_v8_traits<Set>::template arg_type<0>;

        set(from_v8<value_type>(info.GetIsolate(), value));
    }

    static void set_impl(Set set, v8::Local<v8::String> name,
                         v8::Local<v8::Value> value, v8::PropertyCallbackInfo<void> const& info,
                         direct_setter_tag)
    {
        set(name, value, info);
    }

    static void set_impl(Set set, v8::Local<v8::String>,
                         v8::Local<v8::Value> value, v8::PropertyCallbackInfo<void> const& info,
                         isolate_setter_tag)
    {
        using value_type = typename call_from_v8_traits<Set>::template arg_type<1>;

        v8::Isolate *isolate = info.GetIsolate();

        set(isolate, from_v8<value_type>(info.GetIsolate(), value));
    }

    static void set(v8::Local<v8::String> name, v8::Local<v8::Value> value,
                    v8::PropertyCallbackInfo<void> const& info)
    try
    {
        property_type const& prop = detail::external_data::get<property_type>(info.Data());
        CHECK(prop.setter);

        if (prop.setter)
        {
            set_impl(prop.setter, name, value, info, select_setter_tag<Set>());
        }
    }
    catch (const JSException& ex)
    {
        info.GetReturnValue().Set(JSException::TakeOver(ex));
    }
    catch (std::exception const& ex)
    {
        info.GetReturnValue().Set(throw_(info.GetIsolate(), ex.what()));
    }
};
} // namespace detail

template<typename Get, typename Set>
struct PropertyObj : detail::rw_property_impl<Get, Set, std::is_member_function_pointer<Set>::value>
{
    static_assert(detail::is_getter<Get>::value
                  || detail::is_direct_getter<Get>::value
                  || detail::is_isolate_getter<Get>::value,
                  "property get function must be either `T ()` or "
                  "`void (v8::Local<v8::String> name, v8::PropertyCallbackInfo<v8::Value> const& info)` or "
                  "`T (v8::Isolate*)`");

    static_assert(detail::is_setter<Set>::value
                  || detail::is_direct_setter<Set>::value
                  || detail::is_isolate_setter<Set>::value,
                  "property set function must be either `void (T)` or "
                  "`void (v8::Local<v8::String> name, v8::Local<v8::Value> value, "
                  "v8::PropertyCallbackInfo<void> const& info)` or "
                  "`void (v8::Isolate*, T)`");

    Get getter;
    Set setter;

    enum
    {
        is_readonly = false
    };

    PropertyObj(Get getter, Set setter)
            : getter(getter), setter(setter)
    {
    }

    template<typename OtherGet, typename OtherSet>
    explicit PropertyObj(PropertyObj<OtherGet, OtherSet> const& other)
            : getter(other.getter), setter(other.setter)
    {
    }
};

/// Read-only property class specialization for get only method
template<typename Get>
struct PropertyObj<Get, Get>
        : detail::r_property_impl<Get, Get, std::is_member_function_pointer<Get>::value>
{
    static_assert(detail::is_getter<Get>::value
                  || detail::is_direct_getter<Get>::value
                  || detail::is_isolate_getter<Get>::value,
                  "property get function must be either `T ()` or "
                  "void (v8::Local<v8::String> name, v8::PropertyCallbackInfo<v8::Value> const& info)` or "
                  "`T (v8::Isolate*)`");

    Get getter;

    enum
    {
        is_readonly = true
    };

    explicit PropertyObj(Get getter)
            : getter(getter)
    {
    }

    template<typename OtherGet>
    explicit PropertyObj(PropertyObj<OtherGet, OtherGet> const& other)
            : getter(other.getter)
    {
    }
};

/// Create read/write property from get and set member functions
template<typename Get, typename Set>
PropertyObj<Get, Set> Property(Get get, Set set)
{
    return PropertyObj<Get, Set>(get, set);
}

/// Create read-only property from a get function
template<typename Get>
PropertyObj<Get, Get> Property(Get get)
{
    return PropertyObj<Get, Get>(get);
}


GALLIUM_BINDER_NS_END
#endif //COCOA_GALLIUM_BINDER_PROPERTY_H
