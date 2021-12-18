#include <string>
#include <vector>
#include <cstring>
#include <map>

#include <tinyxml2.h>

#include "Core/Exception.h"
#include "Core/CrpkgImage.h"
#include "Core/Data.h"
#include "Core/Journal.h"
#include "Core/Errors.h"
#include "Koi/KoiBase.h"
#include "Koi/Internals.h"
KOI_NS_BEGIN

extern "C" const ::uint8_t __koi_internals_sfs_compressed[]; // NOLINT
extern "C" const ::size_t __koi_internals_sfs_size;         // NOLINT

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Koi)

namespace {

std::vector<InternalScript*> iCachedScripts;
std::shared_ptr<CrpkgImage> iImage;

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

using tinyxml2::XMLError;
using tinyxml2::XMLDocument;
using tinyxml2::XMLNode;
using tinyxml2::XMLElement;

bool parseScriptXmlDirectiveScope(XMLElement *pElement, InternalScript *pScript)
{
    auto attr = pElement->FindAttribute("attr");
    auto value = pElement->FindAttribute("value");
    if (!attr || !value)
        return false;
    if (!iScopeAttrValueNames.contains(value->Value()))
        return false;

    for (const auto& p : iScopeAttrNames)
    {
        if (p.second == attr->Value())
        {
            pScript->scope[p.first] = iScopeAttrValueNames[value->Value()];
            return true;
        }
    }
    return false;
}

bool parseScriptXmlDirective(XMLElement *pElement, InternalScript *pScript)
{
    for (auto& attr : pScript->scope)
        attr = InternalScript::ScopeAttrValue::kForbidden;
    XMLElement *directive = pElement->FirstChildElement();
    while (directive)
    {
        if (!std::strcmp(directive->Name(), "scope"))
        {
            if (!parseScriptXmlDirectiveScope(directive, pScript))
                return false;
        }
        else if (!std::strcmp(directive->Name(), "author"))
        {
            if (!directive->FirstChild() || !directive->FirstChild()->ToText())
                return false;
            pScript->author = directive->FirstChild()->Value();
        }
        directive = directive->NextSiblingElement();
    }
    return true;
}

InternalScript *parseScriptXml(const char *ptr, size_t size)
{
    XMLDocument document;
    auto err = document.Parse(ptr, size);
    if (err != XMLError::XML_SUCCESS)
        throw RuntimeException(__func__, XMLDocument::ErrorIDToName(err));

    XMLElement *element = document.FirstChildElement();
    if (std::strcmp(element->Name(), "internal") != 0)
        return nullptr;
    element = element->FirstChildElement();
    auto script = new InternalScript;
    ScopeEpilogue ep([script] { delete script; });
    while (element)
    {
        if (!std::strcmp(element->Name(), "directive"))
        {
            if (!parseScriptXmlDirective(element, script))
                return nullptr;
        }
        else if (!std::strcmp(element->Name(), "content"))
        {
            if (!element->FirstChild() || !element->FirstChild()->ToText())
                return nullptr;
            script->content = element->FirstChild()->Value();
        }
        element = element->NextSiblingElement();
    }

    ep.abolish();
    return script;
}

std::tuple<InternalScript::ConstPtr, InternalScript::Error>
findFromCompressed(const std::string& name, InternalScript::ScopeAttr scope)
{
    std::string fsname;
    for (char p : name)
        fsname.push_back(p == '/' ? '.' : p);

    if (!iImage)
    {
        auto ptr = static_cast<void*>(const_cast<uint8_t*>(__koi_internals_sfs_compressed));
        auto data = Data::MakeFromPtrWithoutCopy(ptr,
                                                 __koi_internals_sfs_size,
                                                 false);
        CHECK(data);
        iImage = CrpkgImage::MakeFromData(data);
        CHECK(iImage);
    }

    auto file = iImage->openFile("/" + fsname + ".xml");
    if (!file)
        return {nullptr, InternalScript::Error::kNotFound};
    auto maybeStat = file->stat();
    CHECK(maybeStat);

    char *buf = new char[maybeStat->size + 1];
    CHECK(buf);
    std::memset(buf, '\0', maybeStat->size + 1);
    CHECK(file->read(buf, static_cast<ssize_t>(maybeStat->size)) > 0);

    auto script = parseScriptXml(buf, maybeStat->size);
    delete[] buf;
    if (!script)
    {
        std::string str(fmt::format("An error has occurred when parse internal script {} (CRPKG:/{}.xml)", name, fsname));
        throw RuntimeException(__func__, str);
    }
    script->name = name;
    iCachedScripts.push_back(script);

    if (!checkScriptScope(script, scope))
        return {nullptr, InternalScript::Error::kOutOfScope};
    return {script, InternalScript::Error::kNone};
}

} // namespace anonymous

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
    {
        CHECK(ptr != nullptr);
        delete ptr;
    }
    iCachedScripts.clear();
    iImage.reset();
}

KOI_NS_END
