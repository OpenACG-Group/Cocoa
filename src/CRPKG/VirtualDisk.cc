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

#include <stack>

#include "Core/Data.h"
#include "Core/Errors.h"
#include "Core/Journal.h"
#include "Core/Utils.h"
#include "CRPKG/CRPKG.h"
#include "CRPKG/VirtualDisk.h"
#include "CRPKG/Protocol.h"
CRPKG_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(CRPKG.VirtualDisk)

namespace {

using FlattenedDirtreeEntryList = std::vector<const proto::DirTreeFlattenedEntry*>;

class DirtreeNode
{
public:
    enum Type
    {
        kFile_Type,
        kDirectory_Type
    };

    // NOLINTNEXTLINE
    static std::unique_ptr<DirtreeNode> Build(VirtualDisk::Package *package,
                                              const FlattenedDirtreeEntryList& from,
                                              uint32_t p, uint32_t recursive_depth)
    {
        if (recursive_depth > VirtualDisk::kMaxRecursiveDepth)
        {
            QLOG(LOG_ERROR, "Exceeded the maximum recursive depth");
            return nullptr;
        }

        if (!((from[p]->flags & DT_FLAG_FILE) ^ (from[p]->flags & DT_FLAG_DIRECTORY)))
        {
            QLOG(LOG_ERROR, "Corrupted package file: invalid data flags");
            return nullptr;
        }

        auto node = std::make_unique<DirtreeNode>();
        node->package_ = package;
        node->type_ = (from[p]->flags & DT_FLAG_DIRECTORY) ?
                      kDirectory_Type : kFile_Type;
        node->name_id_ = from[p]->name;

        if (from[p]->flags & DT_FLAG_FILE)
        {
            if (from[p]->nb_children != 1)
            {
                QLOG(LOG_ERROR, "Corrupted package file: invalid nb_children");
                return nullptr;
            }
            node->data_id_ = from[p]->children[0];
            return node;
        }

        for (uint32_t i = 0; i < from[p]->nb_children; i++)
        {
            if (from[p]->children[i] >= from.size())
            {
                QLOG(LOG_ERROR, "Corrupted package file: invalid children index");
                return nullptr;
            }
            std::unique_ptr<DirtreeNode> child = Build(package, from, from[p]->children[i],
                                                       recursive_depth + 1);
            if (!child)
                return nullptr;
            node->children_.emplace_back(std::move(child));
        }

        return node;
    }

    DirtreeNode()
        : package_(nullptr), type_(kFile_Type)
        , name_id_(0), data_id_(0) {}

    VirtualDisk::Package                        *package_;
    Type                                         type_;
    uint32_t                                     name_id_;
    uint32_t                                     data_id_;
    std::vector<std::unique_ptr<DirtreeNode>>    children_;
};

struct HashedStringView
{
    explicit HashedStringView(std::string_view sv)
    {
        hash = std::hash<std::string_view>()(sv);
        view = sv;
    }

    HashedStringView() : hash(0), view() {}

    bool operator==(const HashedStringView& other) const {
        if (hash != other.hash)
            return false;
        return (view == other.view);
    }

    size_t hash;
    std::string_view view;
};

} // namespace

class VirtualDisk::Package
{
public:
    Package()
        : pkg_addr_(nullptr), pkg_size_(0) {}

    ~Package()
    {
        root_node_.reset();
    }

    static std::unique_ptr<Package> Create(const std::shared_ptr<Data>& data)
    {
        CHECK(data);

        if (!data->hasAccessibleBuffer())
            return nullptr;

        auto result = std::make_unique<Package>();
        result->data_ = data;
        result->pkg_addr_ = reinterpret_cast<const uint8_t*>(data->getAccessibleBuffer());
        result->pkg_size_ = data->size();

        return result;
    }

