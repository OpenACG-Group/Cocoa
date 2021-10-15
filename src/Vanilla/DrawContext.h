#ifndef COCOA_SURFACE_H
#define COCOA_SURFACE_H

#include <mutex>

#include "include/core/SkRect.h"
#include "include/core/SkSurface.h"
#include "Vanilla/Base.h"
VANILLA_NS_BEGIN

class Window;
class DrawContext
{
public:
    class DrawScope
    {
    public:
        explicit DrawScope(Handle<DrawContext> ctx);
        DrawScope(const DrawScope&) = delete;
        DrawScope& operator=(const DrawScope&) = delete;
        ~DrawScope();

    private:
        Handle<DrawContext>  fContext;
    };

    class PresentationScope
    {
    public:
        PresentationScope(Handle<DrawContext> ctx, const SkRect& region);
        PresentationScope(const PresentationScope&) = delete;
        PresentationScope& operator=(const PresentationScope&) = delete;
        ~PresentationScope();

        va_nodiscard inline sk_sp<SkSurface> surface()
        { return fSurface; }

    private:
        Handle<DrawContext>   fContext;
        sk_sp<SkSurface>      fSurface;
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

    va_nodiscard SkColorType getColorType() const;

    /**
     * @brief Prepare for presenting a frame.
     *
     * @param region Clipping rectangle, may not be used.
     * @return A reference-counted SkSurface.
     *         Anything that you paint on the surface will be presented
     *         on the window directly.
     *
     * @note Class `PresentationScope` can call beginFrame/endFrame automatically
     *       when constructing/destructing.
     */
    sk_sp<SkSurface> beginFrame(const SkRect& region);

    /**
     * @brief Presents current frame.
     * @relatedalso `beginFrame`
     */
    void endFrame();

    inline SkRect frameRegion()
    { return fRegion; }
    va_nodiscard inline bool isInFrame() const
    { return fInFrame; }

    /**
     * @brief Lock the mutex of this DrawContext, blocks if it's already locked.
     *
     * @note The rasterizer in DrawContext is not multi-threaded.
     *       A thread must lock the DrawContext before rasterization or presentation
     *       to avoid other threads operating on the DrawContext.
     *       Class `DrawScope` can lock/unlock the DrawContext automatically
     *       when constructing/destructing.
     */
    void lockContext();

    /**
     * @brief Unlock the mutex of this DrawContext.
     * @relatedalso `lockContext`
     */
    void unlockContext();

    /**
     * @brief Try lock the mutex of this DrawContext, returns false immediately
     *        if it's already locked.
     * @relatedalso `lockContext`
     */
    va_nodiscard bool tryLockContext();

    /**
     * @brief Creates a backend surface.
     *
     * @note Where to store the surface's pixels depends on the type of backend.
     * It's always stored in system memory for Raster backend, or as a GPU texture
     * in GPU memory for GPU backend. Downloading a GPU texture to system memory
     * is always expensive and may cause low-performance composition.
     *
     * @param info      Specify dimensions, color format, etc. for surface.
     * @param budgeted  Ignored for Raster backend.
     * @return A reference-counted SkSurface.
     */
    va_nodiscard virtual sk_sp<SkSurface> createBackendSurface(const SkImageInfo& info, SkBudgeted budgeted)
    { return nullptr; }

protected:
    virtual sk_sp<SkSurface> onBeginFrame(const SkRect& region) = 0;
    virtual void onEndFrame(const SkRect& region) = 0;
    virtual void onResize(int32_t width, int32_t height) = 0;

private:
    void onWindowConfigure(const Handle<Window>& window, const SkRect& rect) va_slot;

    VA_SLOT_FIELDS(VA_SLOT_SIGNATURE(Window, Configure))

    RasterizerType          fType;
    Handle<Window>          fWindow;
    bool                    fInFrame;
    SkRect                  fRegion;
    SkRect                  fLastConfigure;
    std::mutex              fRasterMutex;
};

VANILLA_NS_END
#endif // COCOA_SURFACE_H
