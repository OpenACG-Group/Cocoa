#ifndef COCOA_GRCONTEXT_H
#define COCOA_GRCONTEXT_H

#include <memory>

#include "Ciallo/Ciallo.h"
#include "Ciallo/Drawable.h"
#include "Ciallo/Skia2d/GrBaseCompositor.h"
CIALLO_BEGIN_NS

class GrContextOptions
{
public:
    GrContextOptions() = default;
    ~GrContextOptions() = default;

    static GrContextOptions MakeFromPropertyTree();
    static GrContextOptions Make();

    void setStrictAccel(bool val);
    void setVulkan(bool val);
    void setVulkanDebug(bool val);

    bool strictAccel() const;
    bool vulkan() const;
    bool vulkanDebug() const;

private:
    bool    fUseStrictAccel = false;
    bool    fUseVulkan = false;
    bool    fUseVulkanDebug = false;
};

class GrContext
{
public:
    GrContext(Drawable *drawable, const GrContextOptions& options);
    ~GrContext() = default;

    inline GrBaseCompositor *comp()   { return fCompositor.get(); }

private:
    void createCompStrictly(Drawable *drawable, const GrContextOptions& opts);
    void createCompNormally(Drawable *drawable, const GrContextOptions& opts);
    void createVulkanComp(Drawable *drawable, const GrContextOptions& opts);
    void createRasterComp(Drawable *drawable, const GrContextOptions& opts);

private:
    std::unique_ptr<GrBaseCompositor> fCompositor;
};

CIALLO_END_NS
#endif //COCOA_GRCONTEXT_H
