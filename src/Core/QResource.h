#ifndef COCOA_CORE_QRESOURCE_H
#define COCOA_CORE_QRESOURCE_H

#include <memory>
#include <list>
#include <unordered_map>
#include <vector>

#include "Core/Project.h"
#include "Core/UniquePersistent.h"

namespace cocoa
{

class Data;
class CrpkgImage;

/**
 * `QResource` maintains a highly simplified virtual filesystem based on
 * compressed squashfs (crpkg) format.
 */
class QResource : public UniquePersistent<QResource>
{
public:
    struct ObjectsEntry
    {
        enum class Type { kFile, kDirectory };
        std::string path;
        Type        type;
    };

    struct Package
    {
        std::string                 name;
        std::string                 description;
        std::string                 copyright;
        time_t                      compileUnixTime;
        std::string                 compileTime;
        std::string                 compileId;
        std::string                 checksum;
        std::shared_ptr<CrpkgImage> image;
        std::vector<ObjectsEntry>   entries;
    };

    QResource();
    ~QResource();

    bool Load(const std::shared_ptr<Data>& data);

    g_nodiscard std::shared_ptr<Data> Lookup(const std::string& package,
                                              const std::string& path);

private:
    std::unordered_map<std::string, Package*>   fPackagesHashTable;
};

} // namespace cocoa

#endif //COCOA_CORE_QRESOURCE_H
