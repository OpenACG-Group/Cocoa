#include <iostream>
#include <tuple>

#include <X11/Xutil.h>
#include "include/core/SkImage.h"
#include "include/core/SkPixmap.h"
#include "include/codec/SkCodec.h"

#include "Core/Journal.h"
#include "Vanilla/Base.h"
#include "Vanilla/X11/VaX11Window.h"
#include "Vanilla/X11/VaX11Display.h"
VANILLA_NS_BEGIN

VaX11Window::VaX11Window(WeakHandle<VaX11Display> display, Window window,
                         int32_t width, int32_t height, VaColorFormat format)
    : VaWindow(std::move(display), format),
      fXDisplay(nullptr),
      fWindow(window),
      fWidth(width),
      fHeight(height),
      fMapped(false)
{
    fXDisplay = std::dynamic_pointer_cast<VaX11Display>(getDisplay())->display();
}

VaX11Window::~VaX11Window()
{
}

void VaX11Window::setTitle(const std::string& title)
{
    if (!fMapped)
    {
        fOperationCache.title = title;
        return;
    }

    XStoreName(fXDisplay, fWindow, title.c_str());
    getDisplay()->flush();
}

void VaX11Window::setResizable(bool resizable)
{
    if (!fMapped)
    {
        fOperationCache.resizable = resizable;
        return;
    }

    XSizeHints sizeHints{};
    sizeHints.flags = PMaxSize | PMinSize;
    if (resizable)
    {
        sizeHints.min_width = 0;
        sizeHints.max_width = 0xffff;
        sizeHints.min_height = 0;
        sizeHints.max_height = 0xffff;
    }
    else
    {
        sizeHints.min_width = sizeHints.max_width = fWidth;
        sizeHints.min_height = sizeHints.max_height = fHeight;
    }
    XSetWMNormalHints(fXDisplay, fWindow, &sizeHints);
    getDisplay()->flush();
}

namespace {

std::tuple<int32_t, uint8_t*> LoadIconFile(const std::string& file)
{
    sk_sp<SkData> data = SkData::MakeFromFileName(file.c_str());
    if (data == nullptr)
        return { 0, nullptr };
    UniqueHandle<SkCodec> codec = SkCodec::MakeFromData(data);
    if (codec == nullptr)
        return { 0, nullptr };

    SkColorType colorType = SkColorType::kBGRA_8888_SkColorType;
    SkAlphaType alphaType = SkAlphaType::kUnpremul_SkAlphaType;
    SkImageInfo originInfo = codec->getInfo();

    sk_sp<SkImage> image;
    {
        auto result = codec->getImage();
        if (std::get<1>(result) != SkCodec::Result::kSuccess)
            return { 0, nullptr };
        image = std::get<0>(result);
    }

    int32_t length = 0;
    constexpr int32_t maxLength = (2 + 16 * 16) +
                                  (2 + 32 * 32) +
                                  (2 + 64 * 64) +
                                  (2 + 128 * 128);
    auto *buffer = reinterpret_cast<uint32_t*>(std::malloc(maxLength * sizeof(uint32_t)));
    uint32_t *ptr = buffer;
    std::memset(buffer, 0x00, maxLength);

    for (int32_t s : { 16, 32, 64, 128 })
    {
        if (originInfo.width() < s || originInfo.height() < s)
            break;

        SkImageInfo info = SkImageInfo::Make(s, s, colorType, alphaType);
        length += 2 + s * s;
        *(ptr++) = *(ptr++) = s;
        SkPixmap dstPixmap(info, ptr, info.minRowBytes());
        image->scalePixels(dstPixmap, SkSamplingOptions(SkFilterMode::kLinear));
        ptr += s * s;
    }

    if (length == 0)
    {
        std::free(buffer);
        return { 0, nullptr };
    }
    return { length, reinterpret_cast<uint8_t*>(buffer) };
}

} // namespace anonymous

