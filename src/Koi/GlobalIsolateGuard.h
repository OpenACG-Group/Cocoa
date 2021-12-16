#ifndef COCOA_GLOBALISOLATEGUARD_H
#define COCOA_GLOBALISOLATEGUARD_H

#include "include/v8.h"

#include "Koi/KoiBase.h"
KOI_NS_BEGIN

class Runtime;

class GlobalIsolateGuard
{
public:
    explicit GlobalIsolateGuard(const std::shared_ptr<Runtime>& rt);
    ~GlobalIsolateGuard();

    koi_nodiscard inline v8::Isolate *getIsolate() const {
        return fIsolate;
    }

    koi_nodiscard inline std::shared_ptr<Runtime> getRuntime() const {
        return fRuntime.lock();
    }

private:
    std::weak_ptr<Runtime> fRuntime;
    v8::Isolate           *fIsolate;
};

KOI_NS_END
#endif //COCOA_GLOBALISOLATEGUARD_H