    bool ResolveContents()
    {
        // Read header
        if (pkg_size_ < sizeof(proto::Header))
        {
            QLOG(LOG_ERROR, "Corrupted package file");
            return false;
        }

        auto *hdr = reinterpret_cast<const proto::Header*>(pkg_addr_);
        if (std::memcmp(hdr->magic, proto::kFormatHeaderMagic, sizeof(hdr->magic)) != 0)
        {
            QLOG(LOG_ERROR, "Not a resource package");
            return false;
        }

        if (hdr->version != kVersion)
        {
            QLOG(LOG_ERROR, "Version of resource package is not supported");
            return false;
        }

        if (!BuildStringTable(hdr->GST_offset, hdr->GST_size))
            return false;
        if (!BuildDataTable(hdr->GDT_offset, hdr->GDT_size))
            return false;
        if (!BuildDirtree(hdr->dirtree_offset, hdr->dirtree_size))
            return false;

        return true;
    }

#define CHECK_ADDRESS_RANGE(x)                                      \
    if ((x) >= pkg_addr_ + pkg_size_) {                             \
        QLOG(LOG_ERROR, "Corrupted package file: unexpected EOF");  \
        return false;                                               \
    }

#define CHECK_ADDRESS_RANGE_OUT(x)                                  \
    if ((x) > pkg_addr_ + pkg_size_) {                              \
        QLOG(LOG_ERROR, "Corrupted package file: unexpected EOF");  \
        return false;                                               \
    }

    bool BuildStringTable(size_t offset, uint32_t size)
    {
        GST_.resize(size);

        const uint8_t *cur = pkg_addr_ + offset;
        for (uint32_t i = 0; i < size; i++)
        {
            CHECK_ADDRESS_RANGE(cur)
            auto *entry = reinterpret_cast<const proto::StringTableEntry*>(cur);
            GST_[i] = HashedStringView(std::string_view(entry->str, entry->length));
            cur += entry->ComputeSizeInBytes();
        }
        CHECK_ADDRESS_RANGE_OUT(cur)

        return true;
    }

    bool BuildDataTable(size_t offset, uint32_t size)
    {
        GDT_.resize(size);

        const uint8_t *cur = pkg_addr_ + offset;
        for (uint32_t i = 0; i < size; i++)
        {
            CHECK_ADDRESS_RANGE(cur)
            GDT_[i] = reinterpret_cast<const proto::DataTableEntry*>(cur);
            cur += GDT_[i]->ComputeSizeInBytes();
        }
        CHECK_ADDRESS_RANGE_OUT(cur)

        return true;
    }

    bool BuildDirtree(size_t offset, uint32_t size)
    {
        FlattenedDirtreeEntryList entries(size);

        const uint8_t *cur = pkg_addr_ + offset;
        for (uint32_t i = 0; i < size; i++)
        {
            CHECK_ADDRESS_RANGE(cur)
            entries[i] = reinterpret_cast<const proto::DirTreeFlattenedEntry*>(cur);
            cur += entries[i]->ComputeSizeInBytes();
        }
        CHECK_ADDRESS_RANGE_OUT(cur)

        root_node_ = DirtreeNode::Build(this, entries, 0, 1);
        return bool(root_node_);
    }

    g_nodiscard const HashedStringView& GetGSTStringView(uint32_t idx) const {
        CHECK(idx < GST_.size());
        return GST_[idx];
    }

    g_nodiscard const proto::DataTableEntry *GetGDTEntry(uint32_t idx) const {
        CHECK(idx < GDT_.size());
        return GDT_[idx];
    }

    g_nodiscard DirtreeNode *GetRootDirtreeNode() {
        return root_node_.get();
    }

private:
    std::shared_ptr<Data>           data_;
    const uint8_t                  *pkg_addr_;
    size_t                          pkg_size_;

    std::vector<HashedStringView>   GST_;
    std::vector<const proto::DataTableEntry*> GDT_;

    std::unique_ptr<DirtreeNode>    root_node_;
};

class VirtualDisk::VDiskDirtreeNode
{
public:
    explicit VDiskDirtreeNode(DirtreeNode *ref_node)
            : ref_node_(ref_node) {}
    ~VDiskDirtreeNode() = default;

    // NOLINTNEXTLINE
    static std::unique_ptr<VDiskDirtreeNode> Build(DirtreeNode *from)
    {
        CHECK(from);
        auto node = std::make_unique<VDiskDirtreeNode>(from);
        for (const auto& child : from->children_)
            node->children_.emplace_back(Build(child.get()));
        return node;
    }

