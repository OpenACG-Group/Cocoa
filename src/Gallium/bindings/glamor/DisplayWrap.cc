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

#include <utility>

#include "Gallium/bindings/glamor/Exports.h"
#include "Gallium/bindings/glamor/PromiseHelper.h"

#include "Glamor/Display.h"
#include "Glamor/Monitor.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

DisplayWrap::DisplayWrap(std::shared_ptr<gl::PresentRemoteHandle> handle)
    : handle_(std::move(handle))
{
    DefineSignalEventsOnEventEmitter(this, handle_, {
        { "closed", GLSI_DISPLAY_CLOSED },
        {
            "monitor-added",
            GLSI_DISPLAY_MONITOR_ADDED,
            [this](v8::Isolate *i, gl::PresentSignalArgs &info) {
                v8::HandleScope scope(i);
                auto monitor = info.Get<std::shared_ptr<gl::Monitor>>(0);
                v8::Local<v8::Object> result = binder::NewObject<MonitorWrap>(i, monitor);
                this->monitor_objects_map_[monitor].Reset(i, result);
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
                if (LIKELY(this->monitor_objects_map_.count(monitor) > 0)) {
                    result = this->monitor_objects_map_[monitor].Get(i);
                    this->monitor_objects_map_.erase(monitor);
                } else {
                    // If `monitor` has no corresponding JavaScript instance in V8,
                    // we should create one as a temporary object.
                    // It is always safe to retain an instance of `Monitor` after `monitor-removed`
                    // signal is emitted as `Monitor` itself does not keep any GLAMOR resources.
                    result = binder::NewObject<MonitorWrap>(i, monitor);
                }
                return std::vector<v8::Local<v8::Value>>{result};
            }
        }
    });
}

DisplayWrap::~DisplayWrap() = default;

v8::Local<v8::Value> DisplayWrap::close()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Value> promise = PromisifiedRemoteCall::Call(
            isolate, handle_, {}, GLOP_DISPLAY_CLOSE);
    handle_.reset();
    return promise;
}

namespace {

v8::Local<v8::Value>
create_surface_invoke(DisplayWrap *wrap,
                      const std::shared_ptr<gl::PresentRemoteHandle>& display,
                      bool hw_compose,
                      int32_t width, int32_t height)
{
    if (width <= 0 || height <= 0)
        g_throw(RangeError, "Surface width and height must be positive integers");

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto self_sp = std::make_shared<v8::Global<v8::Object>>(
            isolate, wrap->GetObjectWeakReference().Get(isolate));

    // TODO(sora): Select a appropriate color format

    return PromisifiedRemoteCall::Call(
            isolate, display,
            [self_sp](v8::Isolate *i, gl::PresentRemoteCallReturn& info) {
                auto surface = info.GetReturnValue<std::shared_ptr<gl::PresentRemoteHandle>>();
                return binder::NewObject<SurfaceWrap>(i, surface, self_sp->Get(i));
            },
            hw_compose ? GLOP_DISPLAY_CREATE_HW_COMPOSE_SURFACE
                       : GLOP_DISPLAY_CREATE_RASTER_SURFACE,
            width,
            height,
            SkColorType::kBGRA_8888_SkColorType
    );
}

} // namespace anonymous

v8::Local<v8::Value> DisplayWrap::createRasterSurface(int32_t width, int32_t height)
{
    return create_surface_invoke(this, handle_, false, width, height);
}

v8::Local<v8::Value> DisplayWrap::createHWComposeSurface(int32_t width, int32_t height)
{
    return create_surface_invoke(this, handle_, true, width, height);
}

v8::Local<v8::Value> DisplayWrap::requestMonitorList()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    auto self_sp = std::make_shared<v8::Global<v8::Object>>(
            isolate, GetObjectWeakReference().Get(isolate));

    return PromisifiedRemoteCall::Call(
            isolate,
            handle_,
            [self_sp, this](v8::Isolate *i, gl::PresentRemoteCallReturn& info) {
                auto list = info.GetReturnValue<gl::Display::MonitorList>();
                std::vector<v8::Local<v8::Object>> objects;
                for (const auto& monitor : list)
                {
                    CHECK(monitor);
                    v8::Local<v8::Object> monitor_obj;
                    if (this->monitor_objects_map_.count(monitor) == 0)
                    {
                        monitor_obj = binder::NewObject<MonitorWrap>(i, monitor);
                        this->monitor_objects_map_[monitor].Reset(i, monitor_obj);
                    }
                    else
                        monitor_obj = monitor_objects_map_[monitor].Get(i);
                    objects.push_back(monitor_obj);
                }
                return binder::to_v8(i, objects);
            },
            GLOP_DISPLAY_REQUEST_MONITOR_LIST
    );
}

