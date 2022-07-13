#include <string>
#include <vector>
#include <cstring>
#include <map>

#include "Core/Exception.h"
#include "Core/QResource.h"
#include "Core/CrpkgImage.h"
#include "Core/Data.h"
#include "Core/Journal.h"
#include "Core/Errors.h"
#include "Core/Utils.h"
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
    // FIXME: Check whether `name` is a valid pathname

    auto data = QResource::Instance()->Lookup("org.cocoa.internal.v8", name);
    if (!data)
        return {nullptr, InternalScript::Error::kNotFound};

    auto *script = new InternalScript;
    script->contentSize = data->size();
    script->content = new char[script->contentSize + 1];
    script->contentSize = data->read(script->content, script->contentSize);
    script->content[script->contentSize] = '\0';
    ScopeExitAutoInvoker epi([script] { delete script; });

    if (!parseScriptAttribute(script))
        return {nullptr, InternalScript::Error::kNotFound};

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
