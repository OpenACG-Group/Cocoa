#ifndef COCOA_VIRTFS_H
#define COCOA_VIRTFS_H

#include <memory>

#define VIRTFS_NS_BEGIN     namespace cocoa::virtfs {
#define VIRTFS_NS_END       }
VIRTFS_NS_BEGIN

template<typename T>
using Handle = std::shared_ptr<T>;
template<typename T>
using UniqueHandle = std::unique_ptr<T>;
template<typename T>
using WeakHandle = std::weak_ptr<T>;

template<typename T, typename...ArgsT>
Handle<T> NewHandle(ArgsT&&...args)
{
    return std::make_shared<T>(std::forward<ArgsT>(args)...);
}

template<typename T, typename...ArgsT>
UniqueHandle<T> NewUniqueHandle(ArgsT&&...args)
{
    return std::make_unique<T>(std::forward<ArgsT>(args)...);
}

VIRTFS_NS_END
#endif //COCOA_VIRTFS_H
