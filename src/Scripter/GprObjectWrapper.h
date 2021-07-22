#ifndef COCOA_GPROBJECTWRAPPER_H
#define COCOA_GPROBJECTWRAPPER_H

#include <typeinfo>
#include <functional>
#include <any>

#include "Scripter/ScripterBase.h"
#include "Scripter/ResourceObject.h"
SCRIPTER_NS_BEGIN

class GprObjectWrapper : public ResourceObject
{
    RESOURCE_OBJECT(GprObjectWrapper)
public:
    template<typename T>
    explicit GprObjectWrapper(Runtime *runtime, std::shared_ptr<T> sp,
                              std::function<void(GprObjectWrapper*)> releaseCallback)
        : ResourceObject(runtime), fSp(sp), fReleaseCallback(std::move(releaseCallback)) {}

    template<typename T>
    inline std::shared_ptr<T> get()
    { return std::any_cast<std::shared_ptr<T>>(fSp); }

    inline const std::type_info& type()
    { return fSp.type(); }

private:
    void release() override
    { fReleaseCallback(this); }

    std::any    fSp;
    std::function<void(GprObjectWrapper*)>
                fReleaseCallback;
};

SCRIPTER_NS_END
#endif //COCOA_GPROBJECTWRAPPER_H
