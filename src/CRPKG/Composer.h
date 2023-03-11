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

#ifndef COCOA_CRPKG_COMPOSER_H
#define COCOA_CRPKG_COMPOSER_H

#include <memory>
#include <vector>
#include <functional>

#include "uv.h"

#include "Core/Data.h"
#include "CRPKG/CRPKG.h"
CRPKG_NAMESPACE_BEGIN

class Composer
{
public:
    class DataAccessor
    {
    public:
        /**
         * Compared with `Data::MakeFromFile`, this function does not open the file
         * (create a file descriptor) immediately. Instead, it opens the file only
         * when it is required, and close it when not needed.
         * Use this function to avoid opening too many file descriptors when constructing
         * entries tree.
         */
        static std::shared_ptr<DataAccessor> MakeFromFile(const std::string& path);

        static std::shared_ptr<DataAccessor> MakeDirect(std::shared_ptr<Data> data);

        using Callback = std::function<std::shared_ptr<Data>()>;
        static std::shared_ptr<DataAccessor> MakeFromCallback(Callback callback);

        DataAccessor() = default;
        virtual ~DataAccessor() = default;

        virtual std::shared_ptr<Data> Acquire() = 0;
    };

    enum Status
    {
        kSuccess_Status,
        kDataAcquireError_Status,
        kIOError_Status,
        kAsyncCancelled_Status
    };

    enum EntryType
    {
        kEmpty_EntryType,
        kFile_EntryType,
        kDirectory_EntryType
    };

    class Entry
    {
    public:
        Entry() : type(kEmpty_EntryType) {}

        // Construct a file entry
        Entry(std::string _name, std::shared_ptr<DataAccessor> _data_accessor)
            : type(kFile_EntryType), name(std::move(_name))
            , data_accessor(std::move(_data_accessor)) {}

        // Construct a directory entry
        explicit Entry(std::string _name)
            : type(kDirectory_EntryType), name(std::move(_name)) {}

        // Construct a directory entry with specified children
        Entry(std::string _name, std::vector<Entry> _children)
            : type(kDirectory_EntryType), name(std::move(_name)), children(std::move(_children)) {}

        Entry(const Entry& other) = default;
        Entry(Entry&& other) noexcept
            : type(other.type), name(std::move(other.name))
            , data_accessor(std::move(other.data_accessor))
            , children(std::move(other.children))
        {
            other.type = kEmpty_EntryType;
        }

        Entry& operator=(const Entry& other) = default;
        Entry& operator=(Entry&& other) noexcept
        {
            type = other.type;
            name = std::move(other.name);
            data_accessor = std::move(other.data_accessor);
            children = std::move(other.children);
            other.type = kEmpty_EntryType;
            return *this;
        }

        EntryType                       type;
        std::string                     name;
        std::shared_ptr<DataAccessor>   data_accessor;
        std::vector<Entry>              children;
    };

    using WriteCallback = std::function<ssize_t(const uint8_t *, size_t)>;
    using AsyncFinishCallback = std::function<void(Status)>;

    static Status Compose(const Entry& entries, const WriteCallback& writer);

    /**
     * Asynchronous version of `Compose`, composing will be executed on
     * a worker thread.
     *
     * @param loop      Thread pool.
     * @param entries   The same to `Compose`.
     * @param writer    A callback function that will be called from worker thread
     *                  to write output data.
     * @param finish    A callback function that will be called from the thread
     *                  who is running `loop` to indicate the accomplishment of composing.
     */
    static void ComposeAsync(uv_loop_t *loop,
                             const Entry& entries,
                             const WriteCallback& writer,
                             const AsyncFinishCallback& finish);
};

CRPKG_NAMESPACE_END
#endif //COCOA_CRPKG_COMPOSER_H
