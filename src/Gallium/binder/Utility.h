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

#ifndef COCOA_GALLIUM_BINDER_UTILITY_H
#define COCOA_GALLIUM_BINDER_UTILITY_H

#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <string_view>

#include <concepts>

#include "Gallium/Gallium.h"
GALLIUM_BINDER_NS_BEGIN

namespace detail
{

template<typename T>
struct tuple_tail;

template<typename Head, typename... Tail>
struct tuple_tail<std::tuple<Head, Tail...>>
{ using type = std::tuple<Tail...>; };

// is_string<T>
template<typename T> struct is_string : std::false_type {};

template<typename Char, typename Traits, typename Alloc>
struct is_string<std::basic_string<Char, Traits, Alloc>> : std::true_type {};

template<typename Char, typename Traits>
struct is_string<std::basic_string_view<Char, Traits>> : std::true_type {};

template<>
struct is_string<char const*> : std::true_type {};
template<>
struct is_string<char16_t const*> : std::true_type {};
template<>
struct is_string<char32_t const*> : std::true_type {};
template<>
struct is_string<wchar_t const*> : std::true_type {};

// is_mapping<T>
template<typename T, typename U = void>
struct is_mapping : std::false_type {};

template<typename T>
struct is_mapping<T, std::void_t<typename T::key_type, typename T::mapped_type,
    decltype(std::declval<T>().begin()), decltype(std::declval<T>().end())>> : std::true_type {};

// is_sequence<T>
template<typename T, typename U = void>
struct is_sequence : std::false_type {};

template<typename T>
struct is_sequence<T, std::void_t<typename T::value_type,
        decltype(std::declval<T>().begin()), decltype(std::declval<T>().end()),
        decltype(std::declval<T>().emplace_back(std::declval<typename T::value_type>()))>> : std::negation<is_string<T>> {};

// has_reserve<T>
template<typename T, typename U = void>
struct has_reserve : std::false_type
{
    static void reserve(T& container, size_t capacity) {} // no-op
};

template<typename T>
struct has_reserve<T, std::void_t<decltype(std::declval<T>().reserve(0))>> : std::true_type
{
    static void reserve(T& container, size_t capacity)
    { container.reserve(capacity); }
};

// is_array<T>
template<typename T>
struct is_array : std::false_type
{
    static void check_length(size_t length) {} // no-op for non-arrays
    template<typename U>
    static void set_element_at(T& container, size_t index, U&& item)
    { container.emplace_back(std::forward<U>(item)); }
};

template<typename T, std::size_t N>
struct is_array<std::array<T, N>> : std::true_type
{
    static void check_length(size_t length)
    {
        if (length != N)
        {
            throw std::runtime_error("Invalid array length: expected "
            + std::to_string(N) + " actual "
            + std::to_string(length));
        }
    }

    template<typename U>
    static void set_element_at(std::array<T, N>& array, size_t index, U&& item)
    { array[index] = std::forward<U>(item); }
};

// is_tuple<T>
template<typename T>
struct is_tuple : std::false_type {};

template<typename... Ts>
struct is_tuple<std::tuple<Ts...>> : std::true_type {};

// is_shared_ptr<T>
template<typename T>
struct is_shared_ptr : std::false_type {};

template<typename T>
struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};


// Function traits
template<typename F>
struct function_traits;

template<typename R, typename ...Args>
struct function_traits<R (Args...)>
{
    using return_type = R;
    using arguments = std::tuple<Args...>;
};

// function pointer
template<typename R, typename ...Args>
struct function_traits<R (*)(Args...)> : function_traits<R (Args...)>
{
    using pointer_type = R (*)(Args...);
};

// member function pointer
template<typename C, typename R, typename ...Args>
struct function_traits<R (C::*)(Args...)> : function_traits<R (C&, Args...)>
{
    template<typename D = C>
    using pointer_type = R (D::*)(Args...);
};

// const member function pointer
template<typename C, typename R, typename ...Args>
struct function_traits<R (C::*)(Args...) const> : function_traits<R (C const&, Args...)>
{
    template<typename D = C>
    using pointer_type = R (D::*)(Args...) const;
};

// volatile member function pointer
template<typename C, typename R, typename ...Args>
struct function_traits<R (C::*)(Args...) volatile> : function_traits<R (C volatile&, Args...)>
{
    template<typename D = C>
    using pointer_type = R (D::*)(Args...) volatile;
};

// const volatile member function pointer
template<typename C, typename R, typename ...Args>
struct function_traits<R (C::*)(Args...) const volatile> : function_traits<R (C const volatile&, Args...)>
{
    template<typename D = C>
    using pointer_type = R (D::*)(Args...) const volatile;
};

