#include "Core/Journal.h"
#include "Core/Errors.h"

#include "Gsk/GskDisplayManager.h"
#include "Gsk/wayland/GskWaylandDisplay.h"
GSK_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Gsk)

GskDisplayManager::GskDisplayManager(GskGlobalScope *scope)
    : fWaylandAvailable(false)
    , fXcbAvailable(false)
{
    if (::getenv("XDG_RUNTIME_DIR") && ::getenv("WAYLAND_DISPLAY"))
        fWaylandAvailable = true;
    if (::getenv("DISPLAY"))
        fXcbAvailable = true;

    auto& options = scope->getOptions();
    for (const auto& backend : options.disallow_backends)
    {
        if (backend == GSK_BACKEND_WAYLAND)
            fWaylandAvailable = false;
        else if (backend == GSK_BACKEND_XCB)
            fXcbAvailable = false;
        else if (backend == "*")
        {
            fWaylandAvailable = false;
            fXcbAvailable = false;
        }
    }
    if (fWaylandAvailable)
        QLOG(LOG_INFO, "Wayland (Wayland Compositor) backend is available");
    if (fXcbAvailable)
        QLOG(LOG_INFO, "XCB (X11 Display Server) backend is available");
}

Handle<GskDisplay> GskDisplayManager::openDisplay(const std::string& displayName)
{
    CHECK(!fDefaultDisplay);

    Handle<GskDisplay> display;
    if (fWaylandAvailable)
        display = GskWaylandDisplay::Make(displayName);
    if (!display && fXcbAvailable)
    {
        // TODO: Create display for X11 backend
    }

    if (!display)
        return nullptr;

    g_signal_emit(DisplayOpen, display);
    fDefaultDisplay = display;
    return display;
}

GSK_NAMESPACE_END
