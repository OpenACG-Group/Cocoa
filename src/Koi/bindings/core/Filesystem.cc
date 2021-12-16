#include <cerrno>
#include <cstdlib>

#include "Core/Filesystem.h"
#include "Koi/bindings/core/Exports.h"
#include "Koi/bindings/core/FdRandomize.h"

KOI_BINDINGS_NS_BEGIN

Bitfield<vfs::OpenFlags> resolveOpenFlags(const std::string& _flags)
{
    Bitfield<vfs::OpenFlags> flags;
    bool fR = false, fW = false;
    for (char p : _flags)
    {
        switch (p)
        {
        case 'r': fR = true; break;
        case 'w': fW = true; break;
        case '+': flags |= vfs::OpenFlags::kCreate; break;
        case 'a': flags |= vfs::OpenFlags::kAppend; break;
        case 't': flags |= vfs::OpenFlags::kTrunc; break;
        default:
            JS_THROW_RET_IF(true, "Bad open mode", {}, v8::Exception::Error);
        }
    }
    if (fR && fW)
        flags |= vfs::OpenFlags::kReadWrite;
    else if (fR)
        flags |= vfs::OpenFlags::kReadonly;
    else if (fW)
        flags |= vfs::OpenFlags::kWriteOnly;
    return flags;
}

int32_t coreOpen(const std::string& path, const std::string& _flags, int32_t mode)
{
    auto flags = resolveOpenFlags(_flags);
    if (flags.isEmpty())
        return -1;
    int32_t fd = vfs::Open(path, flags, Bitfield<vfs::Mode>(mode));
    JS_THROW_RET_IF(fd < 0, ::strerror(errno), -1, v8::Exception::Error);
    return FDLRNewRandomizedDescriptor(fd, FDLRTable::kUser, [](int32_t fd) {
        vfs::Close(fd);
    });
}

int32_t resolveRealDirFd(int32_t _dirfd)
{
    if (_dirfd == VFS_AT_FDCWD)
        return VFS_AT_FDCWD;
    auto *p = FDLRGetUnderlyingDescriptor(_dirfd);
    if (!p)
        return -1;
    return p->fd;
}

int32_t coreOpenAt(int32_t dirfd, const std::string& path, const std::string& _flags, int32_t mode)
{
    int32_t realfd = resolveRealDirFd(dirfd);
    JS_THROW_RET_IF(realfd < 0, "Corrupted file (directory) descriptor", -1, v8::Exception::Error);
    auto flags = resolveOpenFlags(_flags);
    if (flags.isEmpty())
        return -1;
    int32_t fd = vfs::OpenAt(realfd, path, flags, Bitfield<vfs::Mode>(mode));
    JS_THROW_RET_IF(fd < 0, ::strerror(errno), -1, v8::Exception::Error);
    return FDLRNewRandomizedDescriptor(fd, FDLRTable::kUser, [](int32_t fd) {
        vfs::Close(fd);
    });
}

void coreClose(int32_t fd)
{
    FDLRTable::Target *target = FDLRGetUnderlyingDescriptor(fd);
    JS_THROW_IF(!target, "Corrupted file descriptor", v8::Exception::Error);
    JS_THROW_IF(!target->closer, "File descriptor is not closable", v8::Exception::Error);
    target->closer(target->fd);
    FDLRMarkUnused(fd);
}

int64_t coreSeek(int32_t fd, int32_t whence, int64_t offset)
{
    FDLRTable::Target *target = FDLRGetUnderlyingDescriptor(fd);
    JS_THROW_RET_IF(!target, "Corrupted file descriptor", -1, v8::Exception::Error);
    JS_THROW_RET_IF(whence < 0 || whence > static_cast<uint8_t>(vfs::SeekWhence::kLastWhence),
                    "Invalid argument #2", -1, v8::Exception::Error);
    auto ret = vfs::Seek(target->fd, offset, static_cast<vfs::SeekWhence>(whence));
    JS_THROW_RET_IF(ret < 0, ::strerror(errno), -1, v8::Exception::Error);
    return ret;
}

void coreRename(const std::string& oldpath, const std::string& newpath)
{
    JS_THROW_IF(vfs::Rename(oldpath, newpath) < 0, ::strerror(errno), v8::Exception::Error);
}

void coreTruncate(const std::string& path, size_t length)
{
    JS_THROW_IF(vfs::Truncate(path, length) < 0, ::strerror(errno), v8::Exception::Error);
}

void coreFTruncate(int32_t fd, size_t length)
{
    auto *target = FDLRGetUnderlyingDescriptor(fd);
    JS_THROW_IF(!target, "Corrupted file descriptor", v8::Exception::Error);
    JS_THROW_IF(vfs::FTruncate(target->fd, length) < 0, ::strerror(errno), v8::Exception::Error);
}

void coreMknod(const std::string& path, int32_t mode, int32_t dev)
{
    Bitfield<vfs::Mode> modeBits(mode);
    if (!(modeBits & vfs::Mode::kChar || modeBits & vfs::Mode::kBlock))
        dev = 0;
    JS_THROW_IF(vfs::Mknod(path, modeBits, dev) < 0, ::strerror(errno), v8::Exception::Error);
}

void coreMknodAt(int32_t dirfd, const std::string& path, int32_t mode, int32_t dev)
{
    Bitfield<vfs::Mode> modeBits(mode);
    if (!(modeBits & vfs::Mode::kChar || modeBits & vfs::Mode::kBlock))
        dev = 0;
    int32_t realfd = resolveRealDirFd(dirfd);
    JS_THROW_IF(realfd < 0, "Corrupted file (directory) descriptor", v8::Exception::Error);
    JS_THROW_IF(vfs::MknodAt(realfd, path, modeBits, dev), ::strerror(errno), v8::Exception::Error);
}

KOI_BINDINGS_NS_END
