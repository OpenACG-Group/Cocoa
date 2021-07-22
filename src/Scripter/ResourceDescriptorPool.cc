#include <mutex>
#include <cassert>

#include "Scripter/ScripterBase.h"
#include "Scripter/ResourceDescriptorPool.h"
SCRIPTER_NS_BEGIN

#define LOCK(E) std::scoped_lock scopedLock(E);

// ----------------------------------------------------------------------------------------

ResourceDescriptorPool::ResourceDescriptorPool(int32_t initialPoolSize)
    : fResources(initialPoolSize)
{
}

ResourceDescriptorPool::~ResourceDescriptorPool()
{
    LOCK(fRIDOpMutex)
    for (RID rid : fTimeOrderedRIDList)
    {
        ResourceObject *res = fResources[rid];
        res->release();
        delete res;
    }
}

void ResourceDescriptorPool::bindResourceWithDescriptor(ResourceObject *resource)
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
    resource->setRID(rid);
    fResources[rid] = resource;
    fTimeOrderedRIDList.push_back(rid);
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
    delete ptr;

    fTimeOrderedRIDList.remove(rid);
}

ResourceObject *ResourceDescriptorPool::acquire(RID rid)
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
