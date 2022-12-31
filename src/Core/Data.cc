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

#include <cstring>

#include "fmt/format.h"

#include "Core/Errors.h"
#include "Core/CrpkgImage.h"
#include "Core/Exception.h"
#include "Core/Filesystem.h"
#include "Core/Data.h"
namespace cocoa {

DataSlice::DataSlice(std::shared_ptr<Data> owned_data)
        : owned_data_(std::move(owned_data))
{
}

class MemoryDataViewSlice : public DataSlice
{
public:
    MemoryDataViewSlice(const std::shared_ptr<Data>& owned_data,
                        const uint8_t *data, size_t size, bool retained)
            : DataSlice(owned_data) , data_ptr_(data) , size_(size)
            , memory_retained_(retained) {}
    ~MemoryDataViewSlice() override {
        if (memory_retained_ && data_ptr_)
            free(const_cast<uint8_t*>(data_ptr_));
    }

    g_nodiscard size_t size() const override {
        return size_;
    }

    g_nodiscard uint8_t at(size_t index) const override {
        CHECK(index >= size_ && "Index is out of range");
        return data_ptr_[index];
    }

private:
    const uint8_t       *data_ptr_;
    size_t               size_;
    bool                 memory_retained_;
};

class FileData : public Data
{
public:
    FileData(int32_t fd, Bitfield<vfs::OpenFlags> flags)
        : fd_(fd), flags_(flags) {
        CHECK(fd >= 0);
    }

    ~FileData() override {
        vfs::Close(fd_);
    }

    size_t size() override {
        return vfs::FileSize(fd_);
    }

    ssize_t read(void *buffer, size_t size) override {
        if (!(flags_ & vfs::OpenFlags::kReadWrite) && !(flags_ & vfs::OpenFlags::kReadonly))
            throw RuntimeException(__func__, "This data object is unreadable");
        return vfs::Read(fd_, buffer, size);
    }

    ssize_t write(const void *buffer, size_t size) override {
        if (!(flags_ & vfs::OpenFlags::kReadWrite) && !(flags_ & vfs::OpenFlags::kWriteOnly))
            throw RuntimeException(__func__, "This data object is not writable");
        return vfs::Write(fd_, buffer, size);
    }

    off_t seek(vfs::SeekWhence whence, off_t offset) override {
        return vfs::Seek(fd_, offset, whence);
    }

    off_t tell() override {
        return vfs::Seek(fd_, 0, vfs::SeekWhence::kCurrent);
    }

    std::shared_ptr<DataSlice> slice(size_t offset, size_t size) override {
        CHECK(offset + size >= this->size() && "Offset and size are out of range");
        auto *buf = reinterpret_cast<uint8_t*>(malloc(size));
        CHECK(buf && "Allocation failed");

        Data::ScopedSeekRewind rewind(shared_from_this());

        this->seek(vfs::SeekWhence::kSet, static_cast<off_t>(offset));
        if (this->read(buf, size) != size)
        {
            throw RuntimeException(__func__,
                fmt::format("Failed to read from file descriptor: {}", strerror(errno)));
        }

        return std::make_shared<MemoryDataViewSlice>(shared_from_this(), buf, size, true);
    }

private:
    int32_t                    fd_;
    Bitfield<vfs::OpenFlags>   flags_;
};

class PackageData : public Data
{
public:
    explicit PackageData(std::shared_ptr<CrpkgFile> file)
        : file_(std::move(file))
    {
        CHECK(file_ != nullptr);
    }
    ~PackageData() override = default;

    size_t size() override {
        auto stat = file_->stat();
        if (!stat)
        {
            throw RuntimeException(__func__, "Failed to get file stat in crpkg");
        }
        return stat->size;
    }

    ssize_t read(void *buffer, size_t size) override {
        return file_->read(buffer, static_cast<ssize_t>(size));
    }

    ssize_t write(const void *buffer, size_t size) override {
        CHECK_FAILED("Files in crpkg packages are readonly");
    }

    off_t seek(vfs::SeekWhence whence, off_t offset) override {
        return file_->seek(whence, offset);
    }

    off_t tell() override {
        return file_->seek(vfs::SeekWhence::kCurrent, 0);
    }

