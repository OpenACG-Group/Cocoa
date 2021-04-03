#include "Virtfs/Virtfs.h"
#include "Virtfs/VfsDentry.h"

#include <utility>
#include "Virtfs/VfsBackendDriver.h"
#include "Virtfs/VfsMountPoint.h"
VIRTFS_NS_BEGIN

namespace {

Handle<VfsDentry> rootDentry = nullptr;

} // namespace anonymous

void VfsDentry::SetRoot(Handle<VfsDentry> dentry)
{
    rootDentry = std::move(dentry);
}

Handle<VfsDentry> VfsDentry::GetRoot()
{
    return rootDentry;
}

Handle<VfsDentry> VfsDentry::Resolve(const std::string& path)
{
    
}

VfsDentry::VfsDentry(VfsIndexNode::InodeId linkedInode,
                     WeakHandle<VfsDentry> parent,
                     Handle<VfsMountPoint> mount)
    : fLinkedInode(linkedInode),
      fParentDentry(std::move(parent)),
      fMount(std::move(mount))
{
}

VfsDentry::~VfsDentry()
{
}

WeakHandle<VfsDentry> VfsDentry::parent()
{
    return fParentDentry;
}

void VfsDentry::appendChild(Handle<VfsDentry> child)
{
    fChildren.emplace_back(std::move(child));
}

Handle<VfsIndexNode> VfsDentry::inode()
{
    return VfsIndexNode::GetInodeHandle(fLinkedInode);
}

Handle<VfsMountPoint> VfsDentry::mountpoint()
{
    return fMount;
}

VIRTFS_NS_END
