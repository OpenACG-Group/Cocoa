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

#include "Gallium/bindings/present/Display.h"
#include "Gallium/bindings/present/Monitor.h"
#include "Gallium/bindings/present/Cursor.h"
#include "Gallium/bindings/present/Surface.h"
#include "Gallium/bindings/present/Promisify.h"
GALLIUM_BINDINGS_PRESENT_NS_BEGIN

Display::Display(std::shared_ptr<gl::Display> display)
    : display_(std::move(display))
{
    DefineSignalEventsOnEventEmitter(this, display_, {
        { "closed", GLSI_DISPLAY_CLOSED },
        {
            "monitor-added",
            GLSI_DISPLAY_MONITOR_ADDED,
            [this](v8::Isolate *i, gl::PresentSignalArgs &info) {
                v8::HandleScope scope(i);
                auto monitor = info.Get<std::shared_ptr<gl::Monitor>>(0);
                v8::Local<v8::Object> result = ffi::JSObject::New<Monitor>(i, monitor);
                this->monitor_map_[monitor].Reset(i, result);
                return std::vector<v8::Local<v8::Value>>{result};
            }
        },
        {
            "monitor-removed",
            GLSI_DISPLAY_MONITOR_REMOVED,
            [this](v8::Isolate *i, gl::PresentSignalArgs &info) {
                v8::HandleScope scope(i);
                auto monitor = info.Get<std::shared_ptr<gl::Monitor>>(0);
                v8::Local<v8::Object> result;
                if (LIKELY(this->monitor_map_.count(monitor) > 0)) {
                    result = this->monitor_map_[monitor].Get(i);
                    this->monitor_map_.erase(monitor);
                } else {
                    // If `monitor` has no corresponding JavaScript instance in V8,
                    // we should create one as a temporary object.
                    // It is always safe to retain an instance of `Monitor` after `monitor-removed`
                    // signal is emitted as `Monitor` itself does not keep any GLAMOR resources.
                    result = ffi::JSObject::New<Monitor>(i, monitor);
                }
                return std::vector<v8::Local<v8::Value>>{result};
            }
        }
    });
}

ffi::RetLocal<v8::Value> Display::close()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto promise = PromisifiedRemoteCall::Call(isolate, display_, {}, GLOP_DISPLAY_CLOSE);
    display_.reset();
    NotifyDisposeState(DisposeState::kDisposed);
    return promise;
}

ffi::RetLocal<v8::Value> Display::requestMonitorList()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    auto self_sp = std::make_shared<v8::Global<v8::Object>>(isolate, GetThisHandle(isolate));
    return PromisifiedRemoteCall::Call(
        isolate,
        display_,
        [self_sp, this](v8::Isolate *i, gl::PresentRemoteCallReturn& info) {
            auto list = info.GetReturnValue<gl::Display::MonitorList>();
            std::vector<v8::Local<v8::Value>> objects;
            for (const auto& monitor : list)
            {
                CHECK(monitor);
                v8::Local<v8::Object> monitor_obj;
                if (this->monitor_map_.count(monitor) == 0)
                {
                    monitor_obj = ffi::JSObject::New<Monitor>(i, monitor);
                    this->monitor_map_[monitor].Reset(i, monitor_obj);
                }
                else
                    monitor_obj = monitor_map_[monitor].Get(i);
                objects.emplace_back(monitor_obj);
            }

            return v8::Array::New(i, objects.data(), objects.size());
        },
        GLOP_DISPLAY_REQUEST_MONITOR_LIST
    );
}

ffi::RetLocal<v8::Value> Display::getDefaultCursorTheme()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!default_cursor_theme_.IsEmpty())
        return default_cursor_theme_.Get(isolate);

    std::shared_ptr<gl::CursorTheme> theme = display_->GetDefaultCursorTheme();
    CHECK(theme);
    v8::Local<v8::Object> object = ffi::JSObject::New<CursorTheme>(isolate, theme);
    default_cursor_theme_.Reset(isolate, object);
    return object;
}

ffi::RetLocal<v8::Value> Display::loadCursorTheme(const std::string& name, int32_t size)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return PromisifiedRemoteCall::Call(
        isolate, display_,
        PromisifiedRemoteCall::GenericConvert<CreateObjCast<std::shared_ptr<gl::CursorTheme>, CursorTheme>>,
        GLOP_DISPLAY_LOAD_CURSOR_THEME, name, size
    );
}

ffi::RetLocal<v8::Value> Display::createCursor(renderer::PixmapAdapter pixmap,
                                               int32_t hotspot_x, int32_t hotspot_y)
{
    SkPixmap& sk_pixmap = *pixmap;
    std::shared_ptr<SkBitmap> bitmap = std::make_shared<SkBitmap>();

    // Allocate and copy pixels
    bitmap->allocPixels(sk_pixmap.info(), sk_pixmap.rowBytes());
    if (!sk_pixmap.readPixels(bitmap->pixmap(), 0, 0))
        return ffi::Fail(ffi::kErr, "failed to read pixels from given pixmap");

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return PromisifiedRemoteCall::Call(
        isolate, display_,
        PromisifiedRemoteCall::GenericConvert<CreateObjCast<std::shared_ptr<gl::Cursor>, Cursor>>,
        GLOP_DISPLAY_CREATE_CURSOR, bitmap, hotspot_x, hotspot_y
    );
}

ffi::RetLocal<v8::Value> Display::createSurface(int32_t width, int32_t height,
                                                ffi::IFace<SurfaceCreationOptions> options)
{
    if (width <= 0 || height <= 0 || width >= 0xffff || height >= 0xffff)
        return ffi::Fail(ffi::kRangeErr, "invalid width or height to create surface");

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto self_sp = std::make_shared<v8::Global<v8::Object>>(isolate, GetThisHandle(isolate));

    auto result_cvt = [self_sp](v8::Isolate *i, gl::PresentRemoteCallReturn& info) {
        auto surface = info.GetReturnValue<std::shared_ptr<gl::Surface>>();
        return ffi::JSObject::New<Surface>(i, std::move(*self_sp), surface);
    };

    if (!options->enable_gpu_pipeline.has_value())
        options->enable_gpu_pipeline = false;
    if (!options->enable_gpu_video_decode_compatible.has_value())
        options->enable_gpu_video_decode_compatible = false;

    if (*options->enable_gpu_pipeline)
    {
        gl::PresentGpuContextOptions ctx_options{
            .video_decode_compatible = *options->enable_gpu_video_decode_compatible
        };
        return PromisifiedRemoteCall::Call(
            isolate, display_, result_cvt,
            GLOP_DISPLAY_CREATE_HW_COMPOSE_SURFACE,
            width, height, ctx_options
        );
    }

    return PromisifiedRemoteCall::Call(
        isolate, display_, result_cvt,
        GLOP_DISPLAY_CREATE_RASTER_SURFACE, width, height
    );
}

GALLIUM_BINDINGS_PRESENT_NS_END
