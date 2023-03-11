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

#include <cstdlib>
#include <cstring>

#include "Core/Errors.h"
#include "CRPKG/CRPKG.h"
#include "CRPKG/Protocol.h"
CRPKG_NAMESPACE_BEGIN

namespace proto {

std::unique_ptr<Header> Header::Allocate()
{
    auto hdr = std::make_unique<Header>();
    std::memcpy(hdr->magic, kFormatHeaderMagic, sizeof(hdr->magic));
    hdr->version = kVersion;

    time_t timestamp;
    time(&timestamp);
    hdr->timestamp = timestamp;

    hdr->dirtree_size = 0;
    hdr->dirtree_offset = 0;
    hdr->GST_offset = 0;
    hdr->GST_size = 0;
    hdr->GDT_offset = 0;
    hdr->GDT_size = 0;
    return hdr;
}

StringTableEntry::Ptr StringTableEntry::Allocate(const std::string_view& str)
{
    const size_t size = sizeof(StringTableEntry) + str.length();
    auto *entry = reinterpret_cast<StringTableEntry*>(std::malloc(size));
    CHECK(entry && "Failed to allocate memory");
    entry->length = str.length();
    std::memcpy(entry->str, str.data(), str.length());
    return {entry, [](StringTableEntry *p) { std::free(p); }};
}

DirTreeFlattenedEntry::Ptr DirTreeFlattenedEntry::Allocate(uint32_t nb_children)
{
    auto *entry = reinterpret_cast<DirTreeFlattenedEntry*>(
            std::malloc(sizeof(DirTreeFlattenedEntry) + nb_children * sizeof(uint64_t)));
    CHECK(entry && "Failed to allocate memory");
    entry->nb_children = nb_children;
    return {entry, [](DirTreeFlattenedEntry *p) { std::free(p); }};
}

} // namespace proto

CRPKG_NAMESPACE_END
