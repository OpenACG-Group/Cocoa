#ifndef COCOA_DRAWABLE_H
#define COCOA_DRAWABLE_H

#include <string>
#include <memory>
#include <cairo.h>

#include <vulkan/vulkan.h>
#include "include/core/SkPixmap.h"
#include "include/core/SkRect.h"

#include "Ciallo/Ciallo.h"
#include "Ciallo/DrawableListener.h"
CIALLO_BEGIN_NS

/**
 * Drawable is an abstraction of graphics output device, such as a
 * window or a pixel buffer.
 * The Skia2d and Cairo2d module uses a Drawable object as destination
 * of drawing.
 */
class Drawable
{
public:
    enum class [[nodiscard]] Backend
    {
        kXcb
    };

    explicit Drawable(Backend backend);
    virtual ~Drawable() = default;

    virtual ImageFormat format() const = 0;
    virtual SkIRect geometry() const = 0;

    virtual void close() = 0;
    virtual bool isClosed() const = 0;

    virtual void writePixmap(const SkPixmap& pixmap, const SkIRect& rect) = 0;
    virtual cairo_surface_t *createCairoSurface() = 0;
    virtual void resizeCairoSurface(cairo_surface_t *surface, int32_t w, int32_t h) = 0;
    virtual VkSurfaceKHR createVkSurface(VkInstance instance) = 0;

    virtual void setTitle(const std::string& title) = 0;
    virtual void setResizable(bool value) = 0;

    virtual void repaint() = 0;

    template<typename T>
    T *cast() {
        return dynamic_cast<T*>(this);
    }

    inline Backend backend() const { return fBackend; }
    void setListener(const std::shared_ptr<DrawableListener>& listener);

protected:
    DrawableListener *listener();

private:
    Backend                             fBackend;
    std::shared_ptr<DrawableListener>   fListener;
};

CIALLO_END_NS
#endif //COCOA_DRAWABLE_H
