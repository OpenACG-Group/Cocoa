#ifndef COCOA_ROOTFSSTORAGEOBJECT_H
#define COCOA_ROOTFSSTORAGEOBJECT_H

#include <string>

#include "virtfs/Base.h"
#include "virtfs/StorageObject.h"
VFS_NS_BEGIN

class RootfsStorageObject : public StorageObject,
                            public std::enable_shared_from_this<RootfsStorageObject>
{
public:
    RootfsStorageObject(Handle<RootfsStorageObject> parent, std::string name);
    ~RootfsStorageObject() override = default;

    int32_t perm() override;
    void setPerm(int32_t flags) override;
    void setOwner(const std::string& group, const std::string& user) override;
    void remove() override;
    Handle<StorageObject> mkdir(const std::string& name) override;
    Handle<OpenInstance> open(OpenMode mode) override;

private:
    int32_t     fPermFlags;
};

VFS_NS_END
#endif //COCOA_ROOTFSSTORAGEOBJECT_H
