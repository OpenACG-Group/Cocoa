#include <cairo.h>

#include "Core/Exception.h"
#include "Ciallo/Ciallo.h"
#include "Ciallo/Cairo2d/CrSurface.h"
#include "Ciallo/Cairo2d/CrDrawableSurface.h"
#include "Ciallo/Cairo2d/CrImageSurface.h"
#include "Ciallo/Cairo2d/CrRecordingSurface.h"
CIALLO_BEGIN_NS

std::shared_ptr<CrSurface> CrSurface::MakeFromDrawable(Drawable *drawable)
{
    return std::make_shared<CrDrawableSurface>(drawable);
}

std::shared_ptr<CrSurface> CrSurface::MakeImage(int32_t width, int32_t height)
{
    return std::make_shared<CrImageSurface>(width, height);
}

std::shared_ptr<CrSurface> CrSurface::MakeImage(const std::string& file)
{
    return std::make_shared<CrImageSurface>(file);
}

std::shared_ptr<CrSurface> CrSurface::MakeRecording(cairo_content_t content, const CrRect& cull)
{
    return std::make_shared<CrRecordingSurface>(content, cull);
}

std::shared_ptr<CrSurface> CrSurface::MakeRecording(cairo_content_t content)
{
    return std::make_shared<CrRecordingSurface>(content);
}

// ----------------------------------------------------------------------

CrSurface::CrSurface(cairo_surface_t *surface, Backend backend)
    : fSurface(surface),
      fBackend(backend)
{
}

CrSurface::~CrSurface()
{
    if (fSurface)
        cairo_surface_destroy(fSurface);
}

CrSurface::Backend CrSurface::backend() const
{
    return fBackend;
}

void CrSurface::flush()
{
    cairo_surface_flush(fSurface);
}

void CrSurface::finish()
{
    cairo_surface_finish(fSurface);
}

void CrSurface::resize(int width, int height)
{
}

void CrSurface::writeToPNG(const std::string& file)
{
    cairo_status_t stat = cairo_surface_write_to_png(fSurface, file.c_str());
    if (stat != CAIRO_STATUS_SUCCESS)
    {
        const char *pErr = cairo_status_to_string(stat);
        throw RuntimeException::Builder(__FUNCTION__)
                .append("Failed to write Cairo surface to PNG file: ")
                .append(pErr)
                .make<RuntimeException>();
    }
}

void CrSurface::markDirty()
{
    cairo_surface_mark_dirty(fSurface);
}

void CrSurface::markDirty(int x, int y, int width, int height)
{
    cairo_surface_mark_dirty_rectangle(fSurface, x, y, width, height);
}

void CrSurface::setDeviceScale(double xScale, double yScale)
{
    cairo_surface_set_device_scale(fSurface, xScale, yScale);
}

void CrSurface::setDeviceOffset(double xOffset, double yOffset)
{
    cairo_surface_set_device_offset(fSurface, xOffset, yOffset);
}

cairo_surface_t *CrSurface::nativeHandle()
{
    return fSurface;
}

int32_t CrSurface::stride()
{
    return 0;
}

uint8_t *CrSurface::peekPixels()
{
    return nullptr;
}

void CrSurface::setNative(cairo_surface_t *surface)
{
    fSurface = surface;
}

// -----------------------------------------------------------------------------------

CrDrawableSurface::CrDrawableSurface(Drawable *drawable)
    : CrSurface(drawable->createCairoSurface(), Backend::kDrawableSurface),
      fDrawable(drawable)
{
}

int32_t CrDrawableSurface::height()
{
    return fDrawable->geometry().height();
}

int32_t CrDrawableSurface::width()
{
    return fDrawable->geometry().width();
}

void CrDrawableSurface::resize(int width, int height)
{
    fDrawable->resizeCairoSurface(CrSurface::nativeHandle(), width, height);
}

// -----------------------------------------------------------------------------------

CrImageSurface::CrImageSurface(int32_t width, int32_t height)
    : CrSurface(nullptr, Backend::kImageSurface)
{
    cairo_surface_t *surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    if (surf == nullptr)
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("Cairo2d: Failed to create an image surface")
                .make<RuntimeException>();
    }

    CrSurface::setNative(surf);
}

CrImageSurface::CrImageSurface(const std::string& file)
    : CrSurface(nullptr, Backend::kImageSurface)
{
    cairo_surface_t *surf = cairo_image_surface_create_from_png(file.c_str());
    if (surf == nullptr)
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("Cairo2d: Failed to create an image surface from PNG file")
                .make<RuntimeException>();
    }

    CrSurface::setNative(surf);
}

uint8_t *CrImageSurface::peekPixels()
{
    return cairo_image_surface_get_data(nativeHandle());
}

int32_t CrImageSurface::width()
{
    return cairo_image_surface_get_width(nativeHandle());
}

int32_t CrImageSurface::height()
{
    return cairo_image_surface_get_height(nativeHandle());
}

int32_t CrImageSurface::stride()
{
    return cairo_image_surface_get_stride(nativeHandle());
}

// ------------------------------------------------------------------------------------

CrRecordingSurface::CrRecordingSurface(cairo_content_t content, const CrRect& cullRect)
    : CrSurface(nullptr, Backend::kRecordingSurface)
{
    cairo_surface_t *surf = cairo_recording_surface_create(content, cullRect.const_ptr());
    if (surf == nullptr)
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("Cairo2d: Failed to create an recording surface")
                .make<RuntimeException>();
    }

    CrSurface::setNative(surf);
}

CrRecordingSurface::CrRecordingSurface(cairo_content_t content)
    : CrSurface(nullptr, Backend::kRecordingSurface)
{
    cairo_surface_t *surf = cairo_recording_surface_create(content, nullptr);
    if (surf == nullptr)
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("Cairo2d: Failed to create an recording surface")
                .make<RuntimeException>();
    }

    CrSurface::setNative(surf);
}

int32_t CrRecordingSurface::width()
{
    cairo_rectangle_t rect;
    cairo_recording_surface_get_extents(nativeHandle(), &rect);
    return rect.width;
}

int32_t CrRecordingSurface::height()
{
    cairo_rectangle_t rect;
    cairo_recording_surface_get_extents(nativeHandle(), &rect);
    return rect.height;
}

CIALLO_END_NS
