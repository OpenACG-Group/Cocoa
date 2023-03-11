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

#include "Core/ByteArrayCodecs.h"
#include "Core/Data.h"
COCOA_BEGIN_NAMESPACE

namespace {

const uint32_t base64_decode_tbl[256] = {
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0, 62, 63, 62, 62, 63,
    52, 53, 54, 55, 56, 57, 58, 59,
    60, 61,  0,  0,  0,  0,  0,  0,
     0,  0,  1,  2,  3,  4,  5,  6,
     7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22,
    23, 24, 25,  0,  0,  0,  0, 63,
     0, 26, 27, 28, 29, 30, 31, 32,
    33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48,
    49, 50, 51,  0,  0,  0,  0,  0
    /* Remaining space is filled by zero */
};

} // namespace anonymous

std::shared_ptr<Data> ByteArrayCodecs::DecodeBase64(const char *str, size_t len)
{
    if (len == 0)
        return nullptr;

    // At least one padding is required
    int padding = len % 4 != 0 || str[len - 1] == '=';

    const size_t L = ((len + 3) / 4 - padding) * 4;
    size_t out_len = L / 4 * 3 + padding;

    auto buffer = std::make_unique<uint8_t[]>(out_len + 1);

    constexpr const uint32_t *T = base64_decode_tbl;
#define S(x) static_cast<uint8_t>(str[x])
    for (size_t i = 0, j = 0; i < L; i += 4)
    {
        uint32_t n = (T[S(i)] << 18) | (T[S(i + 1)] << 12) | (T[S(i + 2)] << 6) | T[S(i + 3)];
        buffer[j++] = static_cast<uint8_t>(n >> 16);
        buffer[j++] = static_cast<uint8_t>((n >> 8) & 0xff);
        buffer[j++] = static_cast<uint8_t>(n & 0xff);
    }
    if (padding)
    {
        uint32_t n = (T[S(L)] << 18) | (T[S(L + 1)] << 12);
        buffer[out_len - 1] = static_cast<uint8_t>(n >> 16);
        if (len > L + 2 && str[L + 2] != '=')
        {
            n |= T[S(L + 2)] << 6;
            buffer[out_len] = static_cast<uint8_t>((n >> 8) & 0xff);
            out_len++;
        }
    }
#undef S

    auto deleter = buffer.get_deleter();
    return Data::MakeFromExternal(buffer.release(), out_len, [deleter](void *p) {
        deleter(reinterpret_cast<uint8_t*>(p));
    });
}

COCOA_END_NAMESPACE
