#ifndef COCOA_RESOURCE_H
#define COCOA_RESOURCE_H

#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

#include "Scripter/ScripterBase.h"
#include "Scripter/ResourceObject.h"
SCRIPTER_NS_BEGIN

class ResourceDescriptorPool
{
public:
    class ScopedAcquire
    {
    public:
        explicit ScopedAcquire(ResourceDescriptorPool& pool, RID rid);
        ~ScopedAcquire();

        inline ResourceObject::Ptr shared_ptr()
        { return fPtr; }

        inline ResourceObject *operator->()
        { return fPtr.get(); }

    private:
        ResourceDescriptorPool&     fPool;
        ResourceObject::Ptr         fPtr;
    };

    explicit ResourceDescriptorPool(int32_t initialPoolSize = 20);
    ~ResourceDescriptorPool();

    /**
     * A ResourceObject can't be acquired by more than one thread at the same time.
     */
    ResourceObject::Ptr acquire(RID rid);
    void giveBack(RID rid);

    void release(RID rid);

    template<typename T, typename...ArgsT>
    std::shared_ptr<T> resourceGen(ArgsT&&...args)
    {
        auto ptr = std::make_shared<T>(std::forward<ArgsT>(args)...);
        bindResourceWithDescriptor(ptr);
        return ptr;
    }

private:
    void bindResourceWithDescriptor(ResourceObject::Ptr resource);

    std::vector<ResourceObject::Ptr>    fResources;
    std::mutex                          fRIDOpMutex;
};

SCRIPTER_NS_END
#endif //COCOA_RESOURCE_H
