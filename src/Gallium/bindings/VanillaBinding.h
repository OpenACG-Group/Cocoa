#ifndef COCOA_GALLIUM_BINDINGS_VANILLABINDING_H
#define COCOA_GALLIUM_BINDINGS_VANILLABINDING_H

#include "Gallium/Gallium.h"
#include "Gallium/lang/Base.h"
GALLIUM_BINDINGS_NS_BEGIN

class VanillaBindingModule : public BindingBase
{
public:
    VanillaBindingModule();
    ~VanillaBindingModule() override = default;
    void getModule(binder::Module& mod) override;
    const char **getExports() override {}
};

GALLIUM_BINDINGS_NS_END
#endif //COCOA_GALLIUM_BINDINGS_VANILLABINDING_H
