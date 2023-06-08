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

#include "Gallium/bindings/glamor/Exports.h"
#include "Gallium/bindings/glamor/PromiseHelper.h"

#include "Glamor/Display.h"
#include "Glamor/Monitor.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

DisplayWrap::DisplayWrap(gl::Shared<gl::RenderClientObject> object)
    : RenderClientObjectWrap(std::move(object))
    , PreventGCObject(v8::Isolate::GetCurrent())
{
    defineSignal("closed", GLSI_DISPLAY_CLOSED, nullptr);
    defineSignal("monitor-added", GLSI_DISPLAY_MONITOR_ADDED,
                 [this](v8::Isolate *i, gl::RenderHostSlotCallbackInfo& info) -> InfoAcceptorResult {
        v8::HandleScope scope(i);
        auto monitor = info.Get<gl::Shared<gl::Monitor>>(0);

        v8::Local<v8::Object> result = binder::NewObject<MonitorWrap>(i, monitor);
        this->monitor_objects_map_[monitor].Reset(i, result);

        return {std::vector<v8::Local<v8::Value>>{result}};
    });
    defineSignal("monitor-removed", GLSI_DISPLAY_MONITOR_REMOVED,
                 [this](v8::Isolate *i, gl::RenderHostSlotCallbackInfo& info) -> InfoAcceptorResult {
        v8::HandleScope scope(i);
        auto monitor = info.Get<gl::Shared<gl::Monitor>>(0);

        v8::Local<v8::Object> result;
        if (LIKELY(this->monitor_objects_map_.count(monitor) > 0))
        {
            result = this->monitor_objects_map_[monitor].Get(i);
            this->monitor_objects_map_.erase(monitor);
        }
        else
        {
            // If `monitor` has no corresponding JavaScript instance in V8,
            // we should create one as a temporary object.
            // It is always safe to retain an instance of `Monitor` after `monitor-removed`
            // signal is emitted as `Monitor` itself does not keep any GLAMOR resources.
            result = binder::NewObject<MonitorWrap>(i, monitor);
        }

        return {std::vector<v8::Local<v8::Value>>{result}};
    });
}

DisplayWrap::~DisplayWrap() = default;

v8::Local<v8::Value> DisplayWrap::close()
{
    markCanBeGarbageCollected();

    auto closure = PromiseClosure::New(v8::Isolate::GetCurrent(), nullptr);
    getObject()->Invoke(GLOP_DISPLAY_CLOSE,
                        closure,
                        PromiseClosure::HostCallback);
    return closure->getPromise();
}

namespace {

v8::Local<v8::Value> create_surface_invoke(const gl::Shared<gl::RenderClientObject>& display,
                                           bool hwCompose,
                                           v8::Isolate *isolate, int32_t width, int32_t height)
{
    if (width <= 0 || height <= 0)
        g_throw(RangeError, "Surface width and height must be positive integers");

    using Sp = gl::Shared<gl::RenderClientObject>;
    auto closure = PromiseClosure::New(isolate,
                                       PromiseClosure::CreateObjectConverter<SurfaceWrap, Sp>);

    // TODO: Select a appropriate color format
    display->Invoke(hwCompose ? GLOP_DISPLAY_CREATE_HW_COMPOSE_SURFACE : GLOP_DISPLAY_CREATE_RASTER_SURFACE,
                    closure,
                    PromiseClosure::HostCallback,
                    width, height, SkColorType::kBGRA_8888_SkColorType);

    return closure->getPromise();
}

} // namespace anonymous

v8::Local<v8::Value> DisplayWrap::createRasterSurface(int32_t width, int32_t height)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return create_surface_invoke(getObject(), false, isolate, width, height);
}

v8::Local<v8::Value> DisplayWrap::createHWComposeSurface(int32_t width, int32_t height)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return create_surface_invoke(getObject(), true, isolate, width, height);
}

v8::Local<v8::Value> DisplayWrap::requestMonitorList()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    auto closure = PromiseClosure::New(isolate, [this](v8::Isolate *i,
            gl::RenderHostCallbackInfo& info) -> v8::Local<v8::Value> {
        // Receive responded monitor objects and add them to local `monitor_objects_map_`
        v8::EscapableHandleScope scope(i);

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
            {
                monitor_obj = monitor_objects_map_[monitor].Get(i);
            }

            objects.push_back(monitor_obj);
        }

        return scope.Escape(binder::to_v8(i, objects));
    });

    getObject()->Invoke(GLOP_DISPLAY_REQUEST_MONITOR_LIST, closure, PromiseClosure::HostCallback);

    return closure->getPromise();
}

v8::Local<v8::Value> DisplayWrap::loadCursorTheme(const std::string& name, int size)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    using W = CursorThemeWrap;
    using T = gl::Shared<gl::CursorTheme>;
    auto closure = PromiseClosure::New(isolate, PromiseClosure::CreateObjectConverter<W, T>);
    getObject()->Invoke(GLOP_DISPLAY_LOAD_CURSOR_THEME, closure,
                        PromiseClosure::HostCallback, name, size);
    return closure->getPromise();
}

v8::Local<v8::Value> DisplayWrap::createCursor(v8::Local<v8::Value> bitmap, int hotspotX, int hotspotY)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    CkBitmapWrap *unwrapped = binder::UnwrapObject<CkBitmapWrap>(isolate, bitmap);
    if (!unwrapped)
        g_throw(TypeError, "Argument \'bitmap\' must be an instance of CkBitmap");

    std::shared_ptr<SkBitmap> bitmap_extracted = unwrapped->getBitmap();
    using W = CursorWrap;
    using T = gl::Shared<gl::Cursor>;
    auto closure = PromiseClosure::New(isolate, PromiseClosure::CreateObjectConverter<W, T>);
    getObject()->Invoke(GLOP_DISPLAY_CREATE_CURSOR, closure,
                        PromiseClosure::HostCallback, bitmap_extracted,
                        hotspotX, hotspotY);

    return closure->getPromise();
}

v8::Local<v8::Value> DisplayWrap::getDefaultCursorTheme()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!default_cursor_theme_.IsEmpty())
        return default_cursor_theme_.Get(isolate);

    auto theme = getObject()->As<gl::Display>()->GetDefaultCursorTheme();
    CHECK(theme);

    auto obj = binder::NewObject<CursorThemeWrap>(isolate, theme);
    default_cursor_theme_.Reset(isolate, obj);

    return obj;
}

namespace {

InfoAcceptorResult monitor_property_set_transcription(v8::Isolate *isolate,
                                                      gl::RenderHostSlotCallbackInfo& info)
{
    auto props = info.Get<gl::Shared<gl::Monitor::PropertySet>>(0);
    std::map<std::string_view, v8::Local<v8::Value>> fields_map = {
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

    std::vector<v8::Local<v8::Value>> result{binder::to_v8(isolate, fields_map)};
    return std::move(result);
}

} // namespace anonymous

MonitorWrap::MonitorWrap(gl::Shared<gl::RenderClientObject> object)
    : RenderClientObjectWrap(std::move(object))
{
    defineSignal("properties-changed", GLSI_MONITOR_PROPERTIES_CHANGED, monitor_property_set_transcription);
    defineSignal("detached", GLSI_MONITOR_DETACHED, {});
}

v8::Local<v8::Value> MonitorWrap::requestPropertySet()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    auto closure = PromiseClosure::New(isolate, nullptr);
    getObject()->Invoke(GLOP_MONITOR_REQUEST_PROPERTIES, closure, PromiseClosure::HostCallback);

    return closure->getPromise();
}

GALLIUM_BINDINGS_GLAMOR_NS_END
