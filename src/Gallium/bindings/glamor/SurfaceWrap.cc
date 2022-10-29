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
#include "Glamor/RenderClientObject.h"
#include "Glamor/Surface.h"
#include "Glamor/Blender.h"
#include "Glamor/Cursor.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

SurfaceWrap::SurfaceWrap(gl::Shared<gl::RenderClientObject> object)
    : RenderClientObjectWrap(std::move(object))
{
    defineSignal("closed", GLSI_SURFACE_CLOSED, nullptr);
    defineSignal("resize", GLSI_SURFACE_RESIZE, GenericInfoAcceptor<NoCast<int32_t>, NoCast<int32_t>>);
    defineSignal("close", GLSI_SURFACE_CLOSE, nullptr);
    defineSignal("configure", GLSI_SURFACE_CONFIGURE,
                 GenericInfoAcceptor<NoCast<int32_t>, NoCast<int32_t>,
                         InfoAcceptorCast<Bitfield<gl::ToplevelStates>, uint32_t>>);
    defineSignal("frame", GLSI_SURFACE_FRAME, GenericInfoAcceptor<NoCast<uint32_t>>);
    defineSignal("pointer-hovering", GLSI_SURFACE_POINTER_HOVERING, GenericInfoAcceptor<NoCast<bool>>);
    defineSignal("pointer-motion", GLSI_SURFACE_POINTER_MOTION,
                 GenericInfoAcceptor<NoCast<double>, NoCast<double>>);
    defineSignal("pointer-button", GLSI_SURFACE_POINTER_BUTTON,
                 GenericInfoAcceptor<AutoEnumCast<gl::PointerButton>, NoCast<bool>>);
}

SurfaceWrap::~SurfaceWrap() = default;

int32_t SurfaceWrap::getWidth()
{
    return getObject()->Cast<gl::Surface>()->GetWidth();
}

int32_t SurfaceWrap::getHeight()
{
    return getObject()->Cast<gl::Surface>()->GetHeight();
}

v8::Local<v8::Value> SurfaceWrap::createBlender()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    using W = BlenderWrap;
    using T = gl::Shared<gl::Blender>;
    auto closure = PromiseClosure::New(isolate, PromiseClosure::CreateObjectConverter<W, T>);
    getObject()->Invoke(GLOP_SURFACE_CREATE_BLENDER, closure, PromiseClosure::HostCallback);
    return closure->getPromise();
}

v8::Local<v8::Value> SurfaceWrap::close()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto closure = PromiseClosure::New(isolate, nullptr);
    getObject()->Invoke(GLOP_SURFACE_CLOSE, closure, PromiseClosure::HostCallback);
    return closure->getPromise();
}

v8::Local<v8::Value> SurfaceWrap::setTitle(const std::string& str)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto closure = PromiseClosure::New(isolate, nullptr);
    getObject()->Invoke(GLOP_SURFACE_SET_TITLE, closure, PromiseClosure::HostCallback, str);
    return closure->getPromise();
}

v8::Local<v8::Value> SurfaceWrap::resize(int32_t width, int32_t height)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    using LV = v8::Local<v8::Value>;
    auto closure = PromiseClosure::New(isolate,
        [](v8::Isolate *isolate, gl::RenderHostCallbackInfo& info) -> LV {
            return v8::Boolean::New(isolate, info.GetReturnValue<bool>());
    });

    getObject()->Invoke(GLOP_SURFACE_RESIZE, closure, PromiseClosure::HostCallback,
                        width, height);
    return closure->getPromise();
}

v8::Local<v8::Value> SurfaceWrap::getBuffersDescriptor()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    using LV = v8::Local<v8::Value>;
    auto closure = PromiseClosure::New(isolate,
        [](v8::Isolate *isolate, gl::RenderHostCallbackInfo& info) -> LV {
        return binder::to_v8(isolate, info.GetReturnValue<std::string>());
    });

    getObject()->Invoke(GLOP_SURFACE_GET_BUFFERS_DESCRIPTOR,
                        closure, PromiseClosure::HostCallback);

    return closure->getPromise();
}

