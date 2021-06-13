#ifndef COCOA_VARENDERABLEBUFFER_H
#define COCOA_VARENDERABLEBUFFER_H

#include "include/core/SkSurface.h"
#include "Vanilla/Base.h"
VANILLA_NS_BEGIN

class VaRenderableBuffer
{
public:

protected:
    virtual sk_sp<SkSurface> onCreateSurface() = 0;

private:
    sk_sp<SkSurface>        fSurface;
};

VANILLA_NS_END
#endif //COCOA_VARENDERABLEBUFFER_H
