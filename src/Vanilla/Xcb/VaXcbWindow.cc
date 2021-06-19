#include <xcb/xcb.h>
#include <xcb/xcb_util.h>
#include <xcb/xcb_icccm.h>

#include "include/core/SkPixmap.h"
#include "include/core/SkImage.h"
#include "include/codec/SkCodec.h"

#include "Vanilla/Base.h"
#include "Vanilla/VaWindow.h"
#include "Vanilla/Xcb/VaXcbDisplay.h"
#include "Vanilla/Xcb/VaXcbWindow.h"
VANILLA_NS_BEGIN

VaXcbWindow::VaXcbWindow(WeakHandle<VaXcbDisplay> display,
                         xcb_window_t window,
                         int32_t width,
                         int32_t height,
                         SkColorType format)
    : VaWindow(std::move(display), format),
      fConnection(nullptr),
      fWindow(window),
      fWidth(width),
      fHeight(height)
{
    fConnection = std::dynamic_pointer_cast<VaXcbDisplay>(getDisplay())->connection();
}

VaXcbWindow::~VaXcbWindow()
{
    xcb_destroy_window(fConnection, fWindow);
    xcb_flush(fConnection);
}

void VaXcbWindow::onClose()
{
    xcb_destroy_window(fConnection, fWindow);
    xcb_flush(fConnection);
}

void VaXcbWindow::setTitle(const std::string& title)
{
    xcb_change_property(fConnection,
                        XCB_PROP_MODE_REPLACE,
                        fWindow,
                        XCB_ATOM_WM_NAME,
                        XCB_ATOM_STRING,
                        8,
                        title.length(),
                        title.c_str());
    xcb_flush(fConnection);
}

void VaXcbWindow::setResizable(bool resizable)
{
    int32_t w = this->width(), h = this->height();

    xcb_size_hints_t hints{
            .flags = XCB_ICCCM_SIZE_HINT_P_MIN_SIZE | XCB_ICCCM_SIZE_HINT_P_MAX_SIZE,
            .min_width = w,
            .min_height = h,
            .max_width = w,
            .max_height = h
    };

    if (resizable)
    {
        hints.min_width = 0;
        hints.min_height = 0;
        hints.max_width = 0xffff;
        hints.max_height = 0xffff;
    }

    xcb_change_property(fConnection,
                        XCB_PROP_MODE_REPLACE,
                        fWindow,
                        XCB_ATOM_WM_NORMAL_HINTS,
                        XCB_ATOM_WM_SIZE_HINTS,
                        32,
                        sizeof(hints) >> 2,
                        &hints);
    xcb_flush(fConnection);
}

namespace {

/**
 * A image file which you want to set as icon should be in a special format.
 * See "_NET_WM_ICON" section at https://specifications.freedesktop.org/wm-spec/wm-spec-latest.html.
 */
std::tuple<int32_t, uint8_t*> load_icon_file_cardinal(const std::string& file)
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

void VaXcbWindow::setIconFile(const std::string& file)
{
    auto [size, ptr] = load_icon_file_cardinal(file);
    if (size == 0)
        return;

    auto display = std::dynamic_pointer_cast<VaXcbDisplay>(getDisplay());
    xcb_change_property(fConnection,
                        XCB_PROP_MODE_REPLACE,
                        fWindow,
                        display->atom(VaXcbAtoms::_NET_WM_ICON),
                        display->atom(VaXcbAtoms::CARDINAL),
                        32,
                        size, ptr);
    xcb_flush(fConnection);
    std::free(ptr);
}

void VaXcbWindow::show()
{
    xcb_map_window(fConnection, fWindow);
    xcb_flush(fConnection);
}

int32_t VaXcbWindow::width()
{
    return fWidth;
}

int32_t VaXcbWindow::height()
{
    return fHeight;
}

void VaXcbWindow::update(const SkRect& rect)
{
    xcb_expose_event_t event{};
    event.response_type = XCB_EXPOSE;
    event.sequence = 0;
    event.window = fWindow;
    event.x = static_cast<int32_t>(rect.x());
    event.y = static_cast<int32_t>(rect.y());
    event.width = static_cast<int32_t>(rect.width());
    event.height = static_cast<int32_t>(rect.height());

    xcb_send_event(fConnection,
                   false, fWindow, XCB_EVENT_MASK_EXPOSURE,
                   reinterpret_cast<const char*>(&event));
    xcb_flush(fConnection);
}

#define emit_event(_signal, ...) signal##_signal ().emit(shared_from_this(), __VA_ARGS__)
#define emit_event_no_arg(_signal) signal##_signal ().emit(shared_from_this())

VA_WIN_HANDLER_IMPL(VaXcbWindow, expose)
{
    SkRect rect = SkRect::MakeXYWH(event->x, event->y,
                                   event->width, event->height);
    emit_event(Repaint, rect);
}

VA_WIN_HANDLER_IMPL(VaXcbWindow, configure_notify)
{
    fWidth = event->width;
    fHeight = event->height;
    SkRect rect = SkRect::MakeXYWH(event->x, event->y,
                                   event->width, event->height);
    emit_event(Configure, rect);
}

VA_WIN_HANDLER_IMPL(VaXcbWindow, client_message)
{
    auto display = std::dynamic_pointer_cast<VaXcbDisplay>(getDisplay());
    if (event->data.data32[0] == display->atom(VaXcbAtoms::WM_DELETE_WINDOW))
    {
        emit_event_no_arg(Close);
    }
}

VA_WIN_HANDLER_IMPL(VaXcbWindow, map_notify)
{
    emit_event_no_arg(Map);
}

VA_WIN_HANDLER_IMPL(VaXcbWindow, unmap_notify)
{
    emit_event_no_arg(Unmap);
}

#undef emit_event
VANILLA_NS_END
