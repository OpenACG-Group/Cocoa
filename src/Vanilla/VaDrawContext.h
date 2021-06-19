#ifndef COCOA_SURFACE_H
#define COCOA_SURFACE_H

#include "include/core/SkRect.h"
#include "include/core/SkSurface.h"
#include "Vanilla/Base.h"
VANILLA_NS_BEGIN

class VaWindow;
class VaDrawContext
{
public:
    class ScopedFrame
    {
    public:
        ScopedFrame(Handle<VaDrawContext> ctx, const SkRect& region);
        /* ScopeFrame is non-copyable and non-movable */
        ScopedFrame(const ScopedFrame&) = delete;
        ScopedFrame& operator=(const ScopedFrame&) = delete;
        ~ScopedFrame();

        va_nodiscard inline sk_sp<SkSurface> surface()
        { return fSurface; }

    private:
        Handle<VaDrawContext>   fContext;
        sk_sp<SkSurface>        fSurface;
    };

    enum class RasterizerType
    {
        kRaster,
        kVulkan
    };

    VaDrawContext(Handle<VaWindow> window, RasterizerType type);
    virtual ~VaDrawContext();

    static Handle<VaDrawContext> MakeRaster(Handle<VaWindow> window);
    static Handle<VaDrawContext> MakeVulkan(Handle<VaWindow> window, bool enableDebug = false);

    va_nodiscard inline RasterizerType backend() const
    { return fType; }
    va_nodiscard inline Handle<VaWindow> getWindow()
    { return fWindow; }

    sk_sp<SkSurface> beginFrame(const SkRect& region);
    void endFrame();
    inline SkRect frameRegion()
    { return fRegion; }

    va_nodiscard inline bool isInFrame() const
    { return fInFrame; }

protected:
    virtual sk_sp<SkSurface> onBeginFrame(const SkRect& region) = 0;
    virtual void onEndFrame(const SkRect& region) = 0;
    virtual void onResize(int32_t width, int32_t height) = 0;

private:
    void onWindowConfigure(const Handle<VaWindow>& window, const SkRect& rect) va_slot;

    VA_SLOT_FIELDS(VA_SLOT_SIGNATURE(VaWindow, Configure))

    RasterizerType          fType;
    Handle<VaWindow>        fWindow;
    bool                    fInFrame;
    SkRect                  fRegion;
    SkRect                  fLastConfigure;
};

VANILLA_NS_END
#endif // COCOA_SURFACE_H
