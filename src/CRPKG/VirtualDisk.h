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

#ifndef COCOA_CRPKG_VIRTUALDISK_H
#define COCOA_CRPKG_VIRTUALDISK_H

#include <vector>
#include <optional>
#include <memory>

#include "Core/Data.h"
#include "CRPKG/CRPKG.h"
CRPKG_NAMESPACE_BEGIN

class VirtualDisk
{
public:
    constexpr static uint32_t kMaxRecursiveDepth = 1024;
    class Package;
    class VDiskDirtreeNode;

    using DataVector = std::vector<std::shared_ptr<Data>>;
    static std::shared_ptr<VirtualDisk> MakeLayerDisk(const DataVector& datas);

    struct Storage
    {
        size_t          size;
        const uint8_t  *addr;
    };

    std::optional<Storage> GetStorage(const std::string_view& path);

private:
    std::vector<std::unique_ptr<Package>> packages_;
    std::unique_ptr<VDiskDirtreeNode> dirtree_;
};

CRPKG_NAMESPACE_END
#endif //COCOA_CRPKG_VIRTUALDISK_H
