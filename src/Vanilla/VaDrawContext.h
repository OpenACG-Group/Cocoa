#ifndef COCOA_SURFACE_H
#define COCOA_SURFACE_H

#include "include/core/SkRect.h"
#include "Vanilla/Base.h"
VANILLA_NS_BEGIN

class VaWindow;
class VaDrawContext
{
public:
    VaDrawContext(Handle<VaWindow> window);
    virtual ~VaDrawContext() = default;

    static Handle<VaDrawContext> MakeRaster(Handle<VaWindow> window);
    static Handle<VaDrawContext> MakeVulkan(Handle<VaWindow> window);

    inline Handle<VaWindow> getWindow()
    { return fWindow; }

    virtual void beginFrame(const SkRect& region) = 0;
    virtual void endFrame() = 0;

private:
    Handle<VaWindow>        fWindow;
};

VANILLA_NS_END
#endif //COCOA_SURFACE_H
