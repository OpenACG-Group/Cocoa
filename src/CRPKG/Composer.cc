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
#include <list>
#include <functional>

#include "Core/Errors.h"
#include "CRPKG/Composer.h"
#include "CRPKG/Protocol.h"
CRPKG_NAMESPACE_BEGIN

namespace {

struct HashedStringTableEntry
{
    explicit HashedStringTableEntry(const std::string_view& str)
    {
        hash = std::hash<std::string_view>()(str);
        entry = proto::StringTableEntry::Allocate(str);
        view = std::string_view(entry->str, entry->length);
    }

    proto::StringTableEntry::Ptr entry;
    size_t hash;
    std::string_view view;
};

class HashedStringTable
{
public:
    HashedStringTable() = default;
    ~HashedStringTable() = default;

    uint32_t AddOrGetString(const std::string_view& str)
    {
        int32_t idx = FindStringIndex(str);
        if (idx >= 0)
            return idx;

        // Insert a new string entry
        entries_.emplace_back(str);
        return entries_.size() - 1;
    }

    int32_t FindStringIndex(const std::string_view& str)
    {
        const size_t hash = std::hash<std::string_view>()(str);
        for (int32_t i = 0; i < entries_.size(); i++)
        {
            if (entries_[i].hash == hash &&
                entries_[i].view == str)
            {
                return i;
            }
        }
        return -1;
    }

    std::vector<HashedStringTableEntry>& GetEntries() {
        return entries_;
    }

    g_nodiscard size_t ComputeTableSizeInBytes() const
    {
        size_t result = 0;
        for (const auto& entry : entries_)
            result += entry.entry->ComputeSizeInBytes();
        return result;
    }

private:
    std::vector<HashedStringTableEntry> entries_;
};

class DataTable
{
public:
    DataTable() = default;
    ~DataTable() = default;

    uint32_t AddOrGetData(const std::shared_ptr<Composer::DataAccessor>& data)
    {
        CHECK(data);
        for (uint32_t i = 0; i < datas_.size(); i++)
        {
            if (datas_[i] == data)
                return i;
        }

        // Insert a new data entry
        datas_.emplace_back(data);
        return datas_.size() - 1;
    }

    g_nodiscard std::vector<std::shared_ptr<Composer::DataAccessor>>& GetEntries() {
        return datas_;
    }

private:
    std::vector<std::shared_ptr<Composer::DataAccessor>> datas_;
};

class FlattenableEntry
{
public:
    explicit FlattenableEntry(const Composer::Entry& from,
                              HashedStringTable& string_table,
                              DataTable& data_table)
        : type(from.type)
        , name_strtbl_idx(0)
        , datatbl_idx(0)
    {
        CHECK(type != Composer::kEmpty_EntryType);

        name_strtbl_idx = string_table.AddOrGetString(from.name);
        if (type == Composer::kFile_EntryType)
            datatbl_idx = data_table.AddOrGetData(from.data_accessor);

        for (const auto& child : from.children)
            children.emplace_back(child, string_table, data_table);
    }

    // NOLINTNEXTLINE
    uint32_t CountEntries() const
    {
        uint32_t count = 1;
        for (const auto& child : children)
            count += child.CountEntries();

        return count;
    }

    // Returns the number of elements in `result` filled by `Flatten`
    // NOLINTNEXTLINE
    uint32_t Flatten(std::vector<proto::DirTreeFlattenedEntry::Ptr>& result, uint32_t pos)
    {
        result[pos] = proto::DirTreeFlattenedEntry::Allocate(
                (type == Composer::kFile_EntryType) ? 1 : children.size());

        result[pos]->name = name_strtbl_idx;
        result[pos]->flags = (type == Composer::kDirectory_EntryType) ?
                             DT_FLAG_DIRECTORY : DT_FLAG_FILE;

        if (result[pos]->flags & DT_FLAG_FILE)
        {
            result[pos]->children[0] = datatbl_idx;
            return 1;
        }

        uint32_t cur = pos + 1;
        for (uint32_t i = 0; i < children.size(); i++)
        {
            result[pos]->children[i] = cur;
            cur += children[i].Flatten(result, cur);
        }

        return cur - pos;
    }

    Composer::EntryType           type;
    std::vector<FlattenableEntry> children;
    uint32_t                      name_strtbl_idx;
    uint32_t                      datatbl_idx;
};

} // namespace anonymous

class FileDataAccessor : public Composer::DataAccessor
{
public:
    explicit FileDataAccessor(std::string path)
        : path_(std::move(path)) {}
    ~FileDataAccessor() override = default;

    std::shared_ptr<Data> Acquire() override {
        return Data::MakeFromFile(path_, {vfs::OpenFlags::kReadonly});
    }

private:
    std::string path_;
};

class DirectDataAccessor : public Composer::DataAccessor
{
public:
    explicit DirectDataAccessor(std::shared_ptr<Data> data)
        : data_(std::move(data)) {}
    ~DirectDataAccessor() override = default;

