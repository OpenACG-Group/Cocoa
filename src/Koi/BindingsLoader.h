#ifndef COCOA_BINDINGSLOADER_H
#define COCOA_BINDINGSLOADER_H

#include <string>

#include "Koi/KoiBase.h"
KOI_NS_BEGIN

void PreloadBindings();
bool LoadBindingsFromDynamicLibrary(const std::string& file);

KOI_NS_END
#endif //COCOA_BINDINGSLOADER_H
