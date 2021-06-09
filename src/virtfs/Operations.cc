#include <string>
#include <vector>

#include "Core/Exception.h"
#include "virtfs/Base.h"
#include "virtfs/Operations.h"
#include "virtfs/StorageObject.h"
#include "virtfs/Rootfs/RootfsStorageObject.h"
VFS_NS_BEGIN

namespace {
Handle<RootfsStorageObject> vfs_root(nullptr);
Handle<StorageObject> current(nullptr);

Handle<StorageObject> resolve_vec_to_storage_object(Handle<StorageObject> where,
                                                    std::vector<std::string>::const_iterator itr,
                                                    std::vector<std::string>::const_iterator endItr)
{
    if (itr == endItr)
        return where;
    for (const auto& child : where->children())
    {
        if (child->name() == *itr)
            return resolve_vec_to_storage_object(child, itr + 1, endItr);
    }
    return nullptr;
}

Handle<StorageObject> resolve_path_to_storage_object(const std::string& path)
{
    std::vector<std::string> vec;
    std::string buf;
    for (char ch : path)
    {
        if (ch == '/')
        {
            if (!buf.empty())
                vec.push_back(buf);
            buf.clear();
        }
        else
            buf.push_back(ch);
    }
    if (!buf.empty())
        vec.push_back(buf);

    Handle<StorageObject> entry = current;
    if (path[0] == '/')
        entry = vfs_root;

    return resolve_vec_to_storage_object(entry, vec.cbegin(), vec.cend());
}

} // namespace anonymous

void _throw_file_not_found(const std::string& str)
{
    throw RuntimeException::Builder(__FUNCTION__)
            .append("virtfs: No such file or directory [")
            .append(str)
            .append("]")
            .make<RuntimeException>();
}

void Initialize()
{
    vfs_root = std::make_shared<RootfsStorageObject>(nullptr, "#root");
    current = vfs_root;
}

void Finalize()
{
    current = nullptr;
    vfs_root = nullptr;
}

void Mkdir(const std::string& path, const std::string& dir)
{
    Handle<StorageObject> object = resolve_path_to_storage_object(path);
    if (object == nullptr)
        _throw_file_not_found(path);
    Handle<StorageObject> pDir = object->mkdir(dir);
    object->appendChild(pDir);
}

void Chdir(const std::string& dir)
{
    Handle<StorageObject> object = resolve_path_to_storage_object(dir);
    if (object == nullptr)
        _throw_file_not_found(dir);
    current = object;
}

VFS_NS_END
