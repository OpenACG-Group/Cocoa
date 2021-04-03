#ifndef COCOA_VFSBACKENDDRIVER_H
#define COCOA_VFSBACKENDDRIVER_H

#include <string>

#include "Virtfs/Virtfs.h"
#include "Virtfs/VfsMountPoint.h"
VIRTFS_NS_BEGIN
class VfsBackendDriver
{
public:
    struct MountOptions
    {
        bool        readonly = false;
        bool        readwrite = true;
        bool        noExec = false;
    };

    virtual ~VfsBackendDriver() = default;

    static void RegisterDriver(const Handle<VfsBackendDriver>& driver);
    static void UnregisterDriver(const std::string& driver);
    static Handle<VfsBackendDriver> GetDriver(const std::string& driver);

    virtual std::string name() = 0;
    virtual Handle<VfsMountPoint> mount(MountOptions options, const std::string& device) = 0;
};

VIRTFS_NS_END
#endif // COCOA_VFSBACKENDDRIVER_H
