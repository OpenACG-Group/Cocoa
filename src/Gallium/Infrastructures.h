#ifndef COCOA_GALLIUM_INFRASTRUCTURES_H
#define COCOA_GALLIUM_INFRASTRUCTURES_H

#include "include/v8.h"
#include "Gallium/Gallium.h"
GALLIUM_NS_BEGIN

namespace infra {

void InstallOnGlobalContext(v8::Isolate *isolate,
                            v8::Local<v8::Context> context);

} // namespace infra

GALLIUM_NS_END
#endif //COCOA_GALLIUM_INFRASTRUCTURES_H
