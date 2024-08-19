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

#ifndef COCOA_GALLIUM_FFI_TYPETRAITS_H
#define COCOA_GALLIUM_FFI_TYPETRAITS_H

#include <cstdint>
#include <type_traits>
#include <string>
#include <string_view>

#include "fmt/format.h"
#include "include/v8-local-handle.h"

#include "Gallium/Gallium.h"
GALLIUM_FFI_NS_BEGIN

template<typename T>
using remove_cref = std::remove_const_t<std::remove_reference_t<T>>;

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

// is_local_handle<T>
template<typename T>
struct is_local_handle : std::false_type {};

template<typename T>
struct is_local_handle<v8::Local<T>> : std::true_type {};

/**
 * Type information for our lightweight RTTI.
 */
class ClassTypeInfo
{
public:
    // Get type information of type T
    template<typename T>
    constexpr static ClassTypeInfo Get();

    // Empty type
    ClassTypeInfo() : hash_(0) {}

    g_nodiscard const std::string_view& Name() const {
        return name_;
    }

    g_nodiscard uint64_t Hash() const {
        return hash_;
    }

    bool operator==(const ClassTypeInfo& other) const {
        if (other.hash_ != hash_) {
            return false;
        }
        return (other.name_ == name_);
    }

    bool operator!=(const ClassTypeInfo& other) const {
        if (other.hash_ != hash_) {
            return true;
        }
        return (other.name_ != name_);
    }

private:
    constexpr ClassTypeInfo(char const *name, size_t size)
        : hash_(std::hash<std::string_view>()(name)), name_(name, size) {}

    uint64_t            hash_;
    std::string_view    name_;
};

template<typename T>
constexpr ClassTypeInfo ClassTypeInfo::Get()
{
    using S = std::char_traits<char>;

    // For Clang compiler, this macro should be like: `... [T = typename]`
    // while for GCC, it should be like: `... [with T = typename]`.
    constexpr const char *pretty_func = __PRETTY_FUNCTION__;
    constexpr size_t pretty_func_len = S::length(pretty_func);

    // +2 to skip the `=` itself and a space
    constexpr const char *typename_begin = S::find(pretty_func, pretty_func_len, '=') + 2;
    static_assert(*(typename_begin - 1) == ' ', "Unsupported compiler");

    // -1 to strip the close-bracket at the end
    return { typename_begin, pretty_func + pretty_func_len - typename_begin - 1 };
}

GALLIUM_FFI_NS_END
#endif //COCOA_GALLIUM_FFI_TYPETRAITS_H