// member object pointer
template<typename C, typename R>
struct function_traits<R (C::*)> : function_traits<R (C&)>
{
    template<typename D = C>
    using pointer_type = R (D::*);
};

// const member object pointer
template<typename C, typename R>
struct function_traits<const R (C::*)> : function_traits<R (C const&)>
{
    template<typename D = C>
    using pointer_type = const R (D::*);
};

// volatile member object pointer
template<typename C, typename R>
struct function_traits<volatile R (C::*)> : function_traits<R (C volatile&)>
{
    template<typename D = C>
    using pointer_type = volatile R (D::*);
};

// const volatile member object pointer
template<typename C, typename R>
struct function_traits<const volatile R (C::*)> : function_traits<R (C const volatile&)>
{
    template<typename D = C>
    using pointer_type = const volatile R (D::*);
};

// function object, std::function, lambda
template<typename F>
struct function_traits
{
    static_assert(!std::is_bind_expression<F>::value, "std::bind result is not supported yet");
private:
    using callable_traits = function_traits<decltype(&F::operator())>;
public:
    using return_type = typename callable_traits::return_type;
    using arguments = typename tuple_tail<typename callable_traits::arguments>::type;
};

template<typename F>
struct function_traits<F&> : function_traits<F> {};

template<typename F>
struct function_traits<F&&> : function_traits<F> {};

template<typename F>
using is_void_return = std::is_same<void,
typename function_traits<F>::return_type>;

template<typename F, bool is_class>
struct is_callable_impl : std::is_function<typename std::remove_pointer<F>::type> {};

template<typename F>
struct is_callable_impl<F, true>
{
private:
    struct fallback { void operator()(); };
    struct derived : F, fallback {};

    template<typename U, U> struct check;

    template<typename>
    static std::true_type test(...);

    template<typename C>
    static std::false_type test(check<void(fallback::*)(), &C::operator()>*);

    using type = decltype(test<derived>(0));
public:
    static const bool value = type::value;
};

template<typename F>
using is_callable = std::integral_constant<bool,
is_callable_impl<F, std::is_class<F>::value>::value>;

/// Type information for custom RTTI
class type_info
{
public:
    g_nodiscard std::string_view name() const { return name_; }
    bool operator==(type_info const& other) const { return name_ == other.name_; }
    bool operator!=(type_info const& other) const { return name_ != other.name_; }
private:
    template<typename T> friend type_info type_id();
    type_info(char const* name, size_t size) : name_(name, size)
    {
    }
    
    std::string_view name_;
};

/// Get type information for type T
/// The idea is borrowed from https://github.com/Manu343726/ctti
template<typename T>
type_info type_id()
{
#if defined(__clang__) || defined(__GNUC__)
	#define BINDER_PRETTY_FUNCTION __PRETTY_FUNCTION__
	#if !defined(__clang__)
		#define BINDER_PRETTY_FUNCTION_PREFIX \
		"cocoa::gallium::binder::detail::type_info cocoa::gallium::binder::detail::type_id() [with T = "
	#else
		#define BINDER_PRETTY_FUNCTION_PREFIX \
		"cocoa::gallium::binder::detail::type_info cocoa::gallium::binder::detail::type_id() [T = "
	#endif
	#define BINDER_PRETTY_FUNCTION_SUFFIX "]"
#else
	#error "Unknown compiler"
#endif

#define BINDER_PRETTY_FUNCTION_LEN (sizeof(BINDER_PRETTY_FUNCTION) - 1)
#define BINDER_PRETTY_FUNCTION_PREFIX_LEN (sizeof(BINDER_PRETTY_FUNCTION_PREFIX) - 1)
#define BINDER_PRETTY_FUNCTION_SUFFIX_LEN (sizeof(BINDER_PRETTY_FUNCTION_SUFFIX) - 1)

    return type_info(BINDER_PRETTY_FUNCTION + BINDER_PRETTY_FUNCTION_PREFIX_LEN,
                     BINDER_PRETTY_FUNCTION_LEN - BINDER_PRETTY_FUNCTION_PREFIX_LEN - BINDER_PRETTY_FUNCTION_SUFFIX_LEN);

#undef BINDER_PRETTY_FUNCTION
#undef BINDER_PRETTY_FUNCTION_PREFIX
#undef BINDER_PRETTY_FUNCTION_SUFFFIX
#undef BINDER_PRETTY_FUNCTION_LEN
#undef BINDER_PRETTY_FUNCTION_PREFIX_LEN
#undef BINDER_PRETTY_FUNCTION_SUFFFIX_LEN
}

} // namespace detail

GALLIUM_BINDER_NS_END
#endif //COCOA_GALLIUM_BINDER_UTILITY_H
