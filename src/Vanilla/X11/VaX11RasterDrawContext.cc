#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>

#include "Vanilla/Base.h"
#include "Vanilla/X11/VaX11RasterDrawContext.h"
#include "Vanilla/X11/VaX11Display.h"
#include "Vanilla/X11/VaX11Window.h"
VANILLA_NS_BEGIN

VaX11RasterDrawContext::VaX11RasterDrawContext(Handle<VaWindow> window)
    : VaDrawContext(std::move(window), RasterizerType::kRaster),
      fXDisplay(nullptr),
      fXWindow(0),
      fPixels(nullptr),
      fSurface(nullptr),
      fImage(nullptr)
{
    auto x11Display = std::dynamic_pointer_cast<VaX11Display>(getWindow()->getDisplay());
    auto x11Window = std::dynamic_pointer_cast<VaX11Window>(getWindow());
    fXDisplay = x11Display->display();
    fXWindow = x11Window->nativeWindowHandle();
    fGc = XDefaultGCOfScreen(x11Display->screen());

    XWindowAttributes attrs;
    XGetWindowAttributes(fXDisplay, fXWindow, &attrs);
    fWindowDepth = attrs.depth;

    createImages(x11Window->width(), x11Window->height());
}

VaX11RasterDrawContext::~VaX11RasterDrawContext()
{
    fSurface = nullptr;
    if (fImage)
        XDestroyImage(fImage);
}

void VaX11RasterDrawContext::createImages(int32_t width, int32_t height)
{
    SkImageInfo info = SkImageInfo::Make(width, height,
                                         getWindow()->format(),
                                         SkAlphaType::kPremul_SkAlphaType);

    size_t sizeInBytes = info.bytesPerPixel() * width * height;

    if (fImage)
        XDestroyImage(fImage);
    fPixels = reinterpret_cast<uint8_t*>(std::malloc(sizeInBytes));

    fSurface = SkSurface::MakeRasterDirect(info, fPixels,
                                           info.bytesPerPixel() * width);
    if (fSurface == nullptr)
        throw VanillaException(__func__, "Failed to create Skia surface");

    fImage = XCreateImage(fXDisplay,
                          std::dynamic_pointer_cast<VaX11Display>(getWindow()->getDisplay())->visual(),
                          fWindowDepth,
                          ZPixmap,
                          0,
                          reinterpret_cast<char*>(fPixels),
                          width,
                          height,
                          32,
                          info.bytesPerPixel() * width);
    if (fImage == nullptr)
        throw VanillaException(__func__, "Failed to create XImage");
}

void VaX11RasterDrawContext::onResize(int32_t width, int32_t height)
{
    this->createImages(width, height);
}

sk_sp<SkSurface> VaX11RasterDrawContext::onBeginFrame(const SkRect& region)
{
    return fSurface;
}

void VaX11RasterDrawContext::onEndFrame(const SkRect& region)
{
    int x = static_cast<int>(region.x()),
        y = static_cast<int>(region.y()),
        w = static_cast<int>(region.width()),
        h = static_cast<int>(region.height());

    XPutImage(fXDisplay, fXWindow, fGc, fImage, x, y, x, y, w, h);
    getWindow()->getDisplay()->flush();
}

VANILLA_NS_END
