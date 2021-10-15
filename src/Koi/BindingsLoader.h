#ifndef COCOA_BINDINGSLOADER_H
#define COCOA_BINDINGSLOADER_H

#include <string>

#include "Koi/KoiBase.h"
#include "Koi/Runtime.h"
KOI_NS_BEGIN

const Runtime::Options& GetGlobalRuntimeOptions();
void PreloadBindings(const Runtime::Options& options);
bool LoadBindingsFromDynamicLibrary(const std::string& file);

KOI_NS_END
#endif //COCOA_BINDINGSLOADER_H
