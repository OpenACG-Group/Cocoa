#include "include/core/SkGraphics.h"

#include "Core/Journal.h"
#include "Core/Errors.h"

#include "Gsk/Gsk.h"
#include "Gsk/GskDisplayManager.h"

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Gsk)

GSK_NAMESPACE_BEGIN

GskGlobalScope::GskGlobalScope(const GskOptions& options)
    : fOptions(options)
{
    QLOG(LOG_INFO, "Graphic Scene Kit (GSK) version {}.{}", GSK_VERSION_MAJOR, GSK_VERSION_MINOR);
    if (options.skia_allow_jit)
    {
        SkGraphics::AllowJIT();
        QLOG(LOG_INFO, "Skia is allowed to use JIT to acceleration CPU-bound operations");
    }
    if (options.vk_hw_accel)
        QLOG(LOG_INFO, "GPU-accelerated rendering is allowed");
    fDisplayManager = std::make_shared<GskDisplayManager>(this);
}

Handle<GskDisplayManager> GskGlobalScope::getDisplayManager() const
{
    CHECK(fDisplayManager);
    return fDisplayManager;
}

GskGlobalScope::~GskGlobalScope()
{
    CHECK(fDisplayManager.unique());
    fDisplayManager.reset();
}

GSK_NAMESPACE_END
