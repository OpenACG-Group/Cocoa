/**
 * This file is part of Cocoa.
 *
 * Cocoa is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * Cocoa is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cocoa. If not, see <https://www.gnu.org/licenses/>.
 */

#include <map>

#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkFontMgr.h"

#include "Core/Journal.h"
#include "Core/Errors.h"
#include "Glamor/RenderClient.h"
#include "Glamor/Cursor.h"
#include "Glamor/CursorTheme.h"
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

GLAMOR_TRAMPOLINE_IMPL(Display, CreateCursor)
{
    GLAMOR_TRAMPOLINE_CHECK_ARGS_NUMBER(3);
    auto _this = info.GetThis()->As<Display>();
    auto cursor = _this->CreateCursor(info.Get<Shared<SkBitmap>>(0),
                                      info.Get<int32_t>(1), info.Get<int32_t>(2));
    info.SetReturnStatus(cursor ? RenderClientCallInfo::Status::kOpSuccess
                                : RenderClientCallInfo::Status::kOpFailed);
    info.SetReturnValue(cursor);
}

GLAMOR_TRAMPOLINE_IMPL(Display, LoadCursorTheme)
{
    GLAMOR_TRAMPOLINE_CHECK_ARGS_NUMBER(2);
    auto _this = info.GetThis()->As<Display>();
    auto theme = _this->LoadCursorTheme(info.Get<std::string>(0), info.Get<int>(1));
    info.SetReturnStatus(theme ? RenderClientCallInfo::Status::kOpSuccess
                               : RenderClientCallInfo::Status::kOpFailed);
    info.SetReturnValue(theme);
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
    SetMethodTrampoline(GLOP_DISPLAY_CLOSE, Display_Close_Trampoline);
    SetMethodTrampoline(GLOP_DISPLAY_CREATE_RASTER_SURFACE, Display_CreateRasterSurface_Trampoline);
    SetMethodTrampoline(GLOP_DISPLAY_CREATE_HW_COMPOSE_SURFACE, Display_CreateHWComposeSurface_Trampoline);
    SetMethodTrampoline(GLOP_DISPLAY_REQUEST_MONITOR_LIST, Display_RequestMonitorList_Trampoline);
    SetMethodTrampoline(GLOP_DISPLAY_CREATE_CURSOR, Display_CreateCursor_Trampoline);
    SetMethodTrampoline(GLOP_DISPLAY_LOAD_CURSOR_THEME, Display_LoadCursorTheme_Trampoline);

    GlobalScope::Ref().GetRenderClient()->RegisterDisplay(this);
}

Display::~Display()
{
    CHECK(has_disposed_ && "Display must be disposed before destructing");
}

void Display::Close()
{
    if (!has_disposed_)
    {
        // Some surfaces may retain references to cursor objects,
        // so we destruct surfaces first.

        // Surfaces will remove themselves from display's `surfaces_list_`
        // when `Surface::Close` method is called. That makes trouble to
        // our iteration here, so we copy a new list and do iterations
        // on it.
        std::list<Shared<Surface>> copySurfaceList(surfaces_list_);
        for (const Shared<Surface>& surface : copySurfaceList)
        {
            // Surfaces are supposed to give up all the resources it has retained
            // after being closed.
            surface->Close();
        }
        copySurfaceList.clear();

        // Destruct cursors and cursor themes.
        for (const Shared<CursorTheme>& theme : cursor_themes_list_)
        {
            theme->Dispose();
        }
        cursor_themes_list_.clear();

        for (const Shared<Cursor>& cursor : created_cursors_list_)
        {
            cursor->Dispose();
        }
        created_cursors_list_.clear();

        // Implementation can release platform-specific resources now.
        this->OnDispose();

        has_disposed_ = true;
        RenderClientObject::Emit(GLSI_DISPLAY_CLOSED, RenderClientEmitterInfo());

        GlobalScope::Ref().GetRenderClient()->UnregisterDisplay(this);
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
        Emit(GLSI_DISPLAY_MONITOR_ADDED, std::move(info));
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
        Emit(GLSI_DISPLAY_MONITOR_REMOVED, std::move(info));

        return true;
    }

    return false;
}

std::list<Shared<Monitor>> Display::RequestMonitorList()
{
    // Return a copy of monitors' list
    return monitors_list_;
}

Shared<Cursor> Display::CreateCursor(const Shared<SkBitmap>& bitmap,
                                     int32_t hotspot_x, int32_t hotspot_y)
{
    auto cursor = this->OnCreateCursor(bitmap, hotspot_x, hotspot_y);
    if (cursor)
        created_cursors_list_.push_back(cursor);
    return cursor;
}

Shared<CursorTheme> Display::LoadCursorTheme(const std::string& name, int size)
{
    auto theme = this->OnLoadCursorTheme(name, size);
    if (theme)
        cursor_themes_list_.push_back(theme);
    return theme;
}

Shared<CursorTheme> Display::OnLoadCursorTheme(g_maybe_unused const std::string& name,
                                               g_maybe_unused int size)
{
    return nullptr;
}

Shared<Cursor> Display::OnCreateCursor(g_maybe_unused const Shared<SkBitmap>& bitmap,
                                       g_maybe_unused int32_t hotspot_x,
                                       g_maybe_unused int32_t hotspot_y)
{
    return nullptr;
}

void Display::AppendDefaultCursorTheme(const Shared<CursorTheme>& theme)
{
    CHECK(cursor_themes_list_.empty() &&
          "Default theme only can be appended into empty list");
    cursor_themes_list_.push_back(theme);
}

void Display::Trace(GraphicsResourcesTrackable::Tracer *tracer) noexcept
{
    int32_t idx = 0;
    for (const auto& monitor : monitors_list_)
    {
        tracer->TraceResource(fmt::format("Monitor#{}", idx++),
                              TRACKABLE_TYPE_CLASS_OBJECT,
                              TRACKABLE_DEVICE_CPU,
                              TRACKABLE_OWNERSHIP_SHARED,
                              TraceIdFromPointer(monitor.get()));
    }

    idx = 0;
    for (const auto& surface : surfaces_list_)
    {
        tracer->TraceMember(fmt::format("Surface#{}", idx++), surface.get());
    }

    // TODO(sora): trace cursors and cursor themes.
}

GLAMOR_NAMESPACE_END
