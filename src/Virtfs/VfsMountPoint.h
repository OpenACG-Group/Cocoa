#ifndef COCOA_VFSMOUNTPOINT_H
#define COCOA_VFSMOUNTPOINT_H

#include <utility>

#include "Virtfs/Virtfs.h"
#include "Virtfs/VfsIndexNode.h"
VIRTFS_NS_BEGIN

class VfsBackendDriver;
class VfsMountPoint
{
public:
    explicit VfsMountPoint(WeakHandle<VfsBackendDriver> driver)
        : fDriver(std::move(driver)) {}
    virtual ~VfsMountPoint() = default;

    inline WeakHandle<VfsBackendDriver> driver()
    {
        return fDriver;
    }
    virtual Handle<VfsIndexNode> queryInode(const std::string& name) = 0;

private:
    WeakHandle<VfsBackendDriver>    fDriver;
};

VIRTFS_NS_END
#endif //COCOA_VFSMOUNTPOINT_H
