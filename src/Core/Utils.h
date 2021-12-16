#ifndef COCOA_UTILS_H
#define COCOA_UTILS_H

#include <functional>
#include <vector>
#include <string_view>
#include <cstring>

#include "Core/Exception.h"
#include "Core/Properties.h"
#include "Core/Errors.h"
namespace cocoa::utils {

void DumpRuntimeException(const RuntimeException& except);
void DumpProperties(const std::shared_ptr<PropertyNode>& root);
void ChangeWorkDirectory(const std::string& dir);
std::string GetAbsoluteDirectory(const std::string& dir);
std::string GetExecutablePath();
std::string GetCpuModel();
size_t GetMemPageSize();
size_t GetMemTotalSize();
std::vector<std::string_view> SplitString(const std::string& str, std::string::value_type delimiter);

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

} // namespace cocoa

#endif //COCOA_UTILS_H
