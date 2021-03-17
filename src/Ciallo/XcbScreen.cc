#include <xcb/xcb.h>
#include <xcb/randr.h>

#include <endian.h>
#include "include/core/SkImageInfo.h"
#include "include/core/SkBitmap.h"

#include "Ciallo/Ciallo.h"
#include "Ciallo/XcbScreen.h"
#include "Ciallo/XcbConnection.h"
CIALLO_BEGIN_NS

namespace {

std::unique_ptr<SkBitmap> xcbPixmapToBitmapUnique(XcbScreen *pScreen,
                                                  xcb_connection_t *connection,
                                                  xcb_pixmap_t pixmap,
                                                  const SkISize& size,
                                                  int depth,
                                                  const xcb_visualtype_t *visualType)
{
    auto *imageReply = xcb_get_image_reply(connection,
                                           xcb_get_image_unchecked(connection,
                                                                   XCB_IMAGE_FORMAT_Z_PIXMAP,
                                                                   pixmap,
                                                                   0, 0,
                                                                   size.width(), size.height(),
                                                                   0xffffffff),
                                           nullptr);
    if (!imageReply)
        return nullptr;
    BeforeLeaveScope beforeLeaveScope([&imageReply]() -> void {
        std::free(imageReply);
    });

    std::unique_ptr<SkBitmap> bitmap = std::make_unique<SkBitmap>();
    ImageFormat format = pScreen->imageFormat(visualType, depth);
    if (format == ImageFormat::kUnknown)
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("Unsupported image format")
                .make<RuntimeException>();
    }

    auto colorInfo = SkColorInfoFromImageFormat(format);
    SkImageInfo info = SkImageInfo::Make(size,
                                         std::get<0>(colorInfo),
                                         std::get<1>(colorInfo));
    bitmap->allocPixels(info);

    auto *pImageData = reinterpret_cast<uint32_t*>(xcb_get_image_data(imageReply));
    // Fix up alpha channel and copy pixels
    for (int32_t y = 0; y < size.height(); y++)
    {
        uint32_t *pScanline = pImageData + y * size.width();
        for (int32_t x = 0; x < size.width(); x++)
        {
            uint32_t *pAddr = bitmap->getAddr32(x, y);
            *pAddr = *pScanline | 0xff000000U;
            pScanline++;
        }
    }

    return bitmap;
}

} // namespace anonymous

XcbScreen::XcbScreen(XcbConnection *connection, xcb_screen_t *screen)
    : fConnection(connection),
      fScreen(screen),
      fRefreshRate(-1),
      fGeometry(SkIRect::MakeEmpty())
{
    xcb_depth_iterator_t depthIterator = xcb_screen_allowed_depths_iterator(fScreen);
    while (depthIterator.rem)
    {
        xcb_depth_t *pDepth = depthIterator.data;
        xcb_visualtype_iterator_t visualtypeIterator =
                xcb_depth_visuals_iterator(pDepth);

        while (visualtypeIterator.rem)
        {
            xcb_visualtype_t *pVisualType = visualtypeIterator.data;
            fVisuals[pVisualType->visual_id] = *pVisualType;
            fDepths[pVisualType->visual_id] = pDepth->depth;
            xcb_visualtype_next(&visualtypeIterator);
        }

        xcb_depth_next(&depthIterator);
    }

    auto cookie = xcb_get_geometry(fConnection->nativeHandle(),
                                   fScreen->root);
    auto *reply = xcb_get_geometry_reply(fConnection->nativeHandle(), cookie, nullptr);
    if (reply == nullptr)
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("Failed to get geometry for root window")
                .make<RuntimeException>();
    }
    fGeometry.setXYWH(reply->x, reply->y, reply->width, reply->height);
    std::free(reply);
}

ImageFormat XcbScreen::imageFormat(xcb_visualid_t visualId, uint8_t depth)
{
    if (!fVisuals.contains(visualId))
        return ImageFormat::kUnknown;
    return imageFormat(&fVisuals[visualId], depth);
}

ImageFormat XcbScreen::imageFormat(const xcb_visualtype_t *visualType, uint8_t depth)
{
    if (depth == 8)
    {
        if (visualType->_class == XCB_VISUAL_CLASS_GRAY_SCALE)
            return ImageFormat::kGrayscale_8;
        return ImageFormat::kUnknown;
    }

    const xcb_format_t *pFormat = fConnection->formatForDepth(depth);
    if (pFormat == nullptr)
        return ImageFormat::kUnknown;

    if (pFormat->bits_per_pixel != 32)
        return ImageFormat::kUnknown;

    constexpr ImageFormat fmt[7]{
        ImageFormat::kBGRA_8888,
        ImageFormat::kRGBA_8888,
        ImageFormat::kBGRA_8888_Premultiplied,
        ImageFormat::kRGBA_8888_Premultiplied,
        ImageFormat::kUnknown,
        ImageFormat::kUnknown,
        ImageFormat::kUnknown
    };

    /* [N]: Is red_mask 0xff0000? (N = [0, 1]) */
    /* [N][M]: Is blue_mask 0x0000ff? (M = [0, 1] */
#if __BYTE_ORDER == __LITTLE_ENDIAN
    constexpr int fmt_base_idx[2][2]{
            { 1, 4 },
            { 4, 0 }
    };
#else
    constexpr int fmt_base_idx[2][2]{
            { 0, 4 },
            { 4, 1 }
    };
#endif

    uint32_t red_mask = visualType->red_mask;
    uint32_t blue_mask = visualType->blue_mask;
    int fmt_idx_offset = (depth == 32) ? 2 : 0;

    return fmt[fmt_base_idx[red_mask == 0xff0000][blue_mask == 0xff] + fmt_idx_offset];
}

