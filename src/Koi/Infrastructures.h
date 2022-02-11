#ifndef COCOA_INFRASTRUCTURES_H
#define COCOA_INFRASTRUCTURES_H

#include "include/v8.h"
#include "Koi/KoiBase.h"
KOI_NS_BEGIN

namespace infra {

void InstallOnGlobalContext(v8::Isolate *isolate,
                            v8::Local<v8::Context> context);

} // namespace infra

KOI_NS_END
#endif //COCOA_INFRASTRUCTURES_H
