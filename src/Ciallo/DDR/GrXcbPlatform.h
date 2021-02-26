#ifndef COCOA_GRXCBPLATFORM_H
#define COCOA_GRXCBPLATFORM_H

#include <memory>
#include <mutex>

#include <xcb/xcb.h>
#include <xcb/xcb_renderutil.h>
#include <xcb/xcb_image.h>

#include "Ciallo/GrBase.h"
#include "Ciallo/DDR/GrBaseCompositor.h"
#include "Ciallo/DDR/GrBasePlatform.h"
CIALLO_BEGIN_NS

class GrXcbPlatform : public GrBasePlatform
{
public:
    GrXcbPlatform(::xcb_connection_t *connection,
                  ::xcb_window_t window,
                  int32_t width,
                  int32_t height,
                  ::xcb_get_window_attributes_reply_t *attrs,
                  ::xcb_screen_t *screen,
                  const GrPlatformOptions& options);
    ~GrXcbPlatform() override;

    static std::unique_ptr<GrBasePlatform> MakeFromXcbWindow(::xcb_connection_t *connection,
                                                             ::xcb_window_t window,
                                                             int screenp,
                                                             const GrPlatformOptions& options);

    inline int32_t width() const   { return fWidth; }
    inline int32_t height() const  { return fHeight; }

private:
    std::unique_ptr<::xcb_render_pictforminfo_t> getPictFormInfo();
    void setPictureFormatInfo();

    std::shared_ptr<GrBaseCompositor> onCreateCompositor() override;
    void onPresent(const uint8_t *pBuffer) override;

    std::shared_ptr<GrBaseCompositor> createGpuCompositor();
    std::shared_ptr<GrBaseCompositor> createCpuCompositor();
    std::shared_ptr<GrBaseCompositor> createOpenCLCompositor();
    void createXcbResources();

private:
    ::xcb_connection_t          *fConnection;
    ::xcb_window_t               fWindow;
    int32_t                      fWidth;
    int32_t                      fHeight;
    ::xcb_get_window_attributes_reply_t
                                *fWindowAttributes;
    ::xcb_screen_t              *fScreen;

    ::xcb_gcontext_t             fXcbGContext;
    size_t                       fBufferSize;
    GrColorFormat                fColorFormat;
};

CIALLO_END_NS
#endif //COCOA_GRXCBPLATFORM_H