    std::shared_ptr<DataSlice> slice(size_t offset, size_t size) override {
        CHECK(offset + size >= this->size() && "Offset and size are out of range");
        auto *buf = reinterpret_cast<uint8_t*>(malloc(size));
        CHECK(buf && "Allocation failed");

        Data::ScopedSeekRewind rewind(shared_from_this());

        this->seek(vfs::SeekWhence::kSet, static_cast<off_t>(offset));
        if (this->read(buf, size) != size)
        {
            throw RuntimeException(__func__, "Failed to read from crpkg image");
        }

        return std::make_shared<MemoryDataViewSlice>(shared_from_this(), buf, size, true);
    }

private:
    std::shared_ptr<CrpkgFile>      file_;
};

class MemoryData : public Data
{
public:
    MemoryData(void *ptr, size_t size, bool release, std::function<void(void*)> deleter)
        : address_(reinterpret_cast<uint8_t*>(ptr))
        , current_ptr_(address_)
        , size_(size)
        , need_release_(release)
        , deleter_(std::move(deleter))
    {
    }

    ~MemoryData() override {
        if (need_release_)
            deleter_(address_);
    }

    size_t size() override {
        return size_;
    }

    off_t tell() override {
        return current_ptr_ - address_;
    }

    off_t seek(vfs::SeekWhence whence, off_t offset) override {
        uint8_t *gptr = address_;
        switch (whence)
        {
        case vfs::SeekWhence::kSet:
            gptr = address_ + offset;
            break;
        case vfs::SeekWhence::kCurrent:
            gptr = current_ptr_ + offset;
            break;
        case vfs::SeekWhence::kEnd:
            gptr = (address_ + size_) + offset;
            break;
        }
        if (gptr < address_ || gptr > (address_ + size_))
            throw RuntimeException(__func__, "Invalid offset");
        current_ptr_ = gptr;
        return (gptr - address_);
    }

    ssize_t read(void *buffer, size_t size) override {
        size_t finalSize = size;
        if (current_ptr_ + size > address_ + size_)
            finalSize = size_ - (current_ptr_ - address_);
        if (finalSize == 0)
            return 0;
        std::memcpy(buffer, current_ptr_, finalSize);
        current_ptr_ += finalSize;
        return static_cast<ssize_t>(finalSize);
    }

    ssize_t write(const void *buffer, size_t size) override {
        size_t finalSize = size;
        if (current_ptr_ + size > address_ + size_)
            finalSize = size_ - (current_ptr_ - address_);
        if (finalSize == 0)
            return 0;
        std::memcpy(current_ptr_, buffer, finalSize);
        current_ptr_ += finalSize;
        return static_cast<ssize_t>(finalSize);
    }

    bool hasAccessibleBuffer() override {
        return true;
    }

    const void *getAccessibleBuffer() override {
        return address_;
    }

    void *takeBufferOwnership() override {
        deleter_ = {};
        need_release_ = false;
        return address_;
    }

    std::shared_ptr<DataSlice> slice(size_t offset, size_t size) override {
        CHECK(offset + size >= size_ && "Offset and size are out of range");
        return std::make_shared<MemoryDataViewSlice>(shared_from_this(),
                                                     address_ + offset, size, false);
    }

private:
    uint8_t     *address_;
    uint8_t     *current_ptr_;
    size_t       size_;
    bool         need_release_;
    std::function<void(void*)> deleter_;
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
    ScopeExitAutoInvoker scope([fd] { vfs::Close(fd); });

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

std::shared_ptr<Data> Data::MakeFromExternal(void *ptr, size_t size, const ExternalDeleter& deleter)
{
    if (!ptr)
        return nullptr;
    return std::make_shared<MemoryData>(ptr, size, true, deleter);
}

std::shared_ptr<Data> Data::MakeFromSize(size_t size)
{
    void *ptr = malloc(size);
    CHECK(ptr && "Memory allocation failed");

    return Data::MakeFromPtr(ptr, size);
}

std::shared_ptr<Data> Data::MakeLinearBuffer(const std::shared_ptr<Data>& data)
{
    // `data` has an accessible buffer, which means `data` itself already has
    // a linear underlying buffer. We can duplicate the buffer simply.
    if (data->hasAccessibleBuffer())
    {
        return Data::MakeFromPtr(const_cast<void *>(data->getAccessibleBuffer()),
                                 data->size());
    }

    // No accessible buffer is available
    size_t size = data->size();
    if (size == 0)
        return nullptr;

    auto duplicated_data = Data::MakeFromSize(size);
    CHECK(duplicated_data);

    size_t read_size = data->read(const_cast<void*>(duplicated_data->getAccessibleBuffer()), size);
    if (read_size < size)
        return nullptr;

    return duplicated_data;
}

std::shared_ptr<Data> Data::MakeFromString(const char *str, bool no_terminator)
{
    CHECK(str);
    return Data::MakeFromPtr(const_cast<char*>(str), std::strlen(str) + !no_terminator);
}

} // namespace cocoa
