#ifndef COCOA_INTERNALS_H
#define COCOA_INTERNALS_H

#include <string>
#include <tuple>

#include "Core/EnumClassBitfield.h"
#include "Koi/KoiBase.h"
KOI_NS_BEGIN

struct InternalScript
{
    enum class Scope : uint8_t
    {
        kUserExecutable  = 0x01,
        kUserImportable  = 0x02,
        kPrivate         = 0x04
    };

    enum class Error
    {
        kNone,
        kOutOfScope,
        kNotFound
    };

    std::string name;
    Bitfield<Scope> scope;
    const char *content;

    using ConstPtr = const InternalScript*;

    static std::tuple<ConstPtr, Error> Get(const std::string& name, Scope scope);
};

KOI_NS_END
#endif //COCOA_INTERNALS_H
