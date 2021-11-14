#ifndef COCOA_UUID_H
#define COCOA_UUID_H

#include <string>
#include <cstring>
#include <cassert>

#include "Koi/KoiBase.h"
KOI_NS_BEGIN

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
        assert(fBuffer && other.fBuffer);
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

KOI_NS_END
#endif //COCOA_UUID_H
