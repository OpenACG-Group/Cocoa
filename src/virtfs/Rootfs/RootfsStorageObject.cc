#include "Core/Exception.h"
#include "virtfs/Rootfs/RootfsStorageObject.h"
VFS_NS_BEGIN

RootfsStorageObject::RootfsStorageObject(Handle<RootfsStorageObject> parent, std::string name)
    : StorageObject(std::move(parent), std::move(name)),
      fPermFlags(kPerm_UserR | kPerm_UserW |
                 kPerm_GroupR | kPerm_GroupW |
                 kPerm_OtherR)
{
}

int32_t RootfsStorageObject::perm()
{
    return fPermFlags;
}

void RootfsStorageObject::setPerm(int32_t flags)
{
    fPermFlags = flags;
}

void RootfsStorageObject::setOwner(const std::string& group, const std::string& user)
{
    throw RuntimeException::Builder(__FUNCTION__)
            .append("virtfs: Operation not permitted")
            .make<RuntimeException>();
}

void RootfsStorageObject::remove()
{
}

Handle<StorageObject> RootfsStorageObject::mkdir(const std::string& name)
{
    return std::make_shared<RootfsStorageObject>(shared_from_this(),
                                                 name);
}

Handle<OpenInstance> RootfsStorageObject::open(OpenMode mode)
{
    // TODO: Implement open()
    return nullptr;
}

VFS_NS_END
