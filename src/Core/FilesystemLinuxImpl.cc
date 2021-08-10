#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <linux/limits.h>

#include <vector>

#include "Core/Filesystem.h"
VFS_NS_BEGIN

namespace {

int32_t ModeFlagsToNative(Bitfield<Mode> mode)
{
    int32_t p = 0;
    p |= (mode & Mode::kUsrR) ? S_IRUSR : 0;
    p |= (mode & Mode::kUsrW) ? S_IWUSR : 0;
    p |= (mode & Mode::kUsrX) ? S_IXUSR : 0;
    p |= (mode & Mode::kOthR) ? S_IROTH : 0;
    p |= (mode & Mode::kOthW) ? S_IWOTH : 0;
    p |= (mode & Mode::kOthX) ? S_IXOTH : 0;
    p |= (mode & Mode::kGrpR) ? S_IRGRP : 0;
    p |= (mode & Mode::kGrpW) ? S_IWGRP : 0;
    p |= (mode & Mode::kGrpX) ? S_IXGRP : 0;
    return p;
}

} // namespace anonymous

int32_t Open(const std::string& path, Bitfield<OpenFlags> flags, Bitfield<Mode> mode)
{
    int32_t iFlags = 0, iMode = 0;
    iFlags |= (flags & OpenFlags::kWriteOnly) ? O_WRONLY : 0;
    iFlags |= (flags & OpenFlags::kReadWrite) ? O_RDWR : 0;
    iFlags |= (flags & OpenFlags::kCreate) ? O_CREAT : 0;
    iFlags |= (flags & OpenFlags::kTrunc) ? O_TRUNC : 0;
    iFlags |= (flags & OpenFlags::kAppend) ? O_APPEND : 0;
    iMode = ModeFlagsToNative(mode);
    return open(path.c_str(), iFlags, iMode);
}

int32_t Close(int32_t fd)
{
    return close(fd);
}

int32_t Chdir(const std::string& path)
{
    return chdir(path.c_str());
}

std::string ReadLink(const std::string& path)
{
    std::vector<char> buf(1024, 0);
    auto size = buf.size();
    bool havePath = false;
    bool shouldContinue = true;
    do
    {
        ssize_t result = readlink(path.c_str(), &buf[0], size);
        if (result < 0)
            shouldContinue = false;
        else if (static_cast<decltype(size)>(result) < size)
        {
            havePath = true;
            shouldContinue = false;
            size = result;
        }
        else
        {
            size *= 2;
            buf.resize(size);
            std::fill(std::begin(buf), std::end(buf), 0);
        }
    } while (shouldContinue);
    if (!havePath)
        return "";

    return std::string(&buf[0], size);
}

namespace {
thread_local char gRealpathBuffer[PATH_MAX];
} // namespace anonymous
std::string Realpath(const std::string& path)
{
    realpath(path.c_str(), gRealpathBuffer);
    return std::string(gRealpathBuffer);
}

AccessResult Access(const std::string& path, Bitfield<AccessMode> mode)
{
    int32_t iMode = 0;
    iMode |= (mode & AccessMode::kReadable) ? R_OK : 0;
    iMode |= (mode & AccessMode::kWritable) ? W_OK : 0;
    iMode |= (mode & AccessMode::kExecutable) ? X_OK : 0;

    return (!access(path.c_str(), iMode)) ? AccessResult::kOk : AccessResult::kFailed;
}

int32_t Rename(const std::string& old, const std::string& _new)
{
    return rename(old.c_str(), _new.c_str());
}

ssize_t FileSize(int32_t fd)
{
    struct stat stbuf{};
    if (fstat(fd, &stbuf) < 0)
        return -1;
    return stbuf.st_size;
}

ssize_t FileSize(const std::string& path)
{
    struct stat stbuf{};
    if (stat(path.c_str(), &stbuf) < 0)
        return -1;
    return stbuf.st_size;
}

ssize_t Read(int32_t fd, void *buffer, size_t size)
{
    return read(fd, buffer, size);
}

ssize_t Write(int32_t fd, const void *buffer, size_t size)
{
    return write(fd, buffer, size);
}

off_t Seek(int32_t fd, off_t offset, SeekWhence whence)
{
    int32_t iWhence;
    switch (whence)
    {
    case SeekWhence::kSet:
        iWhence = SEEK_SET;
        break;
    case SeekWhence::kCurrent:
        iWhence = SEEK_CUR;
        break;
    case SeekWhence::kEnd:
        iWhence = SEEK_END;
        break;
    }
    return lseek(fd, offset, iWhence);

}

void *MemMap(int32_t fd, void *address, Bitfield<MapProtection> protection,
             Bitfield<MapFlags> flags, size_t size, off64_t offset)
{
    int32_t iProtection = 0, iFlags = 0;
    iProtection |= (protection & MapProtection::kRead) ? PROT_READ : 0;
    iProtection |= (protection & MapProtection::kWrite) ? PROT_WRITE : 0;
    iProtection |= (protection & MapProtection::kExec) ? PROT_EXEC : 0;
    iFlags |= (flags & MapFlags::kFixed) ? MAP_FIXED : 0;
    iFlags |= (flags & MapFlags::kShared) ? MAP_SHARED : 0;
    iFlags |= (flags & MapFlags::kPrivate) ? MAP_PRIVATE : 0;

    void *ptr = mmap(address, size, iProtection, iFlags, fd, offset);
    return (ptr == MAP_FAILED) ? nullptr : ptr;
}

int32_t MemUnmap(void *address, size_t size)
{
    return munmap(address, size);
}

VFS_NS_END
