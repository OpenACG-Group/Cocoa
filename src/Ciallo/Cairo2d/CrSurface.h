#ifndef COCOA_CRSURFACE_H
#define COCOA_CRSURFACE_H

#include <memory>
#include <cairo.h>

#include "Ciallo/Ciallo.h"
#include "Ciallo/Drawable.h"
#include "Ciallo/Cairo2d/Cairo2d.h"
CIALLO_BEGIN_NS

class CrSurface
{
public:
    using Ptr = std::shared_ptr<CrSurface>;

    enum class [[nodiscard]] Backend
    {
        kDrawableSurface,
        kImageSurface,
        kRecordingSurface
    };

    virtual ~CrSurface();

    static std::shared_ptr<CrSurface> MakeFromDrawable(Drawable *drawable);
    static std::shared_ptr<CrSurface> MakeImage(int32_t width, int32_t height);
    static std::shared_ptr<CrSurface> MakeImage(const std::string& file);
    static std::shared_ptr<CrSurface> MakeRecording(cairo_content_t content, const CrRect& cull);
    static std::shared_ptr<CrSurface> MakeRecording(cairo_content_t content);

    Backend backend() const;
    virtual int32_t width() = 0;
    virtual int32_t height() = 0;

    /**
     * @brief Get the stride.
     * @return Stride value if backend is ImageSurface,
     *         otherwise it's 0.
     */
    virtual int32_t stride();

    void flush();
    void finish();
    void writeToPNG(const std::string& file);
    void markDirty();
    void markDirty(int x, int y, int width, int height);
    void setDeviceScale(double xScale, double yScale);
    void setDeviceOffset(double xOffset, double yOffset);

    /**
     * @brief Resize a Drawable surface when you receiving
     *        a "reconfigure" event from window system.
     */
    virtual void resize(int width, int height);

    /**
     * @brief Get pixels buffer.
     * @return Address of pixels if backend is ImageSurface,
     *         otherwise it's nullptr.
     */
    virtual uint8_t *peekPixels();
    cairo_surface_t *nativeHandle();

protected:
    CrSurface(cairo_surface_t *surface, Backend backend);
    void setNative(cairo_surface_t *surface);

private:
    cairo_surface_t     *fSurface;
    Backend              fBackend;
};

CIALLO_END_NS
#endif //COCOA_CRSURFACE_H
