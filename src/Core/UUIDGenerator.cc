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

#include <openssl/rand.h>

#include "fmt/format.h"

#include "Core/UUIDGenerator.h"
#include "Core/Errors.h"
COCOA_BEGIN_NAMESPACE

std::string GenerateRandomUUID()
{
    uint16_t buffer[8];
    CHECK(RAND_bytes(reinterpret_cast<unsigned char*>(buffer), sizeof(buffer)));

    // UUID RFC: https://www..ietf.org/rfc/rfc4122.txt
    // Used version 4 - with numbers
    return fmt::format(
        "{:04x}{:04x}-{:04x}-{:04x}-{:04x}-{:04x}{:04x}{:04x}",
        buffer[0],  // time_low
        buffer[1],  // time_mid
        buffer[2],  // time_low
        (buffer[3] & 0x0fff) | 0x4000,  // time_hi_and_version
        (buffer[4] & 0x3fff) | 0x8000,  // clk_seq_hi clk_seq_low
        buffer[5],  // node
        buffer[6],
        buffer[7]
    );
}

COCOA_END_NAMESPACE
