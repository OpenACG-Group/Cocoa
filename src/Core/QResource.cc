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

#include <sstream>
#include <ctime>

#include <tinyxml2.h>

#include "Core/Errors.h"
#include "Core/QResource.h"
#include "Core/Data.h"
#include "Core/Journal.h"
#include "Core/CrpkgImage.h"
#include "Core/Exception.h"

namespace cocoa {

using tinyxml2::XMLDocument;
using tinyxml2::XMLElement;
using tinyxml2::XMLNode;
using tinyxml2::XMLError;
using tinyxml2::XMLText;

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Core.QResource)

#define QRESOURCE_XML_FILE          "/qresource.xml"
#define QRESOURCE_CHECKSUM_FILE     "/qresource.template.checksum"

#define QRESOURCE_COMPATIBLE_SPEC           "org.cocoa.qresource.standard"
#define QRESOURCE_COMPATIBLE_PACKAGE_IMPL   "org.cocoa.qresource.standard.package"
#define QRESOURCE_COMPATIBLE_COMPILER_IMPL  "org.cocoa.qresource.standard.compiler"

extern uint8_t *qresource_autoload_tbl[];
extern size_t qresource_autoload_size_tbl[];
extern size_t qresource_autoload_tbl_size;

QResource::QResource()
{
    size_t failedCount = 0;
    for (size_t i = 0; i < qresource_autoload_tbl_size; i++)
    {
        auto data = Data::MakeFromPtrWithoutCopy(qresource_autoload_tbl[i],
                                                 qresource_autoload_size_tbl[i],
                                                 false);
        if (!data)
        {
            failedCount++;
            continue;
        }

        if (!Load(data))
            failedCount++;
    }

    QLOG(failedCount > 0 ? LOG_ERROR : LOG_INFO, "{} autoload package(s) loaded, {} failed, {} success",
         qresource_autoload_tbl_size, failedCount, qresource_autoload_tbl_size - failedCount);
    if (failedCount > 0)
        throw RuntimeException(__func__, "Failed to load internal packages");
}

QResource::~QResource()
{
    for (const auto& pair : fPackagesHashTable)
        delete pair.second;
    fPackagesHashTable.clear();
}

