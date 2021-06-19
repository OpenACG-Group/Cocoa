#include <xcb/xcb.h>

#include "include/core/SkImageInfo.h"
#include "include/core/SkSurface.h"

#include "Vanilla/Base.h"
#include "Vanilla/Xcb/VaXcbRasterDrawContext.h"
#include "Vanilla/Xcb/VaXcbWindow.h"
#include "Vanilla/Xcb/VaXcbDisplay.h"
VANILLA_NS_BEGIN

VaXcbRasterDrawContext::VaXcbRasterDrawContext(Handle<VaWindow> window)
    : VaDrawContext(std::move(window), RasterizerType::kRaster),
      fConnection(nullptr),
      fWindow(0),
      fXcbGc(0),
      fPixelMemory(nullptr),
      fSurface(nullptr),
      fDepth(0)
{
    auto xcbWindow = std::dynamic_pointer_cast<VaXcbWindow>(getWindow());
    auto xcbDisplay = std::dynamic_pointer_cast<VaXcbDisplay>(getWindow()->getDisplay());
    fConnection = xcbDisplay->connection();
    fWindow = xcbWindow->window();

    fXcbGc = xcb_generate_id(fConnection);
    xcb_create_gc(fConnection, fXcbGc, fWindow, 0, nullptr);

    fDepth = xcbDisplay->screen()->root_depth;
    createSurface(getWindow()->width(), getWindow()->height());
}

void VaXcbRasterDrawContext::createSurface(int32_t width, int32_t height)
{
    if (fSurface)
        fSurface = nullptr;
    if (fPixelMemory)
        std::free(fPixelMemory);

    SkImageInfo info = SkImageInfo::Make(width, height, getWindow()->format(),
                                         SkAlphaType::kPremul_SkAlphaType);

    fPixelMemory = reinterpret_cast<uint8_t*>(std::malloc(width * height * info.bytesPerPixel()));

    fSurface = SkSurface::MakeRasterDirect(info, fPixelMemory, info.minRowBytes());
    if (!fSurface)
        throw VanillaException(__func__, "Failed to Skia surface for presentation");
}

VaXcbRasterDrawContext::~VaXcbRasterDrawContext()
{
    if (fSurface)
        fSurface = nullptr;
    if (fPixelMemory)
        std::free(fPixelMemory);
}

void VaXcbRasterDrawContext::onResize(int32_t width, int32_t height)
{
    createSurface(width, height);
}

sk_sp<SkSurface> VaXcbRasterDrawContext::onBeginFrame(const SkRect& region)
{
    return fSurface;
}

void VaXcbRasterDrawContext::onEndFrame(const SkRect& region)
{
    auto w = static_cast<int32_t>(region.width()), h = static_cast<int32_t>(region.height());
    auto x = static_cast<int32_t>(region.x()), y = static_cast<int32_t>(region.y());

    int32_t bpp = fSurface->imageInfo().bytesPerPixel();
    uint8_t *ptr = fPixelMemory + fSurface->imageInfo().minRowBytes() * y + x * bpp;

    xcb_put_image(fConnection,
                  XCB_IMAGE_FORMAT_Z_PIXMAP,
                  fWindow,
                  fXcbGc,
                  w, h,
                  static_cast<int16_t>(x), static_cast<int16_t>(y),
                  0,
                  fDepth,
                  w * h * bpp,
                  ptr);
    xcb_flush(fConnection);
}

VANILLA_NS_END
