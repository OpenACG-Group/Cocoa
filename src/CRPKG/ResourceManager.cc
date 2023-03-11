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

#include "Core/Data.h"
#include "Core/Errors.h"
#include "Core/Journal.h"
#include "CRPKG/ResourceManager.h"
#include "CRPKG/VirtualDisk.h"
CRPKG_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(CRPKG.ResourceManager)

extern "C" {
extern const uint8_t kInternedCRPKGBytes[];
extern const size_t kInternedCRPKGSize;
}

ResourceManager::ResourceManager()
{
    auto interned = Data::MakeFromPtrWithoutCopy(
            const_cast<uint8_t*>(kInternedCRPKGBytes), kInternedCRPKGSize, false);
    CHECK(interned);
    CHECK(LoadFromData(std::string(kInternalResourceName), { interned }));
}

std::shared_ptr<VirtualDisk>
ResourceManager::GetResource(const std::string& name)
{
    if (resources_.count(name) == 0)
        return nullptr;
    return resources_[name];
}

bool ResourceManager::LoadFromData(const std::string& name,
                                   const std::vector<std::shared_ptr<Data>>& linear_data)
{
    CHECK(!linear_data.empty());

    if (resources_.count(name) != 0)
    {
        QLOG(LOG_ERROR, "Resource name '{}' conflicts with an existing resource", name);
        return false;
    }

    for (const auto& buf : linear_data)
    {
        CHECK(buf);
        if (!buf->hasAccessibleBuffer())
        {
            QLOG(LOG_ERROR, "Cannot create a resource '{}' from nonlinear buffers", name);
            return false;
        }
    }

    auto vdisk = VirtualDisk::MakeLayerDisk(linear_data);
    if (!vdisk)
        return false;

    resources_[name] = vdisk;
    return true;
}

bool ResourceManager::LoadFromFile(const std::string& name,
                                   const std::vector<std::string>& path)
{
    CHECK(!path.empty());

    std::vector<std::shared_ptr<Data>> mapped(path.size());
    for (int32_t i = 0; i < path.size(); i++)
    {
        mapped[i] = Data::MakeFromFileMapped(path[i], {vfs::OpenFlags::kReadonly});
        if (!mapped[i])
        {
            QLOG(LOG_ERROR, "Failed to create resource from file {}", path[i]);
            return false;
        }
    }

    return LoadFromData(name, mapped);
}

CRPKG_NAMESPACE_END
