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

#ifndef COCOA_CORE_FILESYSTEM_H
#define COCOA_CORE_FILESYSTEM_H

#include <cstdint>
#include <type_traits>
#include <vector>
#include <string>

#include "EnumClassBitfield.h"

#define VFS_NS_BEGIN    namespace cocoa::vfs {
#define VFS_NS_END      }

VFS_NS_BEGIN

enum class OpenFlags : uint8_t
{
    kReadonly       = (1 << 0),
    kWriteOnly      = (1 << 1),
    kReadWrite      = (1 << 2),
    kCreate         = (1 << 3),
    kTrunc          = (1 << 4),
    kAppend         = (1 << 5)
};

enum class Mode : uint16_t
{
    kNone       = 0,
    kUsrW       = (1 << 0),
    kUsrR       = (1 << 1),
    kUsrX       = (1 << 2),
    kOthW       = (1 << 3),
    kOthR       = (1 << 4),
    kOthX       = (1 << 5),
    kGrpW       = (1 << 6),
    kGrpR       = (1 << 7),
    kGrpX       = (1 << 8),
    kDir        = (1 << 9),
    kLink       = (1 << 10),
    kRegular    = (1 << 11),
    kChar       = (1 << 12),
    kBlock      = (1 << 13),
    kFifo       = (1 << 14),
    kSocket     = (1 << 15)
};

enum class MapProtection : uint8_t
{
    kNone       = (1 << 0),
    kRead       = (1 << 1),
    kWrite      = (1 << 2),
    kExec       = (1 << 3)
};

enum class MapFlags : uint8_t
{
    kFixed      = (1 << 0),
    kShared     = (1 << 1),
    kPrivate    = (1 << 2)
};

enum class SeekWhence : uint8_t
{
    kSet,
    kCurrent,
    kEnd,
    kLastWhence = kEnd
};

enum class AccessMode : uint8_t
{
    kReadable   = (1 << 0),
    kWritable   = (1 << 1),
    kExecutable = (1 << 2),
    kExist      = (1 << 3),
    kRegular    = (1 << 4)
};

enum class AccessResult
{
    kOk,
    kFailed
};

struct Stat
{
    uint32_t        linkCount;
    Bitfield<Mode>  mode;
    uint32_t        uid;
    uint32_t        gid;
    size_t          size;
    struct timespec atime;
    struct timespec mtime;
    struct timespec ctime;
};

#define VFS_AT_FDCWD  (-1)

int32_t Open(const std::string& path, Bitfield<OpenFlags> flags,
             Bitfield<Mode> mode = Bitfield<Mode>());
int32_t OpenAt(int32_t dirfd, const std::string& path, Bitfield<OpenFlags> flags,
               Bitfield<Mode> mode = Bitfield<Mode>());
int32_t Close(int32_t fd);

int32_t Chdir(const std::string& path);
std::string ReadLink(const std::string& path);
std::string Realpath(const std::string& path);
AccessResult Access(const std::string& path, Bitfield<AccessMode> mode);
int32_t Rename(const std::string& old, const std::string& _new);
bool IsDirectory(const std::string& path);

ssize_t FileSize(int32_t fd);
ssize_t FileSize(const std::string& path);

ssize_t Read(int32_t fd, void *buffer, size_t size);
ssize_t Write(int32_t fd, const void *buffer, size_t size);
off_t Seek(int32_t fd, off_t offset, SeekWhence whence);
void *MemMap(int32_t fd, void *address, Bitfield<MapProtection> protection,
             Bitfield<MapFlags> flags, size_t size, off64_t offset);
bool MemMapHasFailed(void *ret);
int32_t MemUnmap(void *address, size_t size);

int32_t Truncate(const std::string& path, off_t length);
int32_t FTruncate(int32_t fd, off_t length);

int32_t Mknod(const std::string& path, Bitfield<Mode> mode, int32_t dev);
int32_t MknodAt(int32_t dirfd, const std::string& path, Bitfield<Mode> mode, int32_t dev);

VFS_NS_END

#endif //COCOA_CORE_FILESYSTEM_H
