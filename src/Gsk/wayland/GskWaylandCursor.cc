#include "include/core/SkCanvas.h"
#include "include/codec/SkCodec.h"

#include "Core/Journal.h"
#include "Core/QResource.h"
#include "Core/Data.h"

#include "Gsk/Gsk.h"
#include "Gsk/wayland/GskWaylandDisplay.h"
GSK_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Gsk.Wayland.Cursor)

namespace {

const struct {
    const char *css_name, *traditional_name;
} name_map[] = {
        { "default",      "left_ptr" },
        { "help",         "question_arrow" },
        { "context-menu", "left_ptr" },
        { "pointer",      "hand" },
        { "progress",     "left_ptr_watch" },
        { "wait",         "watch" },
        { "cell",         "crosshair" },
        { "crosshair",    "cross" },
        { "text",         "xterm" },
        { "vertical-text","xterm" },
        { "alias",        "dnd-link" },
        { "copy",         "dnd-copy" },
        { "move",         "dnd-move" },
        { "no-drop",      "dnd-none" },
        { "dnd-ask",      "dnd-copy" }, /* not CSS, but we want to guarantee it anyway */
        { "not-allowed",  "crossed_circle" },
        { "grab",         "hand2" },
        { "grabbing",     "hand2" },
        { "all-scroll",   "left_ptr" },
        { "col-resize",   "h_double_arrow" },
        { "row-resize",   "v_double_arrow" },
        { "n-resize",     "top_side" },
        { "e-resize",     "right_side" },
        { "s-resize",     "bottom_side" },
        { "w-resize",     "left_side" },
        { "ne-resize",    "top_right_corner" },
        { "nw-resize",    "top_left_corner" },
        { "se-resize",    "bottom_right_corner" },
        { "sw-resize",    "bottom_left_corner" },
        { "ew-resize",    "h_double_arrow" },
        { "ns-resize",    "v_double_arrow" },
        { "nesw-resize",  "fd_double_arrow" },
        { "nwse-resize",  "bd_double_arrow" },
        { "zoom-in",      "left_ptr" },
        { "zoom-out",     "left_ptr" }
};

const char *name_fallback (const std::string& name)
{
    for (const auto& p : name_map)
    {
        if (p.css_name == name)
            return p.traditional_name;
    }
    return nullptr;
}

wl_cursor *cursor_load_for_name(wl_cursor_theme *theme,
                                uint32_t scale,
                                const std::string& name)
{
    wl_cursor *c = wl_cursor_theme_get_cursor(theme, name.c_str(), scale);
    if (!c)
    {
        const char *fallback = name_fallback(name);
        if (fallback)
            c = wl_cursor_theme_get_cursor(theme, fallback, scale);
    }

    return c;
}

void buffer_release_callback(void *data, wl_buffer *buffer)
{
    auto *wrapped = reinterpret_cast<GskSkSurfaceWrapper*>(data);
    CHECK(wrapped->getBuffer()->buffer == buffer);
    CHECK(!wrapped->getBuffer()->display.expired());

    auto display = wrapped->getBuffer()->display.lock();
    display->removeCursorSurfaceCache(wrapped);

    delete wrapped;
    /* As soon as `wrapped` is deleted, the destructor of `GskSkSurfaceWrapper` will
     * check and destroy the SkSurface object which belongs to itself.
     * All the external resources such as Wayland buffer and mapped memory will be
     * destroyed correctly when the SkSurface is destroyed.
     * (See GskWaylandDisplay.cc:771 to find out how this works.)
     */
}

const wl_buffer_listener buffer_listener_ = {
        buffer_release_callback
};

} // namespace anonymous

