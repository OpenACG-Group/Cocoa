#ifndef COCOA_GALLIUM_INTERNALS_H
#define COCOA_GALLIUM_INTERNALS_H

#include <string>
#include <tuple>

#include "Core/EnumClassBitfield.h"
#include "Gallium/Gallium.h"
GALLIUM_NS_BEGIN

struct InternalScript
{
    enum ScopeAttr
    {
        kUserExecute    = 0,
        kUserImport     = 1,
        kSysExecute     = 2,
        kSysImport      = 3,
        kUnknown        = 4,
        kLastScopeAttr  = kUnknown
    };

    enum class ScopeAttrValue
    {
        kAllowed,
        kForbidden,
        kInformal,
        kEmpty
    };

    enum class Error
    {
        kNone,
        kOutOfScope,
        kNotFound
    };

    ~InternalScript();

    std::string         name;
    std::string         author;
    char               *content;
    size_t              contentSize;
    ScopeAttrValue      scope[kLastScopeAttr + 1];

    using ConstPtr = const InternalScript*;

    static std::tuple<ConstPtr, Error> Get(const std::string& name, ScopeAttr scope);
    static void GlobalCollect();
};

GALLIUM_NS_END
#endif //COCOA_GALLIUM_INTERNALS_H
