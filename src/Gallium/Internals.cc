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

#include <string>
#include <vector>
#include <cstring>
#include <map>

#include "Core/Exception.h"
#include "Core/Journal.h"
#include "Core/Errors.h"
#include "CRPKG/ResourceManager.h"
#include "CRPKG/VirtualDisk.h"
#include "Gallium/Gallium.h"
#include "Gallium/Internals.h"
GALLIUM_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Gallium)

namespace {

std::vector<InternalScript*> iCachedScripts;

// NOLINTNEXTLINE
std::map<InternalScript::ScopeAttr, std::string> iScopeAttrNames = {
        { InternalScript::ScopeAttr::kUserExecute,  "UserExecute" },
        { InternalScript::ScopeAttr::kUserImport,   "UserImport" },
        { InternalScript::ScopeAttr::kSysExecute,   "SysExecute" },
        { InternalScript::ScopeAttr::kSysImport,    "SysImport"}
};

// NOLINTNEXTLINE
std::map<std::string, InternalScript::ScopeAttrValue> iScopeAttrValueNames = {
        { "allowed",     InternalScript::ScopeAttrValue::kAllowed },
        { "forbidden",   InternalScript::ScopeAttrValue::kForbidden },
        { "informal",    InternalScript::ScopeAttrValue::kInformal }
};

bool checkScriptScope(const InternalScript *pScript, InternalScript::ScopeAttr scope)
{
    CHECK(scope >= 0 && scope <= InternalScript::kLastScopeAttr);
    CHECK(pScript);
    auto value = pScript->scope[scope];
    if (value == InternalScript::ScopeAttrValue::kForbidden)
        return false;
    else if (value == InternalScript::ScopeAttrValue::kAllowed)
        return true;
    else if (value == InternalScript::ScopeAttrValue::kInformal)
    {
        QLOG(LOG_WARNING, "Referring internal script {} for {} is informal",
             pScript->name, iScopeAttrNames[scope]);
        return true;
    }
    MARK_UNREACHABLE();
}

std::tuple<InternalScript::ConstPtr, InternalScript::Error>
getFromCachedScript(const std::string& name, InternalScript::ScopeAttr scope)
{
    const InternalScript *s = nullptr;
    for (const InternalScript *internal : iCachedScripts)
    {
        if (name == internal->name)
        {
            s = internal;
            break;
        }
    }
    if (!s)
        return {nullptr, InternalScript::Error::kNotFound};
    if (!checkScriptScope(s, scope))
        return {nullptr, InternalScript::Error::kOutOfScope};
    return {s, InternalScript::Error::kNone};
}

#define is_whitespace(c)    ((c) == ' ' || (c) == '\t')

const char *skipWhitespaces(const char *ptr, const char *ep)
{
    while (is_whitespace(*ptr) && ptr < ep)
        ptr++;
    if (ptr == ep)
        return nullptr;
    return ptr;
}

bool rangeMatch(const char *bp, const char *ep, const char *m, size_t mSize)
{
    if (ep - bp != mSize)
        return false;
    while (ep < bp)
    {
        if (*ep != *m)
            return false;
        ep++;
        m++;
    }
    return true;
}

bool parseScriptSingleAttribute(InternalScript *script, const char *bp, const char *ep)
{
    // attribute:value
    // ^        ^     ^
    // bp       sep   ep

    const char *sep = bp;
    while (*sep != ':' && sep < ep)
        sep++;
    if (sep == ep)
        return false;

    InternalScript::ScopeAttr attr = InternalScript::ScopeAttr::kUnknown;
    for (const auto& pair : iScopeAttrNames)
    {
        if (rangeMatch(bp, sep, pair.second.c_str(), pair.second.length()))
        {
            attr = pair.first;
            break;
        }
    }
    if (attr == InternalScript::ScopeAttr::kUnknown)
        return false;

    InternalScript::ScopeAttrValue value = InternalScript::ScopeAttrValue::kEmpty;
    for (const auto& pair : iScopeAttrValueNames)
    {
        if (rangeMatch(sep + 1, ep, pair.first.c_str(), pair.first.length()))
        {
            value = pair.second;
            break;
        }
    }
    if (value == InternalScript::ScopeAttrValue::kEmpty)
        return false;

    script->scope[attr] = value;
    return true;
}

bool parseScriptAttribute(InternalScript *script)
{
    const char *sp = script->content;
    const char *ep = sp;
    while (*ep != '\0' && *ep != '\n')
        ep++;

    const char *ptr = skipWhitespaces(sp, ep);
    if (!ptr)
        return false;
    int seq = 1;
    while (ptr)
    {
        const char *tsp = ptr;
        while (!is_whitespace(*ptr) && ptr < ep)
            ptr++;

        /* now a separated token is [tsp, ptr) */
        switch (seq)
        {
        case 1:
            if (!rangeMatch(tsp, ptr, "//", std::char_traits<char>::length("//")))
                return false;
            break;
        case 2:
            if (!rangeMatch(tsp, ptr, "%scope", std::char_traits<char>::length("%scope")))
                return false;
            break;
        default:
            if (!parseScriptSingleAttribute(script, tsp, ptr))
                return false;
            break;
        }

        ptr = skipWhitespaces(ptr, ep);
        seq++;
    }

    return true;
}

std::tuple<InternalScript::ConstPtr, InternalScript::Error>
findFromCompressed(const std::string& name, InternalScript::ScopeAttr scope)
{
    if (name.empty())
        return {nullptr, InternalScript::Error::kNotFound};

    std::string file_name = name;
    if (name[0] != '/')
    {
        // We support the situation where user specifies the internal script
        // by URL `internal://foo.js` instead of `internal:///foo.js`.
        // Although the latter is more canonical, it is very unfriendly and
        // easy to write incorrectly (missing a `/`).
        file_name = "/" + name;
    }

    auto vdisk = crpkg::ResourceManager::Instance()->GetResource("@internal");
    CHECK(vdisk);

    std::optional<crpkg::VirtualDisk::Storage> storage = vdisk->GetStorage(file_name);
    if (!storage)
    {
        for (auto possible_postfix : {".js", ".mjs"})
        {
            auto possible_file_name = file_name + possible_postfix;
            storage = vdisk->GetStorage(possible_file_name);
            if (storage)
            {
                file_name = possible_file_name;
                break;
            }
        }

        if (!storage)
            return {nullptr, InternalScript::Error::kNotFound};
    }

    auto *script = new InternalScript;

    script->contentSize = storage->size;
    script->content = new char[script->contentSize + 1];
    std::memcpy(script->content, storage->addr, storage->size);

    script->content[script->contentSize] = '\0';

    script->name = file_name;
    ScopeExitAutoInvoker epi([script] { delete script; });

    // Initialize by default values
    using i = InternalScript::ScopeAttr;
    using v = InternalScript::ScopeAttrValue;
    script->scope[i::kUserExecute] = v::kForbidden;
    script->scope[i::kUserImport] = v::kAllowed;
    script->scope[i::kSysExecute] = v::kAllowed;
    script->scope[i::kSysImport] = v::kAllowed;
    script->scope[i::kUnknown] = v::kEmpty;

    if (!parseScriptAttribute(script))
    {
        // Fallback to default values
        script->scope[i::kUserExecute] = v::kForbidden;
        script->scope[i::kUserImport] = v::kAllowed;
        script->scope[i::kSysExecute] = v::kAllowed;
        script->scope[i::kSysImport] = v::kAllowed;
        script->scope[i::kUnknown] = v::kEmpty;
    }

    epi.cancel();
    iCachedScripts.push_back(script);

    if (!checkScriptScope(script, scope))
        return {nullptr, InternalScript::Error::kOutOfScope};
    return {script, InternalScript::Error::kNone};
}

} // namespace anonymous

InternalScript::~InternalScript()
{
    delete[] content;
}

std::tuple<InternalScript::ConstPtr, InternalScript::Error>
InternalScript::Get(const std::string& name, ScopeAttr scope)
{
    auto lookupCacheResult = getFromCachedScript(name, scope);
    if (std::get<1>(lookupCacheResult) != Error::kNotFound)
        return lookupCacheResult;
    return findFromCompressed(name, scope);
}

void InternalScript::GlobalCollect()
{
    for (InternalScript *ptr : iCachedScripts)
        delete ptr;
    iCachedScripts.clear();
}

GALLIUM_NS_END