wl_buffer *GskWaylandDisplay::cursorGetBuffer(const Handle<GskCursor>& cursor,
                                              uint32_t desiredScale,
                                              uint32_t imageIndex,
                                              Vec2i& hotspot,
                                              SkISize& size,
                                              int& scale)
{
    sk_sp<SkImage> texture;

    if (!cursor->getName().empty())
    {
        if (cursor->getName() == "none")
            goto none;

        wl_cursor *c = cursor_load_for_name(fCursorTheme,
                                            desiredScale,
                                            cursor->getName());
        if (c)
        {
            if (imageIndex >= c->image_count)
            {
                QLOG(LOG_WARNING, "Out of bounds cursor image index [{} / {}]",
                     imageIndex, c->image_count - 1);
                imageIndex = 0;
            }

            wl_cursor_image *image = c->images[imageIndex];

            hotspot = Vec<uint32_t, 2>(image->hotspot_x / desiredScale,
                                       image->hotspot_y / desiredScale);
            size = SkISize::Make(static_cast<int32_t>(image->width / desiredScale),
                                 static_cast<int32_t>(image->height / desiredScale));
            scale = static_cast<int>(desiredScale);

            return wl_cursor_image_get_buffer(image);
        }
    }
    else
    {
        GskSkSurfaceWrapper *wrapped;
        wl_buffer *buffer;

        texture = cursor->getTexture();

from_texture:
        if (fCursorSkSurfaceCache.count(cursor) == 0)
        {
            wrapped = createRasterShmSurface(Vec2i(texture->width(), texture->height()), 1);
            wrapped->getSurface()->getCanvas()->drawImage(texture, 0, 0);
            fCursorSkSurfaceCache[cursor] = wrapped;
        }
        else
            wrapped = fCursorSkSurfaceCache[cursor];

        hotspot = cursor->getHotspot();
        size = texture->imageInfo().dimensions();
        scale = 1;

        buffer = wrapped->getBuffer()->buffer;
        wl_buffer_add_listener(buffer, &buffer_listener_, wrapped);
        return buffer;
    }

    if (cursor->getFallback())
    {
        return cursorGetBuffer(cursor->getFallback(),
                               desiredScale,
                               imageIndex,
                               hotspot,
                               size,
                               scale);
    }
    else
    {
        auto data = QResource::Ref().Lookup("org.cocoa.internal.Gsk", "/icons/default_cursor.png");
        CHECK(data);

        auto *pBuffer = malloc(data->size());
        size_t usedSize = data->read(pBuffer, data->size());

        auto skData = SkData::MakeWithProc(pBuffer, usedSize, [](const void *data, void *ctx) {
            free(const_cast<void*>(data));
        }, nullptr);
        CHECK(skData);

        texture = SkImage::MakeFromEncoded(skData, SkAlphaType::kUnpremul_SkAlphaType);
        CHECK(texture);
        goto from_texture;
    }

none:
    hotspot = Vec2i(0, 0);
    size = SkISize::MakeEmpty();
    scale = 1;
    return nullptr;
}

uint32_t
GskWaylandDisplay::cursorGetNextImageIndex(const Handle<GskCursor>& cursor,
                                           uint32_t scale,
                                           uint32_t currentIndex,
                                           uint32_t& nextImageDelay)
{
    if (cursor->getName().empty() || cursor->getName() == "none")
    {
        nextImageDelay = 0;
        return currentIndex;
    }

    wl_cursor *c = cursor_load_for_name(fCursorTheme, scale, cursor->getName());
    if (c)
    {
        if (currentIndex >= c->image_count)
        {
            QLOG(LOG_WARNING, "Out of bounds cursor image [{} / {}]",
                 currentIndex, c->image_count - 1);
            currentIndex = 0;
        }
        if (c->image_count == 1)
        {
            nextImageDelay = 0;
            return currentIndex;
        }
        else
        {
            nextImageDelay = c->images[currentIndex]->delay;
            return (currentIndex + 1) % c->image_count;
        }
    }

    if (cursor->getFallback())
    {
        return cursorGetNextImageIndex(cursor->getFallback(),
                                       scale,
                                       currentIndex,
                                       nextImageDelay);
    }

    nextImageDelay = 0;
    return currentIndex;
}

GSK_NAMESPACE_END