    // NOLINTNEXTLINE
    void MergeChildren(DirtreeNode *merge)
    {
        for (const auto& merge_child : merge->children_)
        {
            Package *pkg = merge_child->package_;
            const HashedStringView& merge_name =
                    pkg->GetGSTStringView(merge_child->name_id_);

            int32_t idx = FindChildrenByName(merge_name);
            if (idx >= 0)
            {
                if (children_[idx]->ref_node_->type_ != merge_child->type_ ||
                    merge_child->type_ == DirtreeNode::kFile_Type)
                {
                    // Different entry type, the old node will be replaced with
                    // the new node directly.
                    // For file nodes, further comparison is not needed, and
                    // replace it directly too.
                    children_[idx] = Build(merge_child.get());
                }
                else
                {
                    // For directory nodes, we need to compare their children
                    // recursively.
                    children_[idx]->MergeChildren(merge_child.get());
                }
            }
            else
            {
                // A completely new node is not conflicts with existing
                // nodes, so just add it to the children list.
                children_.emplace_back(Build(merge_child.get()));
            }
        }
    }

    int32_t FindChildrenByName(const HashedStringView& other)
    {
        Package *pkg = ref_node_->package_;
        for (int32_t i = 0; i < children_.size(); i++)
        {
            const HashedStringView& view =
                    pkg->GetGSTStringView(children_[i]->ref_node_->name_id_);
            if (view == other)
                return i;
        }
        return -1;
    }

    g_nodiscard const HashedStringView& GetName() const {
        return ref_node_->package_->GetGSTStringView(ref_node_->name_id_);
    }

    g_nodiscard const std::vector<std::unique_ptr<VDiskDirtreeNode>>& GetChildren() const {
        return children_;
    }

    g_nodiscard std::optional<VirtualDisk::Storage> GetStorage() const {
        if (ref_node_->type_ != DirtreeNode::kFile_Type)
            return {};
        const proto::DataTableEntry *entry =
                ref_node_->package_->GetGDTEntry(ref_node_->data_id_);
        return Storage{entry->size, entry->data};
    }

private:
    DirtreeNode *ref_node_;
    std::vector<std::unique_ptr<VDiskDirtreeNode>> children_;
};

std::shared_ptr<VirtualDisk>
VirtualDisk::MakeLayerDisk(const DataVector& datas)
{
    if (datas.empty())
        return nullptr;

    std::vector<std::unique_ptr<Package>> pkgs;
    for (const auto& data : datas)
    {
        std::unique_ptr<Package> pkg = Package::Create(data);
        if (!pkg)
            return nullptr;
        if (!pkg->ResolveContents())
            return nullptr;
        pkgs.emplace_back(std::move(pkg));
    }

    std::unique_ptr<VDiskDirtreeNode> root =
            VDiskDirtreeNode::Build(pkgs[0]->GetRootDirtreeNode());
    for (int32_t i = 1; i < pkgs.size(); i++)
        root->MergeChildren(pkgs[i]->GetRootDirtreeNode());

    auto vdisk = std::make_shared<VirtualDisk>();
    vdisk->packages_ = std::move(pkgs);
    vdisk->dirtree_ = std::move(root);

    return vdisk;
}

std::optional<VirtualDisk::Storage>
VirtualDisk::GetStorage(const std::string_view& path)
{
    if (path.empty())
        return {};

    // Only absolute path is supported
    if (path[0] != '/')
        return {};

    std::string_view sv(path);
    while (sv[0] == '/')
        sv.remove_prefix(1);

    std::vector<std::string_view> name_seq = utils::SplitString(sv, '/');
    std::stack<VirtualDisk::VDiskDirtreeNode*> node_stack;
    node_stack.push(dirtree_.get());

    for (const auto& name : name_seq)
    {
        if (name.empty())
            continue;

        if (name == ".")
            continue;

        if (name == "..")
        {
            if (node_stack.size() <= 1)
                return {};
            node_stack.pop();
            continue;
        }

        HashedStringView name_view(name);
        int32_t child_idx = node_stack.top()->FindChildrenByName(name_view);
        if (child_idx < 0)
            return {};

        auto *next = node_stack.top()->GetChildren()[child_idx].get();
        node_stack.push(next);
    }

    return node_stack.top()->GetStorage();
}

CRPKG_NAMESPACE_END