void VaX11Window::setIconFile(const std::string& file)
{
    if (!fMapped)
    {
        fOperationCache.iconFile = file;
        return;
    }

    auto result = LoadIconFile(file);
    if (std::get<0>(result) == 0)
        return;

    int32_t length = std::get<0>(result);
    auto *ptru32 = reinterpret_cast<uint32_t*>(std::get<1>(result));

#if defined(__x86_64__)
    auto *ptrul = reinterpret_cast<unsigned long*>(std::malloc(length * sizeof(unsigned long)));
    for (int32_t i = 0; i < length; i++)
        ptrul[i] = ptru32[i];
    std::free(ptru32);
#endif /* __x86_64__ */

#if defined(__i386__)
    auto *ptrul = ptru32;
#endif /* __i386__ */

    auto display = std::dynamic_pointer_cast<VaX11Display>(getDisplay());
    XChangeProperty(fXDisplay,
                    fWindow,
                    display->atoms().get(VaX11Atoms::_NET_WM_ICON),
                    display->atoms().get(VaX11Atoms::CARDINAL),
                    32,
                    PropModeReplace,
                    reinterpret_cast<uint8_t*>(ptrul),
                    length);
    display->flush();
    std::free(ptrul);
}

void VaX11Window::show()
{
    XMapWindow(fXDisplay, fWindow);
    VaWindow::update();
    getDisplay()->flush();
}

int32_t VaX11Window::width()
{
    return fWidth;
}

int32_t VaX11Window::height()
{
    return fHeight;
}

void VaX11Window::update(const SkRect& rect)
{
    auto x = static_cast<int32_t>(rect.x()),
         y = static_cast<int32_t>(rect.y()),
         w = static_cast<int32_t>(rect.width()),
         h = static_cast<int32_t>(rect.height());

    XEvent event;
    std::memset(&event, 0x00, sizeof(XEvent));

    event.type = Expose;
    event.xexpose.window = fWindow;
    event.xexpose.x = x;
    event.xexpose.y = y;
    event.xexpose.width = w;
    event.xexpose.height = h;
    XSendEvent(fXDisplay, fWindow, False, ExposureMask, &event);
    getDisplay()->flush();
}

void VaX11Window::dispatchCloseEvent()
{
    VaWindow::signalClose().emit(shared_from_this());
}

void VaX11Window::dispatchConfigureEvent(XConfigureEvent& ev)
{
    fWidth = ev.width;
    fHeight = ev.height;
    SkRect rect = SkRect::MakeXYWH(static_cast<SkScalar>(ev.x),
                                   static_cast<SkScalar>(ev.y),
                                   static_cast<SkScalar>(ev.width),
                                   static_cast<SkScalar>(ev.height));
    VaWindow::signalConfigure().emit(shared_from_this(), rect);
}

void VaX11Window::dispatchMapEvent()
{
    fMapped = true;
    if (fOperationCache.title.has_value())
    {
        setTitle(fOperationCache.title.value());
        fOperationCache.title.reset();
    }

    if (fOperationCache.resizable.has_value())
    {
        setResizable(fOperationCache.resizable.value());
        fOperationCache.resizable.reset();
    }

    if (fOperationCache.iconFile.has_value())
    {
        setIconFile(fOperationCache.iconFile.value());
        fOperationCache.iconFile.reset();
    }

    VaWindow::signalMap().emit(shared_from_this());
}

void VaX11Window::dispatchUnmapEvent()
{
    fMapped = false;
    VaWindow::signalUnmap().emit(shared_from_this());
}

void VaX11Window::dispatchExposure(XExposeEvent& expose)
{
    SkRect rect = SkRect::MakeXYWH(static_cast<SkScalar>(expose.x),
                                   static_cast<SkScalar>(expose.y),
                                   static_cast<SkScalar>(expose.width),
                                   static_cast<SkScalar>(expose.height));
    VaWindow::signalRepaint().emit(shared_from_this(), rect);
}

void VaX11Window::onClose()
{
    XDestroyWindow(fXDisplay, fWindow);
    getDisplay()->flush();
}

VANILLA_NS_END
