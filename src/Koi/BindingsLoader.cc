#include "Koi/KoiBase.h"

#include "Koi/lang/CoreBinding.h"
KOI_NS_BEGIN

void PreloadInternalBindings()
{
    lang::_binding_core_preload_hook();
}

void PreloadBindingsFromDynamicLibrary(const std::string& file)
{
    // TODO: Implement this.
}

KOI_NS_END
