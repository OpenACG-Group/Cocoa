#ifndef COCOA_VIRTFS_BASE_H
#define COCOA_VIRTFS_BASE_H

#include <memory>

#define VFS_NS_BEGIN    namespace cocoa::virtfs {
#define VFS_NS_END      }

VFS_NS_BEGIN

template<typename T>
using Handle = std::shared_ptr<T>;

template<typename T>
using UniqueHandle = std::unique_ptr<T>;

template<typename T>
using WeakHandle = std::weak_ptr<T>;

VFS_NS_END
#endif //COCOA_VIRTFS_BASE_H
