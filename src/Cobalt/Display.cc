#include <map>

#include "Core/Journal.h"
#include "Core/Errors.h"
#include "Cobalt/Display.h"
#include "Cobalt/Wayland/WaylandDisplay.h"
COBALT_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Cobalt.Display)

namespace {
// NOLINTNEXTLINE
std::map<Backends, const char*> backends_name_map_ = {
    { Backends::kWayland, COBALT_BACKEND_WAYLAND }
};
}

COBALT_TRAMPOLINE_IMPL(Display, Close)
{
    auto this_ = info.GetThis()->As<Display>();
    this_->Close();
    info.SetReturnStatus(RenderClientObject::ReturnStatus::kOpSuccess);
}

co_sp<Display> Display::Connect(uv_loop_t *loop, const std::string& name)
{
    CHECK(loop);

    if (!GlobalScope::Instance())
    {
        QLOG(LOG_ERROR, "CobaltScope has not been initialized");
        return nullptr;
    }

    Backends backend = GlobalScope::Ref().GetOptions().GetBackend();

    QLOG(LOG_INFO, "Connecting to {} display [{}]",
         backends_name_map_[backend], name.empty() ? "default" : name);
    switch (backend)
    {
    case Backends::kWayland:
        return WaylandDisplay::Connect(loop, name);
    }

    MARK_UNREACHABLE();
}

Display::Display(uv_loop_t *eventLoop)
    : RenderClientObject(RealType::kDisplay)
    , event_loop_(eventLoop)
    , has_disposed_(false)
{
    SetMethodTrampoline(CROP_DISPLAY_CLOSE, Display_Close_Trampoline);
}

Display::~Display()
{
    CHECK(has_disposed_ && "Display must be disposed before destructing");
}

void Display::Close()
{
    if (!has_disposed_)
    {
        this->OnDispose();
        has_disposed_ = true;
        RenderClientObject::Emit(CRSI_DISPLAY_CLOSED, RenderClientEmitterInfo());
    }
}

COBALT_NAMESPACE_END
