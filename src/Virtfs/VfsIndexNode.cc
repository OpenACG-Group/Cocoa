#include "Core/Exception.h"
#include "Virtfs/VfsIndexNode.h"
VIRTFS_NS_BEGIN

VfsIndexNode::VfsIndexNode(InodeId id,
                           InodeId parent,
                           std::vector<InodeId> children,
                           NodeType type,
                           std::string name,
                           std::string owner,
                           std::string group,
                           PermFlags flags)
    : fInodeId(id),
      fInodeType(type),
      fParentId(parent),
      fInodeName(std::move(name)),
      fOwnerName(std::move(owner)),
      fGroupName(std::move(group)),
      fPermFlags(flags)
{
    for (InodeId child : children)
        fChildIds.push_back(child);
}

void VfsIndexNode::setOwner(const std::string& owner)
{
    fOwnerName = owner;
    onSetOwner(owner);
}

void VfsIndexNode::setGroup(const std::string& group)
{
    fGroupName = group;
    onSetGroup(group);
}

void VfsIndexNode::setPermFlags(PermFlags newFlags)
{
    fPermFlags = newFlags;
    onSetPermFlags(newFlags);
}

void VfsIndexNode::rename(const std::string& name)
{
    fInodeName = name;
    onSetName(name);
}

void VfsIndexNode::remove()
{
    onRemove();
    VfsIndexNode::RemoveInodeEntry(fInodeId);
}

Handle<VfsIndexNode> VfsIndexNode::create(const std::string& name, PermFlags flags)
{
    Handle<VfsIndexNode> inode = onCreate(name, flags);
    AppendInodeEntry(inode);
    return inode;
}

Handle<VfsIndexNode> VfsIndexNode::mkdir(const std::string& name, PermFlags flags)
{
    Handle<VfsIndexNode> inode = onMkdir(name, flags);
    AppendInodeEntry(inode);
    return inode;
}

void VfsIndexNode::permissionDenied()
{
    throw RuntimeException::Builder(__FUNCTION__)
            .append("Virtfs: Permission Denied")
            .make<RuntimeException>();
}

void VfsIndexNode::appendChild(InodeId id)
{
    fChildIds.push_back(id);
}

VIRTFS_NS_END
