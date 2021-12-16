#include "Core/Errors.h"
#include <cstring>

#include "Core/CrpkgImage.h"
#include "Core/Exception.h"
#include "Core/Filesystem.h"
#include "Core/Data.h"
namespace cocoa {

class FileData : public Data
{
public:
    FileData(int32_t fd, Bitfield<vfs::OpenFlags> flags)
        : fFd(fd), fFlags(flags)
    {
        CHECK(fd >= 0);
    }

    ~FileData() override
    {
        vfs::Close(fFd);
    }

    size_t size() override
    { return vfs::FileSize(fFd); }

    ssize_t read(void *buffer, size_t size) override
    {
        if (!(fFlags & vfs::OpenFlags::kReadWrite) && !(fFlags & vfs::OpenFlags::kReadonly))
            throw RuntimeException(__func__, "This data object is unreadable");
        return vfs::Read(fFd, buffer, size);
    }

    ssize_t write(const void *buffer, size_t size) override
    {
        if (!(fFlags & vfs::OpenFlags::kReadWrite) && !(fFlags & vfs::OpenFlags::kWriteOnly))
            throw RuntimeException(__func__, "This data object is not writable");
        return vfs::Write(fFd, buffer, size);
    }

    off_t seek(vfs::SeekWhence whence, off_t offset) override
    { return vfs::Seek(fFd, offset, whence); }

    off_t tell() override
    { return vfs::Seek(fFd, 0, vfs::SeekWhence::kCurrent); }

private:
    int32_t                         fFd;
    Bitfield<vfs::OpenFlags>   fFlags;
};

class PackageData : public Data
{
public:
    explicit PackageData(std::shared_ptr<CrpkgFile> file)
        : fFile(std::move(file))
    {
        CHECK(fFile != nullptr);
    }
    ~PackageData() override = default;

    size_t size() override
    {
        auto stat = fFile->stat();
        if (!stat)
            throw RuntimeException(__func__, "Failed to get file stat in crpkg");
        return stat->size;
    }

    ssize_t read(void *buffer, size_t size) override
    { return fFile->read(buffer, static_cast<ssize_t>(size)); }

    ssize_t write(const void *buffer, size_t size) override
    { throw RuntimeException(__func__, "Files in crpkg is not writable"); }

    off_t seek(vfs::SeekWhence whence, off_t offset) override
    { return fFile->seek(whence, offset); }

    off_t tell() override
    { return fFile->seek(vfs::SeekWhence::kCurrent, 0); }

private:
    std::shared_ptr<CrpkgFile>      fFile;
};

class MemoryData : public Data
{
public:
    MemoryData(void *ptr, size_t size, bool release, std::function<void(void*)> deleter)
        : fAddress(reinterpret_cast<uint8_t*>(ptr))
        , fCurPtr(fAddress)
        , fSize(size)
        , fRelease(release)
        , fDeleter(std::move(deleter))
    {
    }

    ~MemoryData() override
    {
        if (fRelease)
            fDeleter(fAddress);
    }

    size_t size() override
    { return fSize; }

    off_t tell() override
    { return fCurPtr - fAddress; }

    off_t seek(vfs::SeekWhence whence, off_t offset) override
    {
        uint8_t *gptr = fAddress;
        switch (whence)
        {
        case vfs::SeekWhence::kSet:
            gptr = fAddress + offset;
            break;
        case vfs::SeekWhence::kCurrent:
            gptr = fCurPtr + offset;
            break;
        case vfs::SeekWhence::kEnd:
            gptr = (fAddress + fSize) + offset;
            break;
        }
        if (gptr < fAddress || gptr > (fAddress + fSize))
            throw RuntimeException(__func__, "Invalid offset");
        fCurPtr = gptr;
        return (gptr - fAddress);
    }

