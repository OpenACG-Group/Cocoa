#ifndef COCOA_COREBINDING_H
#define COCOA_COREBINDING_H

#include "Koi/lang/Base.h"

KOI_LANG_NS_BEGIN

class CoreBindingModule : public BaseBindingModule
{
public:
    CoreBindingModule();
    ~CoreBindingModule() override = default;
    void getModule(binder::Module& self) override;
};

KOI_LANG_NS_END
#endif //COCOA_COREBINDING_H
