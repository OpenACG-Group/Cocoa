#include <map>

#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkFontMgr.h"

#include "Core/Journal.h"
#include "Core/Errors.h"
#include "Cobalt/Display.h"
#include "Cobalt/Surface.h"
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

COBALT_TRAMPOLINE_IMPL(Display, CreateRasterSurface)
{
    COBALT_TRAMPOLINE_CHECK_ARGS_NUMBER(3);
    auto this_ = info.GetThis()->As<Display>();
    co_sp<Surface> result = this_->CreateRasterSurface(info.Get<int32_t>(0),
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

COBALT_TRAMPOLINE_IMPL(Display, CreateHWComposeSurface)
{
    COBALT_TRAMPOLINE_CHECK_ARGS_NUMBER(3);
    auto this_ = info.GetThis()->As<Display>();
    co_sp<Surface> result = this_->CreateHWComposeSurface(info.Get<int32_t>(0),
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
    SetMethodTrampoline(CROP_DISPLAY_CREATE_RASTER_SURFACE, Display_CreateRasterSurface_Trampoline);
    SetMethodTrampoline(CROP_DISPLAY_CREATE_HW_COMPOSE_SURFACE, Display_CreateHWComposeSurface_Trampoline);
}

Display::~Display()
{
    CHECK(has_disposed_ && "Display must be disposed before destructing");
}

void Display::Close()
{
    if (!has_disposed_)
    {
        std::list<co_sp<Surface>> copySurfaceList(surfaces_list_);
        for (const co_sp<Surface>& surface : copySurfaceList)
            surface->Close();
        copySurfaceList.clear();

        this->OnDispose();
        has_disposed_ = true;
        RenderClientObject::Emit(CRSI_DISPLAY_CLOSED, RenderClientEmitterInfo());
    }
}

co_sp<Surface> Display::CreateRasterSurface(int32_t width, int32_t height, SkColorType format)
{
    auto s = this->OnCreateSurface(width, height, format, RenderTarget::RenderDevice::kRaster);
    surfaces_list_.emplace_back(s);
    return s;
}

co_sp<Surface> Display::CreateHWComposeSurface(int32_t width, int32_t height, SkColorType format)
{
    auto s = this->OnCreateSurface(width, height, format, RenderTarget::RenderDevice::kHWComposer);
    surfaces_list_.emplace_back(s);
    return s;
}

void Display::RemoveSurfaceFromList(const co_sp<Surface>& s)
{
    surfaces_list_.remove(s);
}

COBALT_NAMESPACE_END