    ssize_t read(void *buffer, size_t size) override
    {
        size_t finalSize = size;
        if (fCurPtr + size > fAddress + fSize)
            finalSize = fSize - (fCurPtr - fAddress);
        if (finalSize == 0)
            return 0;
        std::memcpy(buffer, fCurPtr, finalSize);
        fCurPtr += finalSize;
        return static_cast<ssize_t>(finalSize);
    }

    ssize_t write(const void *buffer, size_t size) override
    {
        size_t finalSize = size;
        if (fCurPtr + size > fAddress + fSize)
            finalSize = fSize - (fCurPtr - fAddress);
        if (finalSize == 0)
            return 0;
        std::memcpy(fCurPtr, buffer, finalSize);
        fCurPtr += finalSize;
        return static_cast<ssize_t>(finalSize);
    }

    bool hasAccessibleBuffer() override
    {
        return true;
    }

    const void *getAccessibleBuffer() override
    {
        return fAddress;
    }

private:
    uint8_t     *fAddress;
    uint8_t     *fCurPtr;
    size_t       fSize;
    bool         fRelease;
    std::function<void(void*)> fDeleter;
};

std::shared_ptr<Data> Data::MakeFromFileMapped(const std::string& path,
                                               Bitfield<vfs::OpenFlags> flags)
{
    if (vfs::Access(path, {vfs::AccessMode::kReadable}) != vfs::AccessResult::kOk)
        return nullptr;
    int32_t fd = vfs::Open(path, flags);
    if (fd < 0)
        return nullptr;
    size_t size = vfs::FileSize(fd);
    ScopeEpilogue scope([fd] { vfs::Close(fd); });

    Bitfield<vfs::MapProtection> mapprot;
    if (flags & vfs::OpenFlags::kReadonly)
        mapprot |= vfs::MapProtection::kRead;
    if (flags & vfs::OpenFlags::kWriteOnly)
        mapprot |= vfs::MapProtection::kWrite;
    if (flags & vfs::OpenFlags::kReadWrite)
    {
        mapprot |= vfs::MapProtection::kRead;
        mapprot |= vfs::MapProtection::kWrite;
    }

    void *ptr = vfs::MemMap(fd, nullptr, mapprot, {vfs::MapFlags::kPrivate}, size, 0);
    if (!ptr)
        return nullptr;

    return std::make_shared<MemoryData>(ptr, size, true, [size](void *ptr){
        CHECK(ptr);
        vfs::MemUnmap(ptr, size);
    });
}

std::shared_ptr<Data> Data::MakeFromFile(const std::string& path,
                                         Bitfield<vfs::OpenFlags> flags,
                                         Bitfield<vfs::Mode> mode)
{
    int32_t fd = vfs::Open(path, flags, mode);
    if (fd < 0)
        return nullptr;
    return std::make_shared<FileData>(fd, flags);
}

std::shared_ptr<Data> Data::MakeFromFile(int32_t fd, Bitfield<vfs::OpenFlags> flags)
{
    if (fd < 0)
        return nullptr;
    return std::make_shared<FileData>(fd, flags);
}

std::shared_ptr<Data> Data::MakeFromPackage(const std::shared_ptr<CrpkgFile>& file)
{
    if (file == nullptr)
        return nullptr;
    return std::make_shared<PackageData>(file);
}

std::shared_ptr<Data> Data::MakeFromPtr(void *ptr, size_t size)
{
    void *ptrdup = std::malloc(size);
    if (!ptrdup)
        return nullptr;
    std::memcpy(ptrdup, ptr, size);
    return std::make_shared<MemoryData>(ptrdup, size, true, [](void *ptr) {
        std::free(ptr);
    });
}

std::shared_ptr<Data> Data::MakeFromPtrWithoutCopy(void *ptr, size_t size, bool release)
{
    if (!ptr)
        return nullptr;
    return std::make_shared<MemoryData>(ptr, size, release, [](void *ptr) {
        std::free(ptr);
    });
}

} // namespace cocoa
