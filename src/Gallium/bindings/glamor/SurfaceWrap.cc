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

SkRect CkRectToSkRectCast(v8::Isolate *isolate, v8::Local<v8::Value> object)
{
    if (!object->IsObject())
        g_throw(TypeError, "CkRect is not an object");

    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();

    v8::Local<v8::Object> r = object.As<v8::Object>();

    float ltrb[4] = {0, 0, 0, 0};
    float *pw = &ltrb[0];

    for (const char *prop : {"left", "top", "right", "bottom"})
    {
        auto jsName = binder::to_v8(isolate, prop);
        if (!r->HasOwnProperty(ctx, jsName).FromMaybe(false))
            g_throw(TypeError, fmt::format("CkRect does not contain property '{}'", prop));
        *(pw++) = binder::from_v8<float>(isolate, r->Get(ctx, jsName).ToLocalChecked());
    }

    return SkRect::MakeLTRB(ltrb[0], ltrb[1], ltrb[2], ltrb[3]);
}

SkIRect CkRectToSkIRectCast(v8::Isolate *isolate, v8::Local<v8::Value> object)
{
    SkRect rect = CkRectToSkRectCast(isolate, object);
    return SkIRect::MakeLTRB(static_cast<int32_t>(rect.left()),
                             static_cast<int32_t>(rect.top()),
                             static_cast<int32_t>(rect.right()),
                             static_cast<int32_t>(rect.bottom()));
}

SurfaceWrap::SurfaceWrap(glamor::Shared<glamor::RenderClientObject> object)
    : RenderClientObjectWrap(std::move(object))
{
    defineSignal("closed", GLSI_SURFACE_CLOSED, nullptr);
    defineSignal("resize", GLSI_SURFACE_RESIZE,
                 [](v8::Isolate *isolate, i::RenderHostSlotCallbackInfo& info) -> InfoAcceptorResult {
        std::vector<v8::Local<v8::Value>> ret{
            binder::to_v8(isolate, info.Get<int32_t>(0)),
            binder::to_v8(isolate, info.Get<int32_t>(1))
        };
        return std::move(ret);
    });
    defineSignal("close", GLSI_SURFACE_CLOSE, nullptr);
    defineSignal("configure", GLSI_SURFACE_CONFIGURE,
                 [](v8::Isolate *isolate, i::RenderHostSlotCallbackInfo& info) -> InfoAcceptorResult {
        std::vector<v8::Local<v8::Value>> ret{
            binder::to_v8(isolate, info.Get<int32_t>(0)),
            binder::to_v8(isolate, info.Get<int32_t>(1)),
            binder::to_v8(isolate, info.Get<Bitfield<i::ToplevelStates>>(2).value())
        };
        return std::move(ret);
    });
    defineSignal("frame", GLSI_SURFACE_FRAME,
                 [](v8::Isolate *isolate, i::RenderHostSlotCallbackInfo& info) -> InfoAcceptorResult {
        std::vector<v8::Local<v8::Value>> ret{
            binder::to_v8(isolate, info.Get<uint32_t>(0))
        };
        return std::move(ret);
    });
    defineSignal("hovered", GLSI_SURFACE_HOVERED,
                 [](v8::Isolate *isolate, i::RenderHostSlotCallbackInfo& info) -> InfoAcceptorResult {
        std::vector<v8::Local<v8::Value>> ret{
            binder::to_v8(isolate, info.Get<bool>(0))
        };
        return std::move(ret);
    });
}

SurfaceWrap::~SurfaceWrap() = default;

int32_t SurfaceWrap::getWidth()
{
    return getObject()->Cast<i::Surface>()->GetWidth();
}

int32_t SurfaceWrap::getHeight()
{
    return getObject()->Cast<i::Surface>()->GetHeight();
}

v8::Local<v8::Value> SurfaceWrap::createBlender()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    using W = BlenderWrap;
    using T = glamor::Shared<glamor::Blender>;
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
        [](v8::Isolate *isolate, i::RenderHostCallbackInfo& info) -> LV {
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
        [](v8::Isolate *isolate, i::RenderHostCallbackInfo& info) -> LV {
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
        [](v8::Isolate *isolate, i::RenderHostCallbackInfo& info) -> LV {
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

    glamor::Shared<glamor::Monitor> monitor_ptr;

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

    glamor::Shared<glamor::Cursor> extracted_cursor = unwrapped->getObject()->As<glamor::Cursor>();
    CHECK(extracted_cursor);

    auto closure = PromiseClosure::New(isolate, nullptr);
    getObject()->Invoke(GLOP_SURFACE_SET_ATTACHED_CURSOR, closure,
                        PromiseClosure::HostCallback, extracted_cursor);

    return closure->getPromise();
}

GALLIUM_BINDINGS_GLAMOR_NS_END