namespace {

#define ret_if_failed(cond, ret, fmt, ...)                  \
    do {                                                    \
        if (UNLIKELY(!(cond))) {                            \
            QLOG(LOG_ERROR, fmt __VA_OPT__(,) __VA_ARGS__); \
            return ret;                                     \
        }                                                   \
    } while (false)

bool parse_package_element(XMLElement *node, QResource::Package *pack)
{
    ret_if_failed(node->Attribute("implements"), false, "{} missing 'spec' attribute in <package>",
                  QRESOURCE_XML_FILE);
    if (std::strcmp(node->Attribute("implements"), QRESOURCE_COMPATIBLE_PACKAGE_IMPL) != 0)
    {
        QLOG(LOG_ERROR, "Unsupported QResource package name specification [{} compatible]",
             QRESOURCE_COMPATIBLE_PACKAGE_IMPL);
        return false;
    }

    XMLText *text = node->FirstChild()->ToText();
    ret_if_failed(text, false, "{} missing text in <package> element", QRESOURCE_XML_FILE);

    pack->name = text->Value();
    QLOG(LOG_DEBUG, "Loading package \"%fg<ma,hl>{}%reset\"", pack->name);
    return true;
}

bool parse_compiler(XMLElement *element, QResource::Package *pack)
{
    ret_if_failed(element->Attribute("implements"), false, "{} missing 'implements' in <compiler>",
                  QRESOURCE_XML_FILE);
    if (std::strcmp(element->Attribute("implements"), QRESOURCE_COMPATIBLE_COMPILER_IMPL) != 0)
    {
        QLOG(LOG_ERROR, "Unsupported QResource compiler specification [{} compatible]",
             QRESOURCE_COMPATIBLE_COMPILER_IMPL);
        return false;
    }

    ret_if_failed(element->Attribute("time"), false, "{} missing 'time' attribute in <compiler>",
                  QRESOURCE_XML_FILE);
    {
        time_t timestamp;
        tinyxml2::XMLUtil::ToInt64(element->Attribute("time"), &timestamp);
        pack->compileUnixTime = timestamp;

        char timebuf[100];
        struct tm *localTime = localtime(&timestamp);
        strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", localTime);
        QLOG(LOG_DEBUG, "(%fg<ma,hl>{}%reset) Compiler marked timestamp: {}", pack->name, timebuf);
        pack->compileTime = timebuf;
    }

    XMLElement *idElement = element->FirstChildElement("id");
    ret_if_failed(idElement, false, "{} missing <id> element", QRESOURCE_XML_FILE);

    ret_if_failed(idElement->FirstChild(), false, "{} missing text in <id> element", QRESOURCE_XML_FILE);
    XMLText *text = idElement->FirstChild()->ToText();
    ret_if_failed(text, false, "{} missing text in <id> element", QRESOURCE_XML_FILE);
    QLOG(LOG_DEBUG, "(%fg<ma,hl>{}%reset) Compiler ID: {}", pack->name, text->Value());
    pack->compileId = text->Value();

    XMLElement *sourceVerifyElement = element->FirstChildElement("source-verify");
    ret_if_failed(sourceVerifyElement, false, "{} missing <source-verify> element", QRESOURCE_XML_FILE);
    ret_if_failed(sourceVerifyElement->Attribute("algorithm"), false,
                  "{} missing 'algorithm' attribute in element <source-verify>", QRESOURCE_XML_FILE);
    text = sourceVerifyElement->FirstChild()->ToText();
    ret_if_failed(text, false, "{} missing text in <source-verify> element", QRESOURCE_XML_FILE);
    pack->checksum = text->Value();

    return true;
}

bool parse_objects(XMLElement *node, QResource::Package *pack)
{
    XMLElement *c = node->FirstChildElement("entry");
    QLOG(LOG_DEBUG, "(%fg<ma,hl>{}%reset) Package contains entries: ", pack->name);
    while (c)
    {
        ret_if_failed(c->Attribute("type"), false, "{} missing 'type' attribute in <entry> element",
                      QRESOURCE_XML_FILE);
        ret_if_failed(c->Attribute("path"), false, "{} missing 'path' attribute in <entry> element",
                      QRESOURCE_XML_FILE);
        std::string type = c->Attribute("type");
        std::string path = c->Attribute("path");
        ret_if_failed(type == "file" || type == "directory", false,
                      "{} 'type' attribute in <entry> element has an unrecognized value {}",
                      QRESOURCE_XML_FILE, type);

        QResource::ObjectsEntry entry{};
        if (type == "file")
            entry.type = QResource::ObjectsEntry::Type::kFile;
        else
            entry.type = QResource::ObjectsEntry::Type::kDirectory;
        entry.path = path;
        pack->entries.emplace_back(entry);

        QLOG(LOG_DEBUG, "(%fg<ma,hl>{}%reset)   {}", pack->name, path);

        c = c->NextSiblingElement("entry");
    }
    return true;
}

QResource::Package *parse_document_package(XMLDocument *document)
{
    auto *pack = new QResource::Package;
    ScopeExitAutoInvoker scope([pack] { delete pack; });

    XMLElement *root = document->FirstChildElement("qresource");
    ret_if_failed(root, nullptr, "{} missing <qresource> element", QRESOURCE_XML_FILE);
    ret_if_failed(root->Attribute("spec"), nullptr, "{} missing 'spec' attribute in <qresource>",
                  QRESOURCE_XML_FILE);

    if (std::strcmp(root->Attribute("spec"), QRESOURCE_COMPATIBLE_SPEC) != 0)
    {
        QLOG(LOG_ERROR, "Unsupported QResource specification [{} compatible]",
             QRESOURCE_COMPATIBLE_SPEC);
        return nullptr;
    }

    XMLElement *element = root->FirstChildElement("package");
    ret_if_failed(element, nullptr, "{} missing <package> element", QRESOURCE_XML_FILE);
    if (!parse_package_element(element, pack))
        return nullptr;

    element = root->FirstChildElement("description");
    ret_if_failed(element, nullptr, "{} missing <description> element", QRESOURCE_XML_FILE);
    XMLText *text = element->FirstChild()->ToText();
    ret_if_failed(text, nullptr, "{} missing text in <description> element", QRESOURCE_XML_FILE);
    QLOG(LOG_DEBUG, "(%fg<ma,hl>{}%reset) Package describes self \"{}\"", pack->name, text->Value());
    pack->description = text->Value();

    element = root->FirstChildElement("copyright");
    ret_if_failed(element, nullptr, "{} missing <copyright> element", QRESOURCE_XML_FILE);
    text = element->FirstChild()->ToText();
    ret_if_failed(text, nullptr, "{} missing text in <copyright> element", QRESOURCE_XML_FILE);
    QLOG(LOG_DEBUG, "(%fg<ma,hl>{}%reset) Package copyrighting \"{}\"", pack->name, text->Value());
    pack->copyright = text->Value();

    element = root->FirstChildElement("compiler");
    ret_if_failed(element, nullptr, "{} missing <compiler> element", QRESOURCE_XML_FILE);
    if (!parse_compiler(element, pack))
        return nullptr;

    element = root->FirstChildElement("objects");
    ret_if_failed(element, nullptr, "{} missing <objects> element", QRESOURCE_XML_FILE);
    if (!parse_objects(element, pack))
        return nullptr;

    scope.cancel();
    return pack;
}

#undef ret_if_failed

std::string read_checksum_file(const std::shared_ptr<CrpkgImage>& image)
{
    auto file = image->openFile(QRESOURCE_CHECKSUM_FILE);
    if (!file)
    {
        QLOG(LOG_ERROR, "Package does not contain a checksum file {}", QRESOURCE_CHECKSUM_FILE);
        return "";
    }
    size_t size = file->stat()->size;
    char *buffer = new char[size + 8];
    ScopeExitAutoInvoker scope([buffer] { delete[] buffer; });

    memset(buffer, '\0', size + 8);
    file->read(buffer, static_cast<ssize_t>(size));

    return {buffer};
}

} // namespace anonymous

