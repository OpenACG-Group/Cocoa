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

#ifndef COCOA_CRPKG_PROTOCOL_H
#define COCOA_CRPKG_PROTOCOL_H

#include <cstdint>
#include <memory>
#include <functional>
#include <string_view>

#include "Core/Project.h"
#include "CRPKG/CRPKG.h"
CRPKG_NAMESPACE_BEGIN

namespace proto {

const char kFormatHeaderMagic[] = "CRPKG\xe3";

struct [[gnu::packed]] Header
{
    static std::unique_ptr<Header> Allocate();

    // Filled with `kFormatHeaderMagic`
    uint8_t     magic[6];

    // Format version
    uint32_t    version;

    // A Unix timestamp in milliseconds (since Unix Epoch)
    uint64_t    timestamp;

    size_t      GST_offset;       ///< Global String Table (GST) offset
    uint32_t    GST_size;         ///< Number of entries in GST

    size_t      dirtree_offset;   ///< Dirtree offset
    uint32_t    dirtree_size;     ///< Number of nodes in dirtree

    size_t      GDT_offset;       ///< Global Data Table (GDT) offset
    uint32_t    GDT_size;         ///< Number of entries in GDT
};

template<typename T>
using UniquePtrLambdaDeleter = std::unique_ptr<T, std::function<void(T*)>>;

struct [[gnu::packed]] StringTableEntry
{
    using Ptr = UniquePtrLambdaDeleter<StringTableEntry>;

    static Ptr Allocate(const std::string_view& str);

    g_nodiscard g_inline size_t ComputeSizeInBytes() const {
        return sizeof(StringTableEntry) + length;
    }

    uint32_t length;

    // A C-string without terminator ('\0'). Length of this
    // string is constrained by `length`
    char str[0];
};

struct [[gnu::packed]] DirTreeFlattenedEntry
{
    using Ptr = UniquePtrLambdaDeleter<DirTreeFlattenedEntry>;
    static Ptr Allocate(uint32_t nb_children);

#define DT_FLAG_FILE        (1 << 0)
#define DT_FLAG_DIRECTORY   (1 << 1)

    g_nodiscard g_inline size_t ComputeSizeInBytes() const {
        return sizeof(DirTreeFlattenedEntry) + nb_children * sizeof(uint64_t);
    }

    // For file entry, `nb_children` is 1, and a data table index is store at children[0];
    // for directory entry, `nb_children` is the number of sub-entries, and
    // `children` is an array of dirtree indices.
    uint32_t    nb_children;
    uint8_t     flags;
    uint32_t    name;
    uint64_t    children[0];
};

struct [[gnu::packed]] DataTableEntry
{
    g_nodiscard g_inline size_t ComputeSizeInBytes() const {
        return sizeof(DataTableEntry) + size;
    }

    uint64_t size;
    uint8_t data[];
};

} // namespace proto

CRPKG_NAMESPACE_END
#endif //COCOA_CRPKG_PROTOCOL_H
