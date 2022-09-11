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

#ifndef COCOA_CORE_UTILS_H
#define COCOA_CORE_UTILS_H

#include <functional>
#include <vector>
#include <string_view>
#include <cstring>

#include "Core/Exception.h"
#include "Core/Properties.h"
#include "Core/Errors.h"
namespace cocoa::utils {

void SerializeException(const RuntimeException& except);
void ChangeWorkDirectory(const std::string& dir);
std::string GetAbsoluteDirectory(const std::string& dir);
std::string GetExecutablePath();
std::string GetCpuModel();
size_t GetMemPageSize();
size_t GetMemTotalSize();

//! Bugfix note:
//! Pass the origin string with `const std::string_view&` type instead of `const std::string&`
//! to avoid the implicit copy construction which is unsafe.
//! For example, consider the code (if we use `const std::string&` for the `str` parameter):
//! @code
//!   const char *str = "content1.content2";
//!   auto sv = SplitString(str, '.');
//! @endcode
//! The `str` points to a persistent string literal, but it is copied by `std::string`
//! when passing it to the `SplitString`. The copied `std::string` is a temporary object
//! which will be destructed after `SplitString` returns. `sv` is a vector containing
//! `std::string_view` objects which is constructed from that temporary `std::string`,
//! so the string pointers stored in `sv` is completely dangling pointers.
std::vector<std::string_view> SplitString(const std::string_view& str,
                                          std::string::value_type delimiter);

void SetThreadName(const char *name);

void PrintStackBacktrace(const std::string_view& title);

enum class Endian
{
    kLittle,
    kBig
};

inline enum Endian GetEndianness()
{
    const union {
        uint8_t b[2];
        uint16_t w;
    } u = {{1, 0}};
    return u.w == 1 ? Endian::kLittle : Endian::kBig;
}

// Round up a to the next highest multiple of b.
template <typename T>
constexpr T RoundUp(T a, T b) {
    return a % b != 0 ? a + b - (a % b) : a;
}

// Align ptr to an `alignment`-bytes boundary.
template <typename T, typename U>
constexpr T* AlignUp(T* ptr, U alignment) {
    return reinterpret_cast<T*>(
            RoundUp(reinterpret_cast<uintptr_t>(ptr), alignment));
}

inline void SwapBytes16(uint8_t *ptr, size_t size) {
    CHECK(size % 2 == 0);
    uint16_t t;
    for (size_t i = 0; i < size; i += sizeof(t)) {
        memcpy(&t, &ptr[i], sizeof(t));
        t = (t << 8) | (t >> 8);
        memcpy(&ptr[i], &t, sizeof(t));
    }
}

template<typename Map, typename K>
bool MapContains(const Map& map, const K& key)
{
    return (map.find(key) != map.end());
}

inline bool StrStartsWith(const std::string& str, char ch)
{
    if (str.empty())
        return false;
    return str[0] == ch;
}

inline bool StrStartsWith(const std::string& str, const std::string& prefix)
{
    if (str.empty() || str.length() < prefix.length())
        return false;
    return str.find(prefix, 0) == 0;
}

inline bool StrStartsWith(const std::string& str, const char *prefix)
{
    if (!prefix || str.empty() || str.length() < std::strlen(prefix))
        return false;
    return str.find(prefix, 0) == 0;
}

inline bool StrStartsWith(const std::string_view& str, char ch)
{
    if (str.empty())
        return false;
    return str[0] == ch;
}

inline bool StrStartsWith(const std::string_view& str, const std::string& prefix)
{
    if (str.empty() || str.length() < prefix.length())
        return false;
    return str.find(prefix, 0) == 0;
}

inline bool StrStartsWith(const std::string_view& str, const char *prefix)
{
    if (!prefix || str.empty() || str.length() < std::strlen(prefix))
        return false;
    return str.find(prefix, 0) == 0;
}

} // namespace cocoa

#endif //COCOA_CORE_UTILS_H
