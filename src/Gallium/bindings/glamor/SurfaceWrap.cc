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

#include "fmt/format.h"

#include "Core/EnumClassBitfield.h"
#include "Gallium/bindings/glamor/Exports.h"
#include "Gallium/bindings/glamor/PromiseHelper.h"
#include "Glamor/PresentRemoteHandle.h"
#include "Glamor/Surface.h"
#include "Glamor/Blender.h"
#include "Glamor/Cursor.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

SurfaceWrap::SurfaceWrap(gl::Shared<gl::PresentRemoteHandle> handle,
                         v8::Local<v8::Object> display)
    : handle_(std::move(handle))
    , display_wrapped_(v8::Isolate::GetCurrent(), display)
{
    DefineSignalEventsOnEventEmitter(this, handle_, {
        { "closed", GLSI_SURFACE_CLOSED },
        { "resize", GLSI_SURFACE_RESIZE,
          GenericSignalArgsConverter<NoCast<int32_t>, NoCast<int32_t>> },
        { "close", GLSI_SURFACE_CLOSE },
        { "configure", GLSI_SURFACE_CONFIGURE,
          GenericSignalArgsConverter<NoCast<int32_t>,
                                     NoCast<int32_t>,
                                     SignalArgsCast<Bitfield<gl::ToplevelStates>, uint32_t>> },
        { "frame", GLSI_SURFACE_FRAME, GenericSignalArgsConverter<NoCast<uint32_t>> },
        { "pointer-hovering", GLSI_SURFACE_POINTER_HOVERING,
          GenericSignalArgsConverter<NoCast<bool>> },
        { "pointer-motion", GLSI_SURFACE_POINTER_MOTION,
          GenericSignalArgsConverter<NoCast<double>, NoCast<double>> },
        { "pointer-button", GLSI_SURFACE_POINTER_BUTTON,
          GenericSignalArgsConverter<AutoEnumCast<gl::PointerButton>, NoCast<bool>> },
        { "pointer-axis", GLSI_SURFACE_POINTER_AXIS,
          GenericSignalArgsConverter<AutoEnumCast<gl::AxisSourceType>, NoCast<double>, NoCast<double>> },
        { "pointer-highres-scroll", GLSI_SURFACE_POINTER_HIGHRES_SCROLL,
          GenericSignalArgsConverter<AutoEnumCast<gl::AxisSourceType>, NoCast<int32_t>, NoCast<int32_t>> },
        { "keyboard-focus", GLSI_SURFACE_KEYBOARD_FOCUS, GenericSignalArgsConverter<NoCast<bool>> },
        { "keyboard-key", GLSI_SURFACE_KEYBOARD_KEY,
          GenericSignalArgsConverter<AutoEnumCast<gl::KeyboardKey>,
                                     EnumBitfieldCast<gl::KeyboardModifiers>,
                                     NoCast<bool>> }
    });

    using InfoT = gl::PresentSignalArgs;
    surface_closed_slot_ = handle_->Connect(
            GLSI_SURFACE_CLOSED, [this](InfoT& info) { display_wrapped_.Reset(); });
}

SurfaceWrap::~SurfaceWrap()
{
    handle_->Disconnect(surface_closed_slot_);
}

int32_t SurfaceWrap::getWidth()
{
    return handle_->Cast<gl::Surface>()->GetWidth();
}

int32_t SurfaceWrap::getHeight()
{
    return handle_->Cast<gl::Surface>()->GetHeight();
}

v8::Local<v8::Value> SurfaceWrap::getDisplay()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (display_wrapped_.IsEmpty())
        return v8::Null(isolate);
    return display_wrapped_.Get(isolate);
}

v8::Local<v8::Value> SurfaceWrap::createBlender()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    using CreateCast = CreateObjCast<gl::Shared<gl::Blender>, BlenderWrap>;
    return PromisifiedRemoteCall::Call(
            isolate, handle_,
            PromisifiedRemoteCall::GenericConvert<CreateCast>,
            GLOP_SURFACE_CREATE_BLENDER);
}

