#ifndef COCOA_SURFACE_H
#define COCOA_SURFACE_H

#include "include/core/SkRect.h"
#include "include/core/SkSurface.h"
#include "Vanilla/Base.h"
VANILLA_NS_BEGIN

class Window;
class DrawContext
{
public:
    class ScopedFrame
    {
    public:
        ScopedFrame(Handle<DrawContext> ctx, const SkRect& region);
        /* ScopeFrame is non-copyable and non-movable */
        ScopedFrame(const ScopedFrame&) = delete;
        ScopedFrame& operator=(const ScopedFrame&) = delete;
        ~ScopedFrame();

        va_nodiscard inline sk_sp<SkSurface> surface()
        { return fSurface; }

    private:
        Handle<DrawContext>   fContext;
        sk_sp<SkSurface>        fSurface;
    };

    enum class RasterizerType
    {
        kRaster,
        kVulkan
    };

    DrawContext(Handle<Window> window, RasterizerType type);
    virtual ~DrawContext();

    static Handle<DrawContext> MakeRaster(Handle<Window> window);
    static Handle<DrawContext> MakeVulkan(Handle<Window> window, bool enableDebug = false);

    va_nodiscard inline RasterizerType backend() const
    { return fType; }
    va_nodiscard inline Handle<Window> getWindow()
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
    void onWindowConfigure(const Handle<Window>& window, const SkRect& rect) va_slot;

    VA_SLOT_FIELDS(VA_SLOT_SIGNATURE(Window, Configure))

    RasterizerType          fType;
    Handle<Window>        fWindow;
    bool                    fInFrame;
    SkRect                  fRegion;
    SkRect                  fLastConfigure;
};

VANILLA_NS_END
#endif // COCOA_SURFACE_H
