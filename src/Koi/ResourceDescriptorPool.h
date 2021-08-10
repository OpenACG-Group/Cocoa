#ifndef COCOA_RESOURCE_H
#define COCOA_RESOURCE_H

#include <iostream>
#include <cstdint>
#include <map>
#include <list>
#include <memory>
#include <mutex>
#include <vector>
#include <typeinfo>

#include "Koi/KoiBase.h"
#include "Koi/ResourceObject.h"
KOI_NS_BEGIN

class ResourceDescriptorPool
{
public:
    template<typename T>
    class ScopedAcquire
    {
    public:
        explicit ScopedAcquire(ResourceDescriptorPool& pool, RID rid)
            : fPool(pool), fPtr(nullptr) {
            ResourceObject *pGeneral = pool.acquire(rid);
            if (pGeneral)
                fPtr = T::Cast(pGeneral);
        }

        ~ScopedAcquire() {
            if (fPtr)
                fPool.giveBack(fPtr->getRID());
        }
        ScopedAcquire(const ScopedAcquire&) = delete;
        ScopedAcquire operator=(const ScopedAcquire&) = delete;

        ScopedAcquire(ScopedAcquire&& rhs) noexcept
            : fPool(rhs.fPool), fPtr(rhs.fPtr) {
            rhs.fPtr = nullptr;
        }

        inline bool valid()
        { return fPtr != nullptr; }

        inline T *operator->()
        { return fPtr; }

        inline void release() {
            if (fPtr)
                fPool.release(fPtr->getRID());
            fPtr = nullptr;
        }

    private:
        ResourceDescriptorPool&  fPool;
        T                       *fPtr;
    };

    explicit ResourceDescriptorPool(int32_t initialPoolSize = 64);
    ~ResourceDescriptorPool();

    /**
     * A ResourceObject can't be acquired by more than one thread at the same time.
     */
    ResourceObject *acquire(RID rid);
    void giveBack(RID rid);

    void release(RID rid);

    template<typename T, typename...ArgsT>
    T *resourceGen(ArgsT&&...args)
    {
        auto ptr = new T(std::forward<ArgsT>(args)...);
        bindResourceWithDescriptor(ptr);
        return ptr;
    }

private:
    void bindResourceWithDescriptor(ResourceObject *resource);

    std::vector<ResourceObject*>    fResources;
    std::list<RID>                  fTimeOrderedRIDList;
    std::mutex                      fRIDOpMutex;
};

KOI_NS_END
#endif //COCOA_RESOURCE_H