bool QResource::Load(const std::shared_ptr<Data>& data)
{
    auto image = CrpkgImage::MakeFromData(data);
    if (!image)
    {
        QLOG(LOG_ERROR, "Package is not a crpkg (squashfs with gzip compression) archive");
        return false;
    }

    auto xmlFile = image->openFile(QRESOURCE_XML_FILE);
    if (!xmlFile)
    {
        QLOG(LOG_ERROR, "This crpkg archive is not a QResource package (missing {})",
             QRESOURCE_XML_FILE);
        return false;
    }

    std::string checksumFromFile = read_checksum_file(image);
    if (checksumFromFile.empty())
        return false;

    size_t xmlFileSize;
    if (auto maybe = xmlFile->stat())
        xmlFileSize = maybe->size;
    else
    {
        QLOG(LOG_ERROR, "Failed to stat {} in this package", QRESOURCE_XML_FILE);
        return false;
    }

    char *buffer = new char[xmlFileSize + 8];
    ScopeExitAutoInvoker scope([buffer] { delete[] buffer; });

    memset(buffer, '\0', xmlFileSize + 8);
    xmlFile->read(buffer, static_cast<ssize_t>(xmlFileSize));

    XMLDocument document;
    if (document.Parse(buffer, xmlFileSize) != XMLError::XML_SUCCESS)
    {
        QLOG(LOG_ERROR, "Failed to parse {} in this package", QRESOURCE_XML_FILE);
        return false;
    }

    Package *package = parse_document_package(&document);
    if (!package)
        return false;
    ScopeExitAutoInvoker scope2([package] { delete package; });

    if (fPackagesHashTable.count(package->name) > 0)
    {
        QLOG(LOG_ERROR, "(%fg<ma,hl>{}%reset) Conflicts with a loaded package which has the same name",
             package->name);
        return false;
    }

    if (package->checksum != checksumFromFile)
    {
        QLOG(LOG_ERROR, "(%fg<ma,hl>{}%reset) Package could not provide correct checksum", package->name);
        return false;
    }

    package->image = image;
    fPackagesHashTable[package->name] = package;
    scope2.cancel();
    return true;
}

namespace {

bool is_equivalent_pathname(const std::string& pa, const std::string& pb)
{
    // FIXME: Support informal pathname.
    return (pa == pb);
}

} // namespace anonymous

std::shared_ptr<Data> QResource::Lookup(const std::string& package,
                                        const std::string& path)
{
    if (fPackagesHashTable.count(package) == 0)
        return nullptr;
    Package *pkg = fPackagesHashTable[package];

    for (const ObjectsEntry& entry : pkg->entries)
    {
        if (entry.type == ObjectsEntry::Type::kFile &&
            is_equivalent_pathname(entry.path, path))
        {
            auto file = pkg->image->openFile(entry.path);
            if (!file)
                return nullptr;
            return Data::MakeFromPackage(file);
        }
    }
    return nullptr;
}

} // namespace cocoa
