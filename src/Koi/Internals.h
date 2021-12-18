#ifndef COCOA_INTERNALS_H
#define COCOA_INTERNALS_H

#include <string>
#include <tuple>

#include "Core/EnumClassBitfield.h"
#include "Koi/KoiBase.h"
KOI_NS_BEGIN

struct InternalScript
{
    enum ScopeAttr
    {
        kUserExecute    = 0,
        kUserImport     = 1,
        kSysExecute     = 2,
        kSysImport      = 3,
        kLastScopeAttr  = kSysImport
    };

    enum class ScopeAttrValue
    {
        kAllowed,
        kForbidden,
        kInformal
    };

    enum class Error
    {
        kNone,
        kOutOfScope,
        kNotFound
    };

    std::string name;
    std::string author;
    std::string content;
    ScopeAttrValue scope[kLastScopeAttr + 1];

    using ConstPtr = const InternalScript*;

    static std::tuple<ConstPtr, Error> Get(const std::string& name, ScopeAttr scope);
    static void GlobalCollect();
};

KOI_NS_END
#endif //COCOA_INTERNALS_H
