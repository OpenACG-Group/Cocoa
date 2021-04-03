#ifndef COCOA_VFSINDEXNODE_H
#define COCOA_VFSINDEXNODE_H

#include <string>
#include <list>
#include <vector>
#include "Virtfs/Virtfs.h"
VIRTFS_NS_BEGIN

class VfsIndexNode
{
public:
    enum NodeType
    {
        kIndexNode_File,
        kIndexNode_Directory
    };

    enum PermBits
    {
        kPerm_OwnerAccess        = 0x0001,
        kPerm_OwnerRead          = 0x0002,
        kPerm_OwnerWrite         = 0x0004,
        kPerm_OwnerExecutable    = 0x0008,
        kPerm_GroupAccess        = 0x0010,
        kPerm_GroupRead          = 0x0020,
        kPerm_GroupWrite         = 0x0040,
        kPerm_GroupExecutable    = 0x0080,
        kPerm_OtherAccess        = 0x0100,
        kPerm_OtherRead          = 0x0200,
        kPerm_OtherWrite         = 0x0400,
        kPerm_OtherExecutable    = 0x0800,
        kPerm_AllAccess          = kPerm_OwnerAccess | kPerm_GroupAccess | kPerm_OtherAccess,
        kPerm_AllRead            = kPerm_OwnerRead | kPerm_GroupRead | kPerm_OtherRead,
        kPerm_AllWrite           = kPerm_OwnerWrite | kPerm_GroupWrite | kPerm_OtherWrite,
        kPerm_AllExecutable      = kPerm_OwnerExecutable | kPerm_GroupExecutable | kPerm_OtherExecutable
    };
    using PermFlags = uint16_t;
    using InodeId = uint64_t;
    static const PermFlags DEFAULT_PERM_FLAGS = kPerm_AllAccess | kPerm_AllRead |
                                                kPerm_OwnerWrite | kPerm_GroupWrite;

    static InodeId AllocInodeId();
    static Handle<VfsIndexNode> GetInodeHandle(InodeId id);
    static void AppendInodeEntry(const Handle<VfsIndexNode>& inode);
    static void RemoveInodeEntry(InodeId id);

    virtual ~VfsIndexNode() = default;

    inline NodeType type()
    {
        return fInodeType;
    }

    inline InodeId inodeId()
    {
        return fInodeId;
    }

    inline std::string name()
    {
        return fInodeName;
    }

    inline std::string owner()
    {
        return fOwnerName;
    }

    inline std::string group()
    {
        return fGroupName;
    }

    inline PermFlags permFlags()
    {
        return fPermFlags;
    }

    inline size_t size()
    {
        return onSize();
    }

    void setOwner(const std::string& owner);
    void setGroup(const std::string& group);
    void setPermFlags(PermFlags newFlags);

    void rename(const std::string& name);
    void remove();

    /* Append a child inode */
    Handle<VfsIndexNode> create(const std::string& name, PermFlags flags = DEFAULT_PERM_FLAGS);
    Handle<VfsIndexNode> mkdir(const std::string& name, PermFlags flags = DEFAULT_PERM_FLAGS);

    inline InodeId parent()
    {
        return fParentId;
    }

    inline std::list<InodeId>& children()
    {
        return fChildIds;
    }

protected:
    VfsIndexNode(InodeId id,
                 InodeId parent,
                 std::vector<InodeId> children,
                 NodeType type,
                 std::string name,
                 std::string owner,
                 std::string group,
                 PermFlags flags);

    virtual void onSetName(const std::string& name) = 0;
    virtual void onSetOwner(const std::string& owner) = 0;
    virtual void onSetGroup(const std::string& group) = 0;
    virtual void onSetPermFlags(PermFlags flags) = 0;
    virtual size_t onSize() = 0;

    virtual void onRemove() = 0;
    virtual void onRename(const std::string& name) = 0;
    virtual Handle<VfsIndexNode> onCreate(const std::string& name, PermFlags flags) = 0;
    virtual Handle<VfsIndexNode> onMkdir(const std::string& name, PermFlags flags) = 0;

    void permissionDenied();
    void appendChild(InodeId id);

private:
    InodeId             fInodeId;
    NodeType            fInodeType;
    InodeId             fParentId;
    std::list<InodeId>  fChildIds;
    std::string         fInodeName;
    std::string         fOwnerName;
    std::string         fGroupName;
    PermFlags           fPermFlags;
};

VIRTFS_NS_END
#endif // COCOA_VFSINDEXNODE_H
