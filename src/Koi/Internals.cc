#include <string>
#include <vector>
#include <cstring>

#include <tinyxml2.h>

#include "Core/Exception.h"
#include "Core/CrpkgImage.h"
#include "Core/Data.h"
#include "Core/Errors.h"
#include "Koi/KoiBase.h"
#include "Koi/Internals.h"
KOI_NS_BEGIN

extern "C" const ::uint8_t __koi_internals_sfs_compressed[]; // NOLINT
extern "C" const ::size_t __koi_internals_sfs_size;         // NOLINT

namespace {

std::vector<InternalScript*> iCachedScripts;
std::shared_ptr<CrpkgImage> iImage;

std::tuple<InternalScript::ConstPtr, InternalScript::Error>
getFromCachedScript(const std::string& name, InternalScript::Scope scope)
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
    if (!(s->scope & scope))
        return {nullptr, InternalScript::Error::kOutOfScope};
    return {s, InternalScript::Error::kNone};
}

using tinyxml2::XMLError;
using tinyxml2::XMLDocument;
using tinyxml2::XMLNode;
using tinyxml2::XMLElement;

InternalScript *parseScriptScopeDirective(const char *ptr, size_t size)
{
    XMLDocument document;
    auto err = document.Parse(ptr, size);
    if (err != XMLError::XML_SUCCESS)
        throw RuntimeException(__func__, XMLDocument::ErrorIDToName(err));

    XMLElement *element = document.FirstChildElement();
    if (std::strcmp(element->Name(), "internal") != 0)
        throw RuntimeException(__func__, "Bad internal XML script-directive");
    element = element->FirstChildElement();
    while (element)
    {
        printf("element %s\n", element->Name());
        element = element->NextSiblingElement();
    }
    return nullptr;
}

std::tuple<InternalScript::ConstPtr, InternalScript::Error>
findFromCompressed(const std::string& name, InternalScript::Scope scope)
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

    auto script = parseScriptScopeDirective(buf, maybeStat->size);
    delete[] buf;
    CHECK(script);

    iCachedScripts.push_back(script);
    if (!(script->scope & scope))
        return {nullptr, InternalScript::Error::kOutOfScope};
    return {script, InternalScript::Error::kNone};
}

} // namespace anonymous

std::tuple<InternalScript::ConstPtr, InternalScript::Error>
InternalScript::Get(const std::string& name, Scope scope)
{
    auto lookupCacheResult = getFromCachedScript(name, scope);
    if (std::get<1>(lookupCacheResult) != Error::kNotFound)
        return lookupCacheResult;
    return findFromCompressed(name, scope);
}

KOI_NS_END
