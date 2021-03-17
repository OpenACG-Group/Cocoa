#include <memory>
#include <vector>

#include "Core/PropertyTree.h"
#include "Core/Journal.h"

#include "Ciallo/Ciallo.h"
#include "Ciallo/Skia2d/GrContext.h"
CIALLO_BEGIN_NS

GrContextOptions GrContextOptions::Make()
{
    return GrContextOptions();
}

GrContextOptions GrContextOptions::MakeFromPropertyTree()
{
    GrContextOptions opts;

    opts.setStrictAccel(PropertyTree::Ref()("/runtime/features/useStrictHardwareDraw")
                        ->cast<PropertyTreeDataNode>()->value());
    opts.setVulkan(PropertyTree::Ref()("/runtime/features/useGpuDraw")
                        ->cast<PropertyTreeDataNode>()->value());
    opts.setVulkanDebug(PropertyTree::Ref()("/runtime/features/enableGpuDebugJournal")
                        ->cast<PropertyTreeDataNode>()->value());
    return opts;
}

void GrContextOptions::setStrictAccel(bool val)
{
    fUseStrictAccel = val;
}

void GrContextOptions::setVulkan(bool val)
{
    fUseVulkan = val;
}

void GrContextOptions::setVulkanDebug(bool val)
{
    fUseVulkanDebug = val;
}

bool GrContextOptions::strictAccel() const
{
    return fUseStrictAccel;
}

bool GrContextOptions::vulkan() const
{
    return fUseVulkan;
}

bool GrContextOptions::vulkanDebug() const
{
    return fUseVulkanDebug;
}

// -------------------------------------------------------------------------------

GrContext::GrContext(Drawable *drawable, const GrContextOptions& options)
    : fCompositor(nullptr)
{
    if (options.strictAccel())
        createCompStrictly(drawable, options);
    else
        createCompNormally(drawable, options);
}

void GrContext::createCompStrictly(Drawable *drawable, const GrContextOptions& opts)
{
    if (opts.vulkan())
        createVulkanComp(drawable, opts);
    else
        createRasterComp(drawable, opts);
}

void GrContext::createCompNormally(Drawable *drawable, const GrContextOptions& opts)
{
    if (opts.vulkan())
    {
        try {
            createVulkanComp(drawable, opts);
        } catch (const RuntimeException& e) {
            log_write(LOG_ERROR) << "Failed to create GPU compositor, using software composition instead." << log_endl;
            log_write(LOG_ERROR) << "Enable features.useStrictHardwareDraw to see what happens." << log_endl;
        }
        createRasterComp(drawable, opts);
    }
    else
        createRasterComp(drawable, opts);
}

void GrContext::createRasterComp(Drawable *drawable, const GrContextOptions& opts)
{
    fCompositor = GrBaseCompositor::MakeRaster(drawable);
}

void GrContext::createVulkanComp(Drawable *drawable, const GrContextOptions& opts)
{
    std::vector<std::string> extensions;
    switch (drawable->backend())
    {
    case Drawable::Backend::kXcb:
        extensions.emplace_back("VK_KHR_surface");
        extensions.emplace_back("VK_KHR_xcb_surface");
        break;
    }

    fCompositor = GrBaseCompositor::MakeVulkan(drawable,
                                               extensions,
                                               {},
                                               opts.vulkanDebug());
}

CIALLO_END_NS
