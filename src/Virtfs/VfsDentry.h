#ifndef COCOA_VFSDENTRY_H
#define COCOA_VFSDENTRY_H

#include "Virtfs/Virtfs.h"
#include "Virtfs/VfsIndexNode.h"
#include "Virtfs/VfsMountPoint.h"
VIRTFS_NS_BEGIN

class VfsDentry
{
public:
    VfsDentry(VfsIndexNode::InodeId linkedInode,
              WeakHandle<VfsDentry> parent,
              Handle<VfsMountPoint> mount);
    ~VfsDentry();

    static void SetRoot(Handle<VfsDentry> dentry);
    static Handle<VfsDentry> GetRoot();
    static Handle<VfsDentry> Resolve(const std::string& path);

    WeakHandle<VfsDentry> parent();
    void appendChild(Handle<VfsDentry> child);

    Handle<VfsIndexNode> inode();
    Handle<VfsMountPoint> mountpoint();

private:
    VfsIndexNode::InodeId           fLinkedInode;
    WeakHandle<VfsDentry>           fParentDentry;
    std::list<Handle<VfsDentry>>    fChildren;
    Handle<VfsMountPoint>           fMount;
};

VIRTFS_NS_END
#endif // COCOA_VFSDENTRY_H
