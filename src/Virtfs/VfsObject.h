#ifndef COCOA_VFSOBJECT_H
#define COCOA_VFSOBJECT_H

#include <string>

#include "Virtfs/Virtfs.h"
VIRTFS_NS_BEGIN

class VfsObject
{
public:
    virtual ~VfsObject() = default;

    VfsObjectType type();

    std::string path();
    std::string owner();
    std::string group();
    VfsObjectPermBits perm();

private:
};

VIRTFS_NS_END
#endif //COCOA_VFSOBJECT_H
