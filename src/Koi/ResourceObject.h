#ifndef COCOA_RESOURCEOBJECT_H
#define COCOA_RESOURCEOBJECT_H

#include <memory>
#include <cassert>

#include "Koi/KoiBase.h"
KOI_NS_BEGIN

#define RESOURCE_OBJECT(T) \
public:                    \
static inline T *Cast(ResourceObject *ptr) { \
    auto ret = dynamic_cast<T*>(ptr); \
    assert(ret != nullptr); \
    return ret; \
}

/* Resource Identifier (Resource Descriptor) */
using RID = int32_t;
class Runtime;
class ResourceObject
{
public:
    friend class ResourceDescriptorPool;
    enum class State
    {
        kReady,
        kAcquired,
        kReleased,
        kError
    };

    explicit ResourceObject(Runtime *runtime);
    virtual ~ResourceObject();

    void setRID(RID rid);

    koi_nodiscard inline Runtime *runtime()
    { return fRuntime; }

    koi_nodiscard inline RID getRID() const
    { return fRID; }

    koi_nodiscard inline State state() const
    { return fState; }

    virtual void release() = 0;

private:
    Runtime         *fRuntime;
    RID              fRID;
    State            fState;
};

KOI_NS_END
#endif //COCOA_RESOURCEOBJECT_H
