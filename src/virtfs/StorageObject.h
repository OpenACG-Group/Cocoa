#ifndef COCOA_STORAGEOBJECT_H
#define COCOA_STORAGEOBJECT_H

#include <list>
#include <string>
#include <utility>

#include "virtfs/Base.h"
VFS_NS_BEGIN

class OpenInstance;
class StorageObject
{
public:
    enum PermFlags
    {
        kPerm_UserR     = 0x001,
        kPerm_UserW     = 0x002,
        kPerm_UserX     = 0x004,

        kPerm_GroupR    = 0x010,
        kPerm_GroupW    = 0x020,
        kPerm_GroupX    = 0x040,

        kPerm_OtherW    = 0x100,
        kPerm_OtherR    = 0x200,
        kPerm_OtherX    = 0x400
    };

    enum OpenMode
    {
        kOpen_Read      = 0x01,
        kOpen_Write     = 0x02,
        kOpen_ReadWrite = 0x04,
        kOpen_Append    = 0x08,
        kOpen_Trunc     = 0x10
    };

    explicit StorageObject(const Handle<StorageObject>& parent, std::string name)
        : fName(std::move(name)), fParent(parent) {}
    virtual ~StorageObject() = default;

    inline std::string name() const
    { return fName; }

    inline void setParent(const Handle<StorageObject>& parent)
    { fParent = parent; }

    inline void appendChild(Handle<StorageObject> child)
    { fChildren.push_back(std::move(child)); }

    inline WeakHandle<StorageObject> parent()
    { return fParent; }

    inline const std::list<Handle<StorageObject>>& children()
    { return fChildren; }

    virtual int32_t perm() = 0;
    virtual void setPerm(int32_t flags) = 0;
    virtual void setOwner(const std::string& group, const std::string& user) = 0;
    virtual void remove() = 0;
    virtual Handle<StorageObject> mkdir(const std::string& name) = 0;
    virtual Handle<OpenInstance> open(OpenMode mode) = 0;

private:
    std::string                         fName;
    WeakHandle<StorageObject>           fParent;
    std::list<Handle<StorageObject>>    fChildren;
};

VFS_NS_END
#endif //COCOA_STORAGEOBJECT_H
