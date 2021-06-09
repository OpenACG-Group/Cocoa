#ifndef COCOA_RESOURCEOBJECT_H
#define COCOA_RESOURCEOBJECT_H

#include <memory>
#include <cassert>

#include "Scripter/ScripterBase.h"
SCRIPTER_NS_BEGIN

#define RESOURCE_OBJECT(T) \
static inline std::shared_ptr<T> Cast(const ResourceObject::Ptr& ptr) { \
    auto ret = std::dynamic_pointer_cast<T>(ptr); \
    assert(ret != nullptr); \
    return ret; \
}

#define __outptr
#define __inptr

/* Resource Identifier (Resource Descriptor) */
using RID = int64_t;
class ResourceObject
{
public:
    friend class ResourceDescriptorPool;

    using Ptr = std::shared_ptr<ResourceObject>;
    enum class State
    {
        kReady,
        kAcquired,
        kReleased,
        kError
    };

    ResourceObject();
    virtual ~ResourceObject() = default;

    inline void setRID(RID rid)
    { fRID = rid; }

    inline RID getRID() const
    { return fRID; }

    inline State state() const
    { return fState; }

    virtual void release() = 0;

private:
    RID         fRID;
    State       fState;
};

class IOResource : public ResourceObject
{
public:
    RESOURCE_OBJECT(IOResource)

    IOResource(int fd);
    ~IOResource() override;

    void release() override;
    ssize_t write(__inptr const void *pSrc, size_t size);
    ssize_t read(__outptr void *pDst, size_t size);

private:
    int     fFd;
};

#undef RESOURCE_OBJECT
#undef __inptr
#undef __outptr
SCRIPTER_NS_END
#endif //COCOA_RESOURCEOBJECT_H
