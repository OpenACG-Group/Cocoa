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

#include <cerrno>
#include <sys/mman.h>
#include <cstdlib>
#include <unistd.h>

#include "Core/Journal.h"
#include "Core/Errors.h"
#include "Reactor/ExecutableMemory.h"

#undef allocate
#undef deallocate
REACTOR_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Reactor.ExecutableMemory)

namespace {
struct Allocation
{
    //	size_t bytes;
    unsigned char *block;
};

void *allocateRaw(size_t bytes, size_t alignment)
{
    CHECK((alignment & (alignment - 1)) == 0);  // Power of 2 alignment.
    auto *block = new unsigned char[bytes + sizeof(Allocation) + alignment];
    auto *aligned = (unsigned char *)
            ((uintptr_t) (block + sizeof(Allocation) + alignment - 1) & -(intptr_t) alignment);
    auto *allocation = (Allocation *) (aligned - sizeof(Allocation));//	allocation->bytes = bytes;
    allocation->block = block;

    return aligned;;
}

int permissionsToMmapProt(Bitfield<MemPermission> permissions)
{
    int result = 0;
    if(permissions & MemPermission::kRead)
    {
        result |= PROT_READ;
    }
    if(permissions & MemPermission::kWrite)
    {
        result |= PROT_WRITE;
    }
    if(permissions & MemPermission::kExecute)
    {
        result |= PROT_EXEC;
    }
    return result;
}
}  // anonymous namespace

size_t MemoryPageSize()
{
    static int pageSize = static_cast<int>(sysconf(_SC_PAGESIZE));
    return pageSize;
}

void *allocate(size_t bytes, size_t alignment)
{
    void *memory = allocateRaw(bytes, alignment);

    if(memory)
    {
        memset(memory, 0, bytes);
    }

    return memory;
}

void deallocate(void *memory)
{
    if(memory)
    {
        auto *aligned = (unsigned char *)memory;
        auto *allocation = (Allocation *)(aligned - sizeof(Allocation));

        delete[] allocation->block;
    }
}

// Rounds |x| up to a multiple of |m|, where |m| is a power of 2.
inline uintptr_t roundUp(uintptr_t x, uintptr_t m)
{
    CHECK(m > 0 && (m & (m - 1)) == 0);  // |m| must be a power of 2.
    return (x + m - 1) & ~(m - 1);
}

void *AllocateMemoryPages(size_t bytes, Bitfield<MemPermission> permissions, bool need_exec)
{
    size_t pageSize = MemoryPageSize();
    size_t length = roundUp(bytes, pageSize);
    void *mapping = allocate(length, pageSize);

    ProtectMemoryPages(mapping, length, permissions);

    return mapping;
}

void ProtectMemoryPages(void *memory, size_t bytes, Bitfield<MemPermission> permissions)
{
    if(bytes == 0)
    {
        return;
    }

    bytes = roundUp(bytes, MemoryPageSize());
    int result = mprotect(memory, bytes, permissionsToMmapProt(permissions));
    CHECK(result == 0);
}

void DeallocateMemoryPages(void *memory, size_t bytes)
{
    int result = mprotect(memory, bytes, PROT_READ | PROT_WRITE);
    CHECK(result == 0);
    deallocate(memory);
}

REACTOR_NAMESPACE_END
