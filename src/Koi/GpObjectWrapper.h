#ifndef COCOA_GPOBJECTWRAPPER_H
#define COCOA_GPOBJECTWRAPPER_H

#include <typeinfo>
#include <functional>
#include <any>

#include "Koi/KoiBase.h"
#include "Koi/ResourceObject.h"
KOI_NS_BEGIN

class GpObjectWrapper : public ResourceObject
{
    RESOURCE_OBJECT(GpObjectWrapper)
public:
    template<typename T>
    explicit GpObjectWrapper(Runtime *runtime, std::shared_ptr<T> sp,
                              std::function<void(GpObjectWrapper*)> releaseCallback)
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
    std::function<void(GpObjectWrapper*)>
                fReleaseCallback;
};

KOI_NS_END
#endif //COCOA_GPOBJECTWRAPPER_H
