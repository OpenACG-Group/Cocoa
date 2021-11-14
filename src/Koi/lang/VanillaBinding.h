#ifndef COCOA_VANILLABINDING_H
#define COCOA_VANILLABINDING_H

#include "Koi/KoiBase.h"
#include "Koi/lang/Base.h"
KOI_LANG_NS_BEGIN

class VanillaBindingModule : public BindingBase
{
public:
    VanillaBindingModule();
    ~VanillaBindingModule() override = default;
    void getModule(binder::Module& mod) override;
    const char **getExports() override {}
};

KOI_LANG_NS_END
#endif //COCOA_VANILLABINDING_H
