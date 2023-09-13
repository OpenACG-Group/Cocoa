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

#include "Core/Journal.h"
#include "Glamor/Surface.h"
#include "Glamor/RenderTarget.h"
#include "Glamor/Display.h"
#include "Glamor/Blender.h"
#include "Glamor/Cursor.h"
GLAMOR_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Glamor.Surface)

GLAMOR_TRAMPOLINE_IMPL(Surface, Close)
{
    auto this_ = info.GetThis()->Cast<Surface>();
    this_->Close();
    info.SetReturnStatus(PresentRemoteCall::Status::kOpSuccess);
}

GLAMOR_TRAMPOLINE_IMPL(Surface, Resize)
{
    GLAMOR_TRAMPOLINE_CHECK_ARGS_NUMBER(2);
    auto this_ = info.GetThis()->Cast<Surface>();
    bool result = this_->Resize(info.Get<int32_t>(0), info.Get<int32_t>(1));

    info.SetReturnStatus(result ? PresentRemoteCall::Status::kOpSuccess
                                : PresentRemoteCall::Status::kOpFailed);
}

GLAMOR_TRAMPOLINE_IMPL(Surface, SetTitle)
{
    GLAMOR_TRAMPOLINE_CHECK_ARGS_NUMBER(1);
    auto this_ = info.GetThis()->Cast<Surface>();
    this_->SetTitle(info.Get<std::string>(0));
    info.SetReturnStatus(PresentRemoteCall::Status::kOpSuccess);
}

GLAMOR_TRAMPOLINE_IMPL(Surface, GetBuffersDescriptor)
{
    auto this_ = info.GetThis()->Cast<Surface>();
    info.SetReturnStatus(PresentRemoteCall::Status::kOpSuccess);
    info.SetReturnValue(this_->GetBuffersDescriptor());
}

GLAMOR_TRAMPOLINE_IMPL(Surface, RequestNextFrame)
{
    info.SetReturnStatus(PresentRemoteCall::Status::kOpSuccess);
    info.SetReturnValue(info.GetThis()->Cast<Surface>()->RequestNextFrame());
}

GLAMOR_TRAMPOLINE_IMPL(Surface, SetMinSize)
{
    GLAMOR_TRAMPOLINE_CHECK_ARGS_NUMBER(2);
    info.GetThis()->Cast<Surface>()->SetMinSize(info.Get<int32_t>(0), info.Get<int32_t>(1));
    info.SetReturnStatus(PresentRemoteCall::Status::kOpSuccess);
}

GLAMOR_TRAMPOLINE_IMPL(Surface, SetMaxSize)
{
    GLAMOR_TRAMPOLINE_CHECK_ARGS_NUMBER(2);
    info.GetThis()->Cast<Surface>()->SetMaxSize(info.Get<int32_t>(0), info.Get<int32_t>(1));
    info.SetReturnStatus(PresentRemoteCall::Status::kOpSuccess);
}

GLAMOR_TRAMPOLINE_IMPL(Surface, SetMaximized)
{
    GLAMOR_TRAMPOLINE_CHECK_ARGS_NUMBER(1);
    info.GetThis()->Cast<Surface>()->SetMaximized(info.Get<bool>(0));
    info.SetReturnStatus(PresentRemoteCall::Status::kOpSuccess);
}

GLAMOR_TRAMPOLINE_IMPL(Surface, SetMinimized)
{
    GLAMOR_TRAMPOLINE_CHECK_ARGS_NUMBER(1);
    info.GetThis()->Cast<Surface>()->SetMinimized(info.Get<bool>(0));
    info.SetReturnStatus(PresentRemoteCall::Status::kOpSuccess);
}

GLAMOR_TRAMPOLINE_IMPL(Surface, SetFullscreen)
{
    GLAMOR_TRAMPOLINE_CHECK_ARGS_NUMBER(2);
    info.GetThis()->Cast<Surface>()->SetFullscreen(info.Get<bool>(0), info.Get<Shared<Monitor>>(1));
    info.SetReturnStatus(PresentRemoteCall::Status::kOpSuccess);
}

GLAMOR_TRAMPOLINE_IMPL(Surface, CreateBlender)
{
    GLAMOR_TRAMPOLINE_CHECK_ARGS_NUMBER(0);
    Shared<Blender> blender = info.GetThis()->Cast<Surface>()->CreateBlender();
    info.SetReturnStatus(blender ? PresentRemoteCall::Status::kOpSuccess
                                 : PresentRemoteCall::Status::kOpFailed);
    info.SetReturnValue(blender);
}

GLAMOR_TRAMPOLINE_IMPL(Surface, SetAttachedCursor)
{
    GLAMOR_TRAMPOLINE_CHECK_ARGS_NUMBER(1);
    info.GetThis()->As<Surface>()->SetAttachedCursor(info.Get<Shared<Cursor>>(0));
    info.SetReturnStatus(PresentRemoteCall::Status::kOpSuccess);
}