    std::shared_ptr<Data> Acquire() override {
        return data_;
    }

private:
    std::shared_ptr<Data> data_;
};

class CallbackDataAccessor : public Composer::DataAccessor
{
public:
    explicit CallbackDataAccessor(Callback callback)
        : callback_(std::move(callback)) { CHECK(callback_); }
    ~CallbackDataAccessor() override = default;

    std::shared_ptr<Data> Acquire() override {
        return callback_();
    }

private:
    Callback callback_;
};

std::shared_ptr<Composer::DataAccessor>
Composer::DataAccessor::MakeFromFile(const std::string& path)
{
    return std::make_shared<FileDataAccessor>(path);
}

std::shared_ptr<Composer::DataAccessor>
Composer::DataAccessor::MakeDirect(std::shared_ptr<Data> data)
{
    return std::make_shared<DirectDataAccessor>(std::move(data));
}

std::shared_ptr<Composer::DataAccessor>
Composer::DataAccessor::MakeFromCallback(Callback callback)
{
    return std::make_shared<CallbackDataAccessor>(std::move(callback));
}

Composer::Status Composer::Compose(const Entry& entries, const WriteCallback& writer)
{
    HashedStringTable str_table;
    DataTable data_table;

    // Convert to flattenable entry tree, and build string and data table
    FlattenableEntry flattenable(entries, str_table, data_table);

    // Flatten the directory tree
    uint32_t nb_entries = flattenable.CountEntries();
    std::vector<proto::DirTreeFlattenedEntry::Ptr> dir_tree(nb_entries);
    flattenable.Flatten(dir_tree, 0);

    // Compute offsets
    std::unique_ptr<proto::Header> hdr = proto::Header::Allocate();
    hdr->GST_offset = sizeof(proto::Header);
    hdr->GST_size = str_table.GetEntries().size();

    hdr->dirtree_offset = hdr->GST_offset + str_table.ComputeTableSizeInBytes();
    hdr->dirtree_size = nb_entries;

    hdr->GDT_offset = hdr->dirtree_offset;
    for (const auto& entry : dir_tree)
        hdr->GDT_offset += entry->ComputeSizeInBytes();
    hdr->GDT_size = data_table.GetEntries().size();

#define WR(ptr, size)                                                 \
    if (writer(reinterpret_cast<const uint8_t*>(ptr), size) < 0) {    \
        return kIOError_Status;                                       \
    }

    // Write header
    WR(hdr.get(), sizeof(proto::Header))

    // Write string table
    for (const auto& entry : str_table.GetEntries())
        WR(entry.entry.get(), entry.entry->ComputeSizeInBytes())

    // Write dirtree
    for (const auto& entry : dir_tree)
        WR(entry.get(), entry->ComputeSizeInBytes())

    // Write data table
    for (const auto& entry : data_table.GetEntries())
    {
        std::shared_ptr<Data> data = entry->Acquire();
        if (!data)
            return kDataAcquireError_Status;

        uint64_t size = data->size();
        WR(&size, sizeof(uint64_t))

        if (data->size() == 0)
            continue;

        if (data->hasAccessibleBuffer())
        {
            WR(data->getAccessibleBuffer(), data->size())
        }
        else
        {
            auto buffer = std::make_unique<uint8_t[]>(1024);

            ssize_t read_bytes = data->read(buffer.get(), 1024);
            while (read_bytes > 0)
            {
                WR(buffer.get(), read_bytes)
                read_bytes = data->read(buffer.get(), 1024);
            }
        }
    }

#undef WR

    return kSuccess_Status;
}

namespace {

struct AsyncContext
{
    Composer::Entry entries;
    Composer::WriteCallback writer;
    Composer::AsyncFinishCallback finish;
    Composer::Status status;
    uv_work_t req;
};

} // namespace anonymous

void Composer::ComposeAsync(uv_loop_t *loop,
                            const Entry& entries,
                            const WriteCallback& writer,
                            const AsyncFinishCallback& finish)
{
    CHECK(loop);
    CHECK(writer);
    CHECK(finish);

    auto *async_ctx = new AsyncContext{
        .entries = entries,
        .writer = writer,
        .finish = finish,
        .req = {}
    };
    async_ctx->req.data = async_ctx;

    uv_queue_work(loop, &async_ctx->req, [](uv_work_t *work) {
        auto *async_ctx = reinterpret_cast<AsyncContext*>(work->data);
        async_ctx->status = Composer::Compose(async_ctx->entries, async_ctx->writer);
    }, [](uv_work_t *work, int status) {
        auto *async_ctx = reinterpret_cast<AsyncContext*>(work->data);
        if (status == UV_ECANCELED)
            async_ctx->status = Composer::kAsyncCancelled_Status;
        async_ctx->finish(async_ctx->status);
        delete async_ctx;
    });
}

CRPKG_NAMESPACE_END
