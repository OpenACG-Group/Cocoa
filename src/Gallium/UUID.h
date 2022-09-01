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

#ifndef COCOA_GALLIUM_UUID_H
#define COCOA_GALLIUM_UUID_H

#include <string>
#include <cstring>
#include "Core/Errors.h"

#include "Gallium/Gallium.h"
GALLIUM_NS_BEGIN

class UUID
{
public:
    enum class Version
    {
        kNil,           // "nil" uuid, that is, all bits set to zero
        kTimeMACBased,  // Version 1 UUID
        kRandom         // Version 4 UUID
    };

    static const size_t UUID_BYTES = 16UL;

    /* Create a specified version UUID */
    explicit UUID(Version version);

    /* Parse `str` as a UUID */
    explicit UUID(const std::string& str);

    /* Copy buffer (128bits) directly */
    explicit UUID(const uint8_t *buffer);

    UUID(const UUID& lhs);
    UUID(UUID&& rhs);

    ~UUID();

    inline bool operator==(const UUID& other) {
        CHECK(fBuffer && other.fBuffer);
        return std::memcmp(fBuffer, other.fBuffer, UUID_BYTES);
    }

    koi_nodiscard inline Version getVersion() {
        return fVersion;
    }

    koi_nodiscard std::string toString();
    void parse(const std::string& str);

private:
    Version     fVersion;
    uint8_t    *fBuffer;
};

GALLIUM_NS_END
#endif //COCOA_GALLIUM_UUID_H
