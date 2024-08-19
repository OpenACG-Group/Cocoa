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

#include "Glamor/HWComposeSwapchain.h"
#include "Gallium/bindings/present/Monitor.h"
#include "Gallium/bindings/present/Cursor.h"
#include "Gallium/bindings/present/Surface.h"
#include "Gallium/bindings/present/ContentAggregator.h"
#include "Gallium/bindings/present/Promisify.h"
#include "Gallium/bindings/multimedia/HWDeviceContext.h"
GALLIUM_BINDINGS_PRESENT_NS_BEGIN

Surface::Surface(v8::Global<v8::Object> display, std::shared_ptr<gl::Surface> surface)
    : dimensions_(), display_(std::move(display)), surface_(std::move(surface))
{
    dimensions_ = { surface_->GetWidth(), surface_->GetHeight() };

    DefineSignalEventsOnEventEmitter(this, surface_, {
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
}

ffi::RetLocal<v8::Value> Surface::close()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    NotifyDisposeState(DisposeState::kDisposed);
    if (!content_aggregator_.IsEmpty())
    {
        ffi::JSObject::Unwrap<ContentAggregator>(
                isolate, content_aggregator_.Get(isolate))->NotifyParentSurfaceDispose();
    }

    return PromisifiedRemoteCall::Call(isolate, surface_, {}, GLOP_SURFACE_CLOSE);
}

ffi::RetLocal<v8::Value> Surface::getContentAggregator()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!content_aggregator_.IsEmpty())
        return content_aggregator_.Get(isolate);

    v8::Global<v8::Object> self(isolate, GetThisHandle(isolate));
    v8::Local<v8::Object> object = ffi::JSObject::New<ContentAggregator>(
            isolate, std::move(self), surface_->GetContentAggregator());
    content_aggregator_.Reset(isolate, object);
    return object;
}

ffi::RetLocal<v8::Value> Surface::setTitle(const std::string& str)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return PromisifiedRemoteCall::Call(isolate, surface_, {}, GLOP_SURFACE_SET_TITLE, str);
}

ffi::RetLocal<v8::Value> Surface::resize(int32_t width, int32_t height)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return PromisifiedRemoteCall::Call(isolate, surface_, {}, GLOP_SURFACE_RESIZE, width, height);
}

ffi::RetLocal<v8::Value> Surface::requestBufferStateInfo()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return PromisifiedRemoteCall::Call(
        isolate, surface_, PromisifiedRemoteCall::GenericConvert<NoCast<std::string>>,
        GLOP_SURFACE_GET_BUFFERS_DESCRIPTOR
    );
}

ffi::RetLocal<v8::Value> Surface::requestNextFrame()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return PromisifiedRemoteCall::Call(
        isolate, surface_, PromisifiedRemoteCall::GenericConvert<NoCast<uint32_t>>,
        GLOP_SURFACE_REQUEST_NEXT_FRAME
    );
}

ffi::RetLocal<v8::Value> Surface::setMinSize(int32_t width, int32_t height)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return PromisifiedRemoteCall::Call(isolate, surface_, {}, GLOP_SURFACE_SET_MIN_SIZE, width, height);
}

ffi::RetLocal<v8::Value> Surface::setMaxSize(int32_t width, int32_t height)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return PromisifiedRemoteCall::Call(isolate, surface_, {}, GLOP_SURFACE_SET_MAX_SIZE, width, height);
}

ffi::RetLocal<v8::Value> Surface::setMinimized(bool value)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return PromisifiedRemoteCall::Call(isolate, surface_, {}, GLOP_SURFACE_SET_MINIMIZED, value);
}

ffi::RetLocal<v8::Value> Surface::setMaximized(bool value)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return PromisifiedRemoteCall::Call(isolate, surface_, {}, GLOP_SURFACE_SET_MAXIMIZED, value);
}

ffi::RetLocal<v8::Value> Surface::setFullscreen(bool value, const ffi::Opt<ffi::Class<Monitor>>& monitor)
{
    if (value && !monitor)
        return ffi::Fail(ffi::kErr, "require a Monitor when entering fullscreen state");
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return PromisifiedRemoteCall::Call(
            isolate, surface_, {}, GLOP_SURFACE_SET_FULLSCREEN, (*monitor)->GetGLMonitor());
}

ffi::RetLocal<v8::Value> Surface::setAttachedCursor(const ffi::Class<Cursor>& cursor)
{
    if (cursor->GetDisposeState() != DisposeState::kNot)
        return ffi::Fail(ffi::kErr, "Cursor has been disposed");
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return PromisifiedRemoteCall::Call(
            isolate, surface_, {}, GLOP_SURFACE_SET_ATTACHED_CURSOR, cursor->GetGLCursor());
}

ffi::RetLocal<v8::Value> Surface::getVideoDecodeCompatibleDevice()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!videodec_compatible_device_.IsEmpty())
        return videodec_compatible_device_.Get(isolate);

    v8::Local<v8::Value> value = v8::Null(isolate);
    std::shared_ptr<gl::RenderTarget> target = surface_->GetRenderTarget();
    if (auto swapchain = target->GetHWComposeSwapchain())
    {
        auto hwctx = swapchain->GetVideoDecodeHWContext();
        if (hwctx)
            value = ffi::JSObject::New<multimedia::HWDeviceContext>(isolate, hwctx);
    }
    videodec_compatible_device_.Reset(isolate, value);
    return value;
}

GALLIUM_BINDINGS_PRESENT_NS_END
