#ifndef COCOA_GRRASTERCOMPOSITOR_H
#define COCOA_GRRASTERCOMPOSITOR_H

#include <memory>

#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkBitmap.h"

#include "Ciallo/Ciallo.h"
#include "Ciallo/Skia2d/GrBaseCompositor.h"
CIALLO_BEGIN_NS

#define CIALLO_ROMAN_CPU_DRIVER_VERSION     210123
#define CIALLO_ROMAN_CPU_API_VERSION        210123
#define CIALLO_ROMAN_CPU_VENDOR             0
#define CIALLO_ROMAN_CPU_DEVICE_NAME        "Skia2d [CPU]"

class GrRasterCompositor : public GrBaseCompositor
{
    friend class GrBaseCompositor;

public:
    explicit GrRasterCompositor(Drawable *drawable);
    ~GrRasterCompositor() override;

private:
    SkSurface *onTargetSurface() override;
    void onPresent() override;
    GrBaseRenderLayer *onCreateRenderLayer(int32_t width,
                                           int32_t height,
                                           int32_t left,
                                           int32_t top,
                                           int zindex) override;

    void createSurface();

private:
    sk_sp<SkSurface>             fBitmapSurface;
};

CIALLO_END_NS
#endif //COCOA_GRRASTERCOMPOSITOR_H
