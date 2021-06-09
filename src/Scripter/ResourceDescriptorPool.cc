#include <mutex>
#include <cassert>

#include "Scripter/ScripterBase.h"
#include "Scripter/Resource.h"
SCRIPTER_NS_BEGIN

#define LOCK(E) std::scoped_lock scopedLock(E);

ResourceDescriptorPool::ScopedAcquire::ScopedAcquire(ResourceDescriptorPool& pool, RID rid)
    : fPool(pool)
{
    fPtr = pool.acquire(rid);
    assert(fPtr != nullptr);
}

ResourceDescriptorPool::ScopedAcquire::~ScopedAcquire()
{
    fPool.giveBack(fPtr->getRID());
}

// ----------------------------------------------------------------------------------------

ResourceDescriptorPool::ResourceDescriptorPool(int32_t initialPoolSize)
    : fResources(initialPoolSize)
{
}

ResourceDescriptorPool::~ResourceDescriptorPool()
{
    LOCK(fRIDOpMutex)
    for (const auto& res : fResources)
        res->release();
}

void ResourceDescriptorPool::bindResourceWithDescriptor(ResourceObject::Ptr resource)
{
    LOCK(fRIDOpMutex)

    size_t size = fResources.size();
    RID rid = -1;
    for (int32_t i = 0; i < size; i++)
    {
        if (fResources[i] == nullptr)
        {
            rid = i;
            break;
        }
    }
    if (rid < 0)
    {
        fResources.resize(size + 20);
        rid = static_cast<RID>(size);
    }
    fResources[rid] = std::move(resource);
}

void ResourceDescriptorPool::release(RID rid)
{
    LOCK(fRIDOpMutex)
    if (rid >= fResources.size())
        return;
    auto ptr = fResources[rid];
    if (ptr == nullptr)
        return;
    fResources[rid] = nullptr;
    ptr->release();
    ptr->fState = ResourceObject::State::kReleased;
}

ResourceObject::Ptr ResourceDescriptorPool::acquire(RID rid)
{
    LOCK(fRIDOpMutex)
    if (rid >= fResources.size())
        return nullptr;
    auto res = fResources[rid];
    if (res == nullptr)
        return nullptr;

    assert(res->state() == ResourceObject::State::kReady);
    res->fState = ResourceObject::State::kAcquired;
    return res;
}

void ResourceDescriptorPool::giveBack(RID rid)
{
    if (rid >= fResources.size())
        return;
    auto ptr = fResources[rid];
    if (ptr == nullptr)
        return;

    assert(ptr->state() == ResourceObject::State::kAcquired);
    ptr->fState = ResourceObject::State::kReady;
}

SCRIPTER_NS_END
