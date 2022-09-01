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
