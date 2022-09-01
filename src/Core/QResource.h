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