v8::Local<v8::Value> SurfaceWrap::close()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return PromisifiedRemoteCall::Call(isolate, handle_, {}, GLOP_SURFACE_CLOSE);
}

v8::Local<v8::Value> SurfaceWrap::setTitle(const std::string& str)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return PromisifiedRemoteCall::Call(isolate, handle_, {},
                                       GLOP_SURFACE_SET_TITLE, str);
}

v8::Local<v8::Value> SurfaceWrap::resize(int32_t width, int32_t height)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return PromisifiedRemoteCall::Call(isolate, handle_, {},
                                       GLOP_SURFACE_RESIZE, width, height);
}

v8::Local<v8::Value> SurfaceWrap::getBuffersDescriptor()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return PromisifiedRemoteCall::Call(
            isolate, handle_,
            PromisifiedRemoteCall::GenericConvert<NoCast<std::string>>,
            GLOP_SURFACE_GET_BUFFERS_DESCRIPTOR);
}

v8::Local<v8::Value> SurfaceWrap::requestNextFrame()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return PromisifiedRemoteCall::Call(
            isolate, handle_,
            PromisifiedRemoteCall::GenericConvert<NoCast<uint32_t>>,
            GLOP_SURFACE_REQUEST_NEXT_FRAME);
}

v8::Local<v8::Value> SurfaceWrap::setMinSize(int32_t width, int32_t height)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return PromisifiedRemoteCall::Call(
            isolate, handle_, {}, GLOP_SURFACE_SET_MIN_SIZE, width, height);
}

v8::Local<v8::Value> SurfaceWrap::setMaxSize(int32_t width, int32_t height)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return PromisifiedRemoteCall::Call(
            isolate, handle_, {}, GLOP_SURFACE_SET_MAX_SIZE, width, height);
}

v8::Local<v8::Value> SurfaceWrap::setMinimized(bool value)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return PromisifiedRemoteCall::Call(
            isolate, handle_, {}, GLOP_SURFACE_SET_MINIMIZED, value);
}

v8::Local<v8::Value> SurfaceWrap::setMaximized(bool value)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return PromisifiedRemoteCall::Call(
            isolate, handle_, {}, GLOP_SURFACE_SET_MAXIMIZED, value);
}

v8::Local<v8::Value> SurfaceWrap::setFullscreen(bool value, v8::Local<v8::Value> monitor)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    gl::Shared<gl::Monitor> monitor_ptr;

    if (!monitor->IsNullOrUndefined())
    {
        auto *unwrapped = binder::UnwrapObject<MonitorWrap>(isolate, monitor);
        if (!unwrapped)
            g_throw(TypeError, "Argument \'monitor\' must be an instance of Monitor");
    }

    if (value && !monitor_ptr)
        g_throw(Error, "Argument \'monitor\' must be provided when enter fullscreen state");

    return PromisifiedRemoteCall::Call(
            isolate, handle_, {}, GLOP_SURFACE_SET_FULLSCREEN, value, monitor_ptr);
}

v8::Local<v8::Value> SurfaceWrap::setAttachedCursor(v8::Local<v8::Value> cursor)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    auto *wrap = binder::UnwrapObject<CursorWrap>(isolate, cursor);
    if (!wrap)
        g_throw(TypeError, "Argument \'cursor\' must be an instance of Cursor");

    return PromisifiedRemoteCall::Call(
            isolate, handle_, {}, GLOP_SURFACE_SET_ATTACHED_CURSOR,
            wrap->GetCursorHandle());
}

v8::Local<v8::Object> SurfaceWrap::OnGetObjectSelf(v8::Isolate *isolate)
{
    return GetObjectWeakReference().Get(isolate);
}

GALLIUM_BINDINGS_GLAMOR_NS_END
