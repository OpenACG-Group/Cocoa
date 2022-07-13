#include <map>

#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkFontMgr.h"

#include "Core/Journal.h"
#include "Core/Errors.h"
#include "Glamor/Display.h"
#include "Glamor/Surface.h"
#include "Glamor/Wayland/WaylandDisplay.h"
GLAMOR_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Glamor.Display)

namespace {
// NOLINTNEXTLINE
std::map<Backends, const char*> backends_name_map_ = {
    { Backends::kWayland, GLAMOR_BACKEND_WAYLAND }
};
}

GLAMOR_TRAMPOLINE_IMPL(Display, Close)
{
    auto this_ = info.GetThis()->As<Display>();
    this_->Close();
    info.SetReturnStatus(RenderClientObject::ReturnStatus::kOpSuccess);
}

GLAMOR_TRAMPOLINE_IMPL(Display, CreateRasterSurface)
{
    GLAMOR_TRAMPOLINE_CHECK_ARGS_NUMBER(3);
    auto this_ = info.GetThis()->As<Display>();
    Shared<Surface> result = this_->CreateRasterSurface(info.Get<int32_t>(0),
                                                        info.Get<int32_t>(1),
                                                        info.Get<SkColorType>(2));
    if (result == nullptr)
        info.SetReturnStatus(RenderClientObject::ReturnStatus::kOpFailed);
    else
    {
        info.SetReturnStatus(RenderClientObject::ReturnStatus::kOpSuccess);
        info.SetReturnValue(result->Cast<RenderClientObject>());
    }
}

GLAMOR_TRAMPOLINE_IMPL(Display, CreateHWComposeSurface)
{
    GLAMOR_TRAMPOLINE_CHECK_ARGS_NUMBER(3);
    auto this_ = info.GetThis()->As<Display>();
    Shared<Surface> result = this_->CreateHWComposeSurface(info.Get<int32_t>(0),
                                                           info.Get<int32_t>(1),
                                                           info.Get<SkColorType>(2));
    if (result == nullptr)
        info.SetReturnStatus(RenderClientObject::ReturnStatus::kOpFailed);
    else
    {
        info.SetReturnStatus(RenderClientObject::ReturnStatus::kOpSuccess);
        info.SetReturnValue(result->Cast<RenderClientObject>());
    }
}

GLAMOR_TRAMPOLINE_IMPL(Display, RequestMonitorList)
{
    auto this_ = info.GetThis()->As<Display>();
    info.SetReturnValue(this_->RequestMonitorList());
    info.SetReturnStatus(RenderClientCallInfo::Status::kOpSuccess);
}

Shared<Display> Display::Connect(uv_loop_t *loop, const std::string& name)
{
    CHECK(loop);

    if (!GlobalScope::Instance())
    {
        QLOG(LOG_ERROR, "GlamorScope has not been initialized");
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
    SetMethodTrampoline(CROP_DISPLAY_CREATE_RASTER_SURFACE, Display_CreateRasterSurface_Trampoline);
    SetMethodTrampoline(CROP_DISPLAY_CREATE_HW_COMPOSE_SURFACE, Display_CreateHWComposeSurface_Trampoline);
    SetMethodTrampoline(CROP_DISPLAY_REQUEST_MONITOR_LIST, Display_RequestMonitorList_Trampoline);
}

Display::~Display()
{
    CHECK(has_disposed_ && "Display must be disposed before destructing");
}

void Display::Close()
{
    if (!has_disposed_)
    {
        std::list<Shared<Surface>> copySurfaceList(surfaces_list_);
        for (const Shared<Surface>& surface : copySurfaceList)
            surface->Close();
        copySurfaceList.clear();

        this->OnDispose();
        has_disposed_ = true;
        RenderClientObject::Emit(CRSI_DISPLAY_CLOSED, RenderClientEmitterInfo());
    }
}

Shared<Surface> Display::CreateRasterSurface(int32_t width, int32_t height, SkColorType format)
{
    auto s = this->OnCreateSurface(width, height, format, RenderTarget::RenderDevice::kRaster);
    if (s != nullptr)
        surfaces_list_.emplace_back(s);
    return s;
}

Shared<Surface> Display::CreateHWComposeSurface(int32_t width, int32_t height, SkColorType format)
{
    auto s = this->OnCreateSurface(width, height, format, RenderTarget::RenderDevice::kHWComposer);
    if (s != nullptr)
        surfaces_list_.emplace_back(s);
    return s;
}

void Display::RemoveSurfaceFromList(const Shared<Surface>& s)
{
    if (s == nullptr)
        return;
    surfaces_list_.remove(s);
}

void Display::AppendMonitor(const Shared<Monitor>& monitor)
{
    auto itr = std::find(monitors_list_.begin(), monitors_list_.end(), monitor);
    if (itr == monitors_list_.end())
    {
        monitors_list_.push_back(monitor);

        RenderClientEmitterInfo info;
        info.EmplaceBack<Shared<Monitor>>(monitor);
        Emit(CRSI_DISPLAY_MONITOR_ADDED, std::move(info));
    }
}

bool Display::RemoveMonitor(const Shared<Monitor>& monitor)
{
    auto itr = std::find(monitors_list_.begin(), monitors_list_.end(), monitor);
    if (itr != monitors_list_.end())
    {
        monitors_list_.remove(monitor);

        RenderClientEmitterInfo info;
        info.EmplaceBack<Shared<Monitor>>(monitor);
        Emit(CRSI_DISPLAY_MONITOR_REMOVED, std::move(info));

        return true;
    }

    return false;
}

std::list<Shared<Monitor>> Display::RequestMonitorList()
{
    // Return a copy of monitors' list
    return monitors_list_;
}

GLAMOR_NAMESPACE_END
