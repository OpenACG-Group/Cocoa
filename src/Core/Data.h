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

#ifndef COCOA_CORE_DATA_H
#define COCOA_CORE_DATA_H

#include <memory>
#include <functional>

#include "Core/Filesystem.h"
#include "Core/Project.h"
namespace cocoa {

class CrpkgFile;
class DataSlice;

class Data : public std::enable_shared_from_this<Data>
{
public:
    class ScopedSeekRewind
    {
    public:
        explicit ScopedSeekRewind(const std::shared_ptr<Data>& data)
            : data_(data), stored_offset_(data->tell()) {}

        ~ScopedSeekRewind() {
            data_->seek(vfs::SeekWhence::kSet, stored_offset_);
        }

    private:
        std::shared_ptr<Data>       data_;
        off_t                       stored_offset_;
    };

    using ExternalDeleter = std::function<void(void*)>;

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
    static std::shared_ptr<Data> MakeFromSize(size_t size);
    static std::shared_ptr<Data> MakeFromString(const char *str, bool no_terminator = false);
    static std::shared_ptr<Data> MakeLinearBuffer(const std::shared_ptr<Data>& data);
    static std::shared_ptr<Data> MakeFromExternal(void *ptr, size_t size, const ExternalDeleter& deleter);

    g_nodiscard virtual size_t size() = 0;

    virtual ssize_t read(void *buffer, size_t size) = 0;
    virtual ssize_t write(const void *buffer, size_t size) = 0;

    g_nodiscard virtual off_t tell() = 0;

    virtual off_t seek(vfs::SeekWhence whence, off_t offset) = 0;

    g_nodiscard virtual bool hasAccessibleBuffer() {
        return false;
    }

    g_nodiscard virtual const void *getAccessibleBuffer() {
        return nullptr;
    }

    g_nodiscard virtual void *takeBufferOwnership() {
        return nullptr;
    }

    g_nodiscard virtual std::shared_ptr<DataSlice> slice(size_t offset, size_t size) = 0;
};

class DataSlice
{
public:
    explicit DataSlice(std::shared_ptr<Data> owned_data);
    virtual ~DataSlice() = default;

    g_nodiscard virtual size_t size() const = 0;
    g_nodiscard virtual uint8_t at(size_t index) const = 0;

    g_nodiscard g_inline uint8_t operator[](size_t index) const {
        return this->at(index);
    }

private:
    std::shared_ptr<Data>       owned_data_;
};

} // namespace cocoa
#endif //COCOA_CORE_DATA_H