v8::Local<v8::Value> SurfaceWrap::requestNextFrame()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    using LV = v8::Local<v8::Value>;
    auto closure = PromiseClosure::New(isolate,
        [](v8::Isolate *isolate, gl::RenderHostCallbackInfo& info) -> LV {
            return binder::to_v8(isolate, info.GetReturnValue<uint32_t>());
    });

    getObject()->Invoke(GLOP_SURFACE_REQUEST_NEXT_FRAME, closure, PromiseClosure::HostCallback);

    return closure->getPromise();
}

#define IMPL_SET_XXX_SIZE(op, w, h)                                         \
    v8::Isolate *isolate = v8::Isolate::GetCurrent();                       \
    auto closure = PromiseClosure::New(isolate, nullptr);                   \
    getObject()->Invoke(op, closure, PromiseClosure::HostCallback, w, h);   \
    return closure->getPromise();

v8::Local<v8::Value> SurfaceWrap::setMinSize(int32_t width, int32_t height)
{
    IMPL_SET_XXX_SIZE(GLOP_SURFACE_SET_MIN_SIZE, width, height)
}

v8::Local<v8::Value> SurfaceWrap::setMaxSize(int32_t width, int32_t height)
{
    IMPL_SET_XXX_SIZE(GLOP_SURFACE_SET_MAX_SIZE, width, height)
}

#define IMPL_SET_BOOLEAN_VALUE(op, value)                                   \
    v8::Isolate *isolate = v8::Isolate::GetCurrent();                       \
    auto closure = PromiseClosure::New(isolate, nullptr);                   \
    getObject()->Invoke(op, closure, PromiseClosure::HostCallback, value);  \
    return closure->getPromise();

v8::Local<v8::Value> SurfaceWrap::setMinimized(bool value)
{
    IMPL_SET_BOOLEAN_VALUE(GLOP_SURFACE_SET_MINIMIZED, value);
}

v8::Local<v8::Value> SurfaceWrap::setMaximized(bool value)
{
    IMPL_SET_BOOLEAN_VALUE(GLOP_SURFACE_SET_MAXIMIZED, value);
}

v8::Local<v8::Value> SurfaceWrap::setFullscreen(bool value, v8::Local<v8::Value> monitor)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    gl::Shared<gl::Monitor> monitor_ptr;

    if (!monitor->IsNullOrUndefined())
    {
        MonitorWrap *unwrapped = binder::Class<MonitorWrap>::unwrap_object(isolate, monitor);
        if (!unwrapped)
            g_throw(TypeError, "Argument \'monitor\' must be an instance of Monitor");
    }

    if (value && !monitor_ptr)
        g_throw(Error, "Argument \'monitor\' must be provided when enter fullscreen state");

    auto closure = PromiseClosure::New(isolate, nullptr);
    getObject()->Invoke(GLOP_SURFACE_SET_FULLSCREEN, closure, PromiseClosure::HostCallback,
                        value, monitor_ptr);

    return closure->getPromise();
}

v8::Local<v8::Value> SurfaceWrap::setAttachedCursor(v8::Local<v8::Value> cursor)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    CursorWrap *unwrapped = binder::Class<CursorWrap>::unwrap_object(isolate, cursor);
    if (!unwrapped)
        g_throw(TypeError, "Argument \'cursor\' must be an instance of Cursor");

    gl::Shared<gl::Cursor> extracted_cursor = unwrapped->getObject()->As<gl::Cursor>();
    CHECK(extracted_cursor);

    auto closure = PromiseClosure::New(isolate, nullptr);
    getObject()->Invoke(GLOP_SURFACE_SET_ATTACHED_CURSOR, closure,
                        PromiseClosure::HostCallback, extracted_cursor);

    return closure->getPromise();
}

GALLIUM_BINDINGS_GLAMOR_NS_END