Surface::Surface(Shared<RenderTarget> rt)
    : PresentRemoteHandle(RealType::kSurface)
    , has_disposed_(false)
    , render_target_(std::move(rt))
    , display_(render_target_->GetDisplay())
{
    CHECK(render_target_ && "Invalid RenderTarget object");

    render_target_->SetFrameNotificationRouter(this);

    SetMethodTrampoline(GLOP_SURFACE_CLOSE, Surface_Close_Trampoline);
    SetMethodTrampoline(GLOP_SURFACE_RESIZE, Surface_Resize_Trampoline);
    SetMethodTrampoline(GLOP_SURFACE_SET_TITLE, Surface_SetTitle_Trampoline);
    SetMethodTrampoline(GLOP_SURFACE_GET_BUFFERS_DESCRIPTOR, Surface_GetBuffersDescriptor_Trampoline);
    SetMethodTrampoline(GLOP_SURFACE_REQUEST_NEXT_FRAME, Surface_RequestNextFrame_Trampoline);
    SetMethodTrampoline(GLOP_SURFACE_SET_MIN_SIZE, Surface_SetMinSize_Trampoline);
    SetMethodTrampoline(GLOP_SURFACE_SET_MAX_SIZE, Surface_SetMaxSize_Trampoline);
    SetMethodTrampoline(GLOP_SURFACE_SET_MAXIMIZED, Surface_SetMaximized_Trampoline);
    SetMethodTrampoline(GLOP_SURFACE_SET_MINIMIZED, Surface_SetMinimized_Trampoline);
    SetMethodTrampoline(GLOP_SURFACE_SET_FULLSCREEN, Surface_SetFullscreen_Trampoline);
    SetMethodTrampoline(GLOP_SURFACE_CREATE_BLENDER, Surface_CreateBlender_Trampoline);
    SetMethodTrampoline(GLOP_SURFACE_SET_ATTACHED_CURSOR, Surface_SetAttachedCursor_Trampoline);
}

Surface::~Surface()
{
    CHECK(has_disposed_ && "Surface should be closed before destructing");
}

int32_t Surface::GetWidth() const
{
    return render_target_->GetWidth();
}

int32_t Surface::GetHeight() const
{
    return render_target_->GetHeight();
}

SkColorType Surface::GetColorType() const
{
    return render_target_->GetColorType();
}

void Surface::Close()
{
    if (!has_disposed_)
    {
        this->OnClose();
        has_disposed_ = true;
        render_target_.reset();
        Emit(GLSI_SURFACE_CLOSED, PresentSignal());
        GetDisplay()->RemoveSurfaceFromList(Self()->Cast<Surface>());
        QLOG(LOG_DEBUG, "Surface has been disposed");
    }
}

bool Surface::Resize(int32_t width, int32_t height)
{
    if (width == render_target_->GetWidth() && height == render_target_->GetHeight())
        return true;

    QLOG(LOG_DEBUG, "Attempting to resize surface to {}x{}", width, height);
    if (width <= 0 || height <= 0)
        return false;

    render_target_->Resize(width, height);

    PresentSignal info;
    info.EmplaceBack<int32_t>(width);
    info.EmplaceBack<int32_t>(height);
    Emit(GLSI_SURFACE_RESIZE, std::move(info));

    return true;
}

void Surface::SetTitle(const std::string_view& title)
{
    QLOG(LOG_DEBUG, "Attempting to set surface title %fg<gr>\"{}\"%reset", title);
    this->OnSetTitle(title);
}

std::string Surface::GetBuffersDescriptor()
{
    return render_target_->GetBufferStateDescriptor();
}

uint32_t Surface::RequestNextFrame()
{
    return render_target_->RequestNextFrame();
}

const SkMatrix& Surface::GetRootTransformation() const
{
    return this->OnGetRootTransformation();
}

const SkMatrix& Surface::OnGetRootTransformation() const
{
    return SkMatrix::I();
}

void Surface::OnFrameNotification(uint32_t sequence)
{
    PresentSignal info;
    info.EmplaceBack<uint32_t>(sequence);
    Emit(GLSI_SURFACE_FRAME, std::move(info));
}

void Surface::SetMaxSize(int32_t width, int32_t height)
{
    this->OnSetMaxSize(width, height);
}

void Surface::SetMinSize(int32_t width, int32_t height)
{
    this->OnSetMinSize(width, height);
}

void Surface::SetMaximized(bool value)
{
    this->OnSetMaximized(value);
}

void Surface::SetMinimized(bool value)
{
    this->OnSetMinimized(value);
}

void Surface::SetFullscreen(bool value, const Shared<Monitor>& monitor)
{
    this->OnSetFullscreen(value, monitor);
}

Shared<Blender> Surface::CreateBlender()
{
    if (!weak_blender_.expired())
    {
        QLOG(LOG_ERROR, "Creating multiple blenders on the same surface is not allowed");
        return nullptr;
    }

    auto blender = Blender::Make(Self()->Cast<Surface>());
    weak_blender_ = blender;

    return blender;
}

void Surface::SetAttachedCursor(const Shared<Cursor>& cursor)
{
    CHECK(cursor);

    if (attached_cursor_)
    {
        // If the cursor is performing animation, it can be aborted
        // to save the CPU time as it is not visible on the surface anymore.
        attached_cursor_->TryAbortAnimation();
    }

    attached_cursor_ = cursor;
    this->OnSetCursor(cursor);

    attached_cursor_->TryStartAnimation();
}

void Surface::Trace(GraphicsResourcesTrackable::Tracer *tracer) noexcept
{
    if (render_target_)
        tracer->TraceMember("RenderTarget", render_target_.get());
    if (!display_.expired())
    {
        tracer->TraceResource("Display",
                              TRACKABLE_TYPE_CLASS_OBJECT,
                              TRACKABLE_DEVICE_CPU,
                              TRACKABLE_OWNERSHIP_WEAK,
                              TraceIdFromPointer(display_.lock().get()));
    }

    if (!weak_blender_.expired())
    {
        tracer->TraceMember("Blender", weak_blender_.lock().get());
    }

    // TODO(sora): trace cursors.
}

GLAMOR_NAMESPACE_END