SkIPoint XcbScreen::translate(xcb_window_t src, xcb_window_t dst, const SkIPoint& pointInChild)
{
    auto cookie = xcb_translate_coordinates_unchecked(fConnection->nativeHandle(),
                                                      src,
                                                      dst,
                                                      pointInChild.x(),
                                                      pointInChild.y());
    auto *reply = xcb_translate_coordinates_reply(fConnection->nativeHandle(),
                                                  cookie, nullptr);

    if (reply == nullptr)
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("Failed to translate coordinates by XCB")
                .make<RuntimeException>();
    }

    SkIPoint point = SkIPoint::Make(reply->dst_x, reply->dst_y);
    std::free(reply);

    return point;
}

std::unique_ptr<SkBitmap> XcbScreen::grabWindow(xcb_window_t window, const SkIRect& rect)
{
    uint8_t depth;
    SkISize windowSize(SkISize::MakeEmpty());
    SkIRect regionRect(SkIRect::MakeEmpty());

    if (window == XCB_NONE)
    {
        window = fScreen->root;
        depth = fScreen->root_depth;
        windowSize.set(fGeometry.width(), fGeometry.height());
        regionRect.setXYWH(rect.x() + fGeometry.x(),
                           rect.y() + fGeometry.y(),
                           rect.width(),
                           rect.height());
    }
    else
    {
        auto cookie = xcb_get_geometry(fConnection->nativeHandle(),
                                       window);
        auto *geometryReply = xcb_get_geometry_reply(fConnection->nativeHandle(),
                                                     cookie, nullptr);

        depth = geometryReply->depth;
        windowSize.set(geometryReply->width, geometryReply->height);
        std::free(geometryReply);
        regionRect = rect;
    }

    if (regionRect.width() < 0)
    {
        regionRect.setXYWH(regionRect.x(),
                           regionRect.y(),
                           windowSize.width() - regionRect.x(),
                           regionRect.height());
    }

    if (regionRect.height() < 0)
    {
        regionRect.setXYWH(regionRect.x(),
                           regionRect.y(),
                           regionRect.width(),
                           windowSize.height() - regionRect.y());
    }

    auto *attrs = xcb_get_window_attributes_reply(fConnection->nativeHandle(),
                                                  xcb_get_window_attributes_unchecked(fConnection->nativeHandle(), window),
                                                  nullptr);
    if (attrs == nullptr)
        return nullptr;

    xcb_visualtype_t *visual = &fVisuals[attrs->visual];
    std::free(attrs);

    xcb_pixmap_t pixmap = xcb_generate_id(fConnection->nativeHandle());
    xcb_create_pixmap(fConnection->nativeHandle(),
                      depth,
                      pixmap,
                      window,
                      regionRect.width(),
                      regionRect.height());

    uint32_t gcValueMask =XCB_GC_SUBWINDOW_MODE;
    uint32_t gcValueList[] = { XCB_SUBWINDOW_MODE_INCLUDE_INFERIORS };

    xcb_gcontext_t gc = xcb_generate_id(fConnection->nativeHandle());
    xcb_create_gc(fConnection->nativeHandle(),
                  gc, pixmap, gcValueMask, gcValueList);

    xcb_copy_area(fConnection->nativeHandle(),
                  window,
                  pixmap,
                  gc,
                  0, 0,
                  regionRect.x(),
                  regionRect.y(),
                  regionRect.width(),
                  regionRect.height());

    BeforeLeaveScope beforeLeaveScope([&]() -> void {
        xcb_free_gc(fConnection->nativeHandle(), gc);
        xcb_free_pixmap(fConnection->nativeHandle(), pixmap);
    });

    return xcbPixmapToBitmapUnique(this,
                                   fConnection->nativeHandle(),
                                   pixmap,
                                   SkISize::Make(regionRect.width(), regionRect.height()),
                                   depth,
                                   visual);
}

xcb_visualtype_t *XcbScreen::visualType(xcb_visualid_t id)
{
    if (!fVisuals.contains(id))
        return nullptr;
    return &fVisuals[id];
}

CIALLO_END_NS
