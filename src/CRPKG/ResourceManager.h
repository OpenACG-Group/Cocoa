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

#ifndef COCOA_CRPKG_RESOURCEMANAGER_H
#define COCOA_CRPKG_RESOURCEMANAGER_H

#include <string>
#include <memory>
#include <unordered_map>

#include "CRPKG/CRPKG.h"
#include "Core/Data.h"
#include "Core/UniquePersistent.h"
CRPKG_NAMESPACE_BEGIN

class VirtualDisk;

class ResourceManager : public UniquePersistent<ResourceManager>
{
public:
    constexpr static std::string_view kInternalResourceName = "@internal";

    ResourceManager();
    ~ResourceManager() = default;

    bool LoadFromData(const std::string& name,
                      const std::vector<std::shared_ptr<Data>>& linear_data);

    bool LoadFromFile(const std::string& name,
                      const std::vector<std::string>& path);

    std::shared_ptr<VirtualDisk> GetResource(const std::string& name);

private:
    using ResourcesMap = std::unordered_map<std::string, std::shared_ptr<VirtualDisk>>;
    ResourcesMap resources_;
};

CRPKG_NAMESPACE_END
#endif //COCOA_CRPKG_RESOURCEMANAGER_H
