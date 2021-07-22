#ifndef COCOA_RESOURCEOBJECT_H
#define COCOA_RESOURCEOBJECT_H

#include <memory>
#include <cassert>

#include "Scripter/ScripterBase.h"
SCRIPTER_NS_BEGIN

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

    explicit ResourceObject(Runtime *runtime)
        : fRuntime(runtime), fRID(0), fState(State::kReady) {}
    virtual ~ResourceObject() = default;

    inline void setRID(RID rid)
    { fRID = rid; }

    inline Runtime *runtime()
    { return fRuntime; }

    inline RID getRID() const
    { return fRID; }

    inline State state() const
    { return fState; }

    virtual void release() = 0;

private:
    Runtime         *fRuntime;
    RID              fRID;
    State            fState;
};

SCRIPTER_NS_END
#endif //COCOA_RESOURCEOBJECT_H
