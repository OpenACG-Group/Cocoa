#include <map>

#include "Core/Exception.h"
#include "Virtfs/VfsIndexNode.h"
VIRTFS_NS_BEGIN

namespace {

struct VfsIndexNodeTable
{
    VfsIndexNode::InodeId idCounter = 0;
    std::map<VfsIndexNode::InodeId, Handle<VfsIndexNode>> table;
} inodeTable{};

} // namespace anonymous

VfsIndexNode::InodeId VfsIndexNode::AllocInodeId()
{
    return inodeTable.idCounter++;
}

Handle<VfsIndexNode> VfsIndexNode::GetInodeHandle(InodeId id)
{
    if (!inodeTable.table.contains(id))
        return nullptr;
    return inodeTable.table[id];
}

void VfsIndexNode::AppendInodeEntry(const Handle<VfsIndexNode>& inode)
{
    if (inodeTable.table.contains(inode->inodeId()))
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("Virtfs: inode conflict, inode ID = ")
                .append(inode->inodeId())
                .make<RuntimeException>();
    }
    inodeTable.table[inode->inodeId()] = inode;
}

void VfsIndexNode::RemoveInodeEntry(InodeId id)
{
    inodeTable.table.erase(id);
}

VIRTFS_NS_END
