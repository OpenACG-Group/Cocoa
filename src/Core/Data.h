#ifndef COCOA_DATA_H
#define COCOA_DATA_H

#include <memory>

#include "Core/Filesystem.h"
namespace cocoa {

class CrpkgFile;

class Data
{
public:
    Data() = default;
    virtual ~Data() = default;
    Data(const Data&) = delete;
    Data& operator=(const Data&) = delete;

    static std::shared_ptr<Data> MakeFromFile(const std::string& path,
                                              Bitfield<vfs::OpenFlags> flags,
                                              Bitfield<vfs::Mode> mode = {});
    /* `fd` will be closed when object is destructed */
    static std::shared_ptr<Data> MakeFromFile(int32_t fd, Bitfield<vfs::OpenFlags> flags);
    static std::shared_ptr<Data> MakeFromFileMapped(const std::string& path,
                                                    Bitfield<vfs::OpenFlags> flags);
    static std::shared_ptr<Data> MakeFromPackage(const std::shared_ptr<CrpkgFile>& file);
    static std::shared_ptr<Data> MakeFromPtr(void *ptr, size_t size);
    static std::shared_ptr<Data> MakeFromPtrWithoutCopy(void *ptr, size_t size, bool release = false);

    virtual size_t size() = 0;
    virtual ssize_t read(void *buffer, size_t size) = 0;
    virtual ssize_t write(const void *buffer, size_t size) = 0;
    virtual off_t tell() = 0;
    virtual off_t seek(vfs::SeekWhence whence, off_t offset) = 0;
    virtual bool hasAccessibleBuffer() {
        return false;
    }
    virtual const void *getAccessibleBuffer() {
        return nullptr;
    }
};

} // namespace cocoa
#endif //COCOA_DATA_H