v8::Local<v8::Value> DisplayWrap::loadCursorTheme(const std::string& name, int size)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    using ObjCast = CreateObjCast<std::shared_ptr<gl::CursorTheme>, CursorThemeWrap>;
    return PromisifiedRemoteCall::Call(
            isolate, handle_, PromisifiedRemoteCall::GenericConvert<ObjCast>,
            GLOP_DISPLAY_LOAD_CURSOR_THEME, name, size);
}

v8::Local<v8::Value> DisplayWrap::createCursor(v8::Local<v8::Value> bitmap, int hotspotX, int hotspotY)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    auto *unwrapped = binder::UnwrapObject<CkBitmapWrap>(isolate, bitmap);
    if (!unwrapped)
        g_throw(TypeError, "Argument \'bitmap\' must be an instance of CkBitmap");

    auto bitmap_extracted = std::make_shared<SkBitmap>(unwrapped->getBitmap());
    using ObjCast = CreateObjCast<std::shared_ptr<gl::Cursor>, CursorWrap>;
    return PromisifiedRemoteCall::Call(
            isolate, handle_, PromisifiedRemoteCall::GenericConvert<ObjCast>,
            GLOP_DISPLAY_CREATE_CURSOR,
            bitmap_extracted, hotspotX, hotspotY);
}

v8::Local<v8::Value> DisplayWrap::getDefaultCursorTheme()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!default_cursor_theme_.IsEmpty())
        return default_cursor_theme_.Get(isolate);

    auto theme = handle_->As<gl::Display>()->GetDefaultCursorTheme();
    CHECK(theme);

    auto obj = binder::NewObject<CursorThemeWrap>(isolate, theme);
    default_cursor_theme_.Reset(isolate, obj);

    return obj;
}

v8::Local<v8::Object> DisplayWrap::OnGetObjectSelf(v8::Isolate *isolate)
{
    return GetObjectWeakReference().Get(isolate);
}

namespace {

SignalArgsVector monitor_property_set_transcription(v8::Isolate *isolate,
                                                    gl::PresentSignalArgs& info)
{
    auto props = info.Get<std::shared_ptr<gl::Monitor::PropertySet>>(0);
    std::unordered_map<std::string_view, v8::Local<v8::Value>> fields_map{
        { "logicalX", binder::to_v8(isolate, props->logical_position.x()) },
        { "logicalY", binder::to_v8(isolate, props->logical_position.y()) },
        { "subpixel", binder::to_v8(isolate, static_cast<uint32_t>(props->subpixel)) },
        { "manufactureName", binder::to_v8(isolate, props->manufacture_name) },
        { "modelName", binder::to_v8(isolate, props->model_name) },
        { "transform", binder::to_v8(isolate, static_cast<uint32_t>(props->transform)) },
        { "modeFlags", binder::to_v8(isolate, props->mode_flags.value()) },
        { "modeWidth", binder::to_v8(isolate, props->mode_size.x()) },
        { "modeHeight", binder::to_v8(isolate, props->mode_size.y()) },
        { "refreshRate", binder::to_v8(isolate, props->refresh_rate_mhz) },
        { "scaleFactor", binder::to_v8(isolate, props->scale_factor) },
        { "connectorName", binder::to_v8(isolate, props->connector_name) },
        { "description", binder::to_v8(isolate, props->description) }
    };

    return std::vector<v8::Local<v8::Value>>{binder::to_v8(isolate, fields_map)};
}

} // namespace anonymous

MonitorWrap::MonitorWrap(std::shared_ptr<gl::PresentRemoteHandle> handle)
    : handle_(std::move(handle))
{
    DefineSignalEventsOnEventEmitter(this, handle_, {
        { "properties-changed",
          GLSI_MONITOR_PROPERTIES_CHANGED, monitor_property_set_transcription },
        { "detached", GLSI_MONITOR_DETACHED }
    });
}

v8::Local<v8::Value> MonitorWrap::requestPropertySet()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return PromisifiedRemoteCall::Call(
            isolate, handle_, {}, GLOP_MONITOR_REQUEST_PROPERTIES);
}

v8::Local<v8::Object> MonitorWrap::OnGetObjectSelf(v8::Isolate *isolate)
{
    return GetObjectWeakReference().Get(isolate);
}

GALLIUM_BINDINGS_GLAMOR_NS_END
