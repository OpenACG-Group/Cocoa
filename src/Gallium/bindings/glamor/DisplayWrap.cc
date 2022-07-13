#include "Gallium/bindings/glamor/Exports.h"
#include "Gallium/bindings/glamor/PromiseHelper.h"

#include "Glamor/Display.h"
#include "Glamor/Monitor.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

DisplayWrap::DisplayWrap(glamor::Shared<glamor::RenderClientObject> object)
        : RenderClientObjectWrap(std::move(object))
{
    defineSignal("closed", CRSI_DISPLAY_CLOSED, nullptr);
    defineSignal("monitor-added", CRSI_DISPLAY_MONITOR_ADDED,
                 [this](v8::Isolate *i, glamor::RenderHostSlotCallbackInfo& info) -> InfoAcceptorResult {
        v8::HandleScope scope(i);
        auto monitor = info.Get<glamor::Shared<glamor::Monitor>>(0);

        v8::Local<v8::Object> result = binder::Class<MonitorWrap>::create_object(i, monitor);
        this->monitor_objects_map_[monitor].Reset(i, result);

        return {std::vector<v8::Local<v8::Value>>{result}};
    });
    defineSignal("monitor-removed", CRSI_DISPLAY_MONITOR_REMOVED,
                 [this](v8::Isolate *i, glamor::RenderHostSlotCallbackInfo& info) -> InfoAcceptorResult {
        v8::HandleScope scope(i);
        auto monitor = info.Get<glamor::Shared<glamor::Monitor>>(0);

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
            result = binder::Class<MonitorWrap>::create_object(i, monitor);
        }

        return {std::vector<v8::Local<v8::Value>>{result}};
    });
}

DisplayWrap::~DisplayWrap() = default;

v8::Local<v8::Value> DisplayWrap::close()
{
    auto closure = PromiseClosure::New(v8::Isolate::GetCurrent(), nullptr);
    getObject()->Invoke(CROP_DISPLAY_CLOSE,
                        closure,
                        PromiseClosure::HostCallback);
    return closure->getPromise();
}

namespace {

v8::Local<v8::Value> create_surface_invoke(const i::Shared<i::RenderClientObject>& display,
                                           bool hwCompose,
                                           v8::Isolate *isolate, int32_t width, int32_t height)
{
    if (width <= 0 || height <= 0)
        g_throw(RangeError, "Surface width and height must be positive integers");

    using Sp = i::Shared<i::RenderClientObject>;
    auto closure = PromiseClosure::New(isolate,
                                       PromiseClosure::CreateObjectConverter<SurfaceWrap, Sp>);

    // TODO: Select a appropriate color format
    display->Invoke(hwCompose ? CROP_DISPLAY_CREATE_HW_COMPOSE_SURFACE : CROP_DISPLAY_CREATE_RASTER_SURFACE,
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
            glamor::RenderHostCallbackInfo& info) -> v8::Local<v8::Value> {
        // Receive responded monitor objects and add them to local `monitor_objects_map_`
        v8::EscapableHandleScope scope(i);

        auto list = info.GetReturnValue<glamor::Display::MonitorList>();
        std::vector<v8::Local<v8::Object>> objects;

        for (const auto& monitor : list)
        {
            CHECK(monitor);
            v8::Local<v8::Object> monitor_obj;

            if (this->monitor_objects_map_.count(monitor) == 0)
            {
                monitor_obj = binder::Class<MonitorWrap>::create_object(i, monitor);
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

    getObject()->Invoke(CROP_DISPLAY_REQUEST_MONITOR_LIST, closure, PromiseClosure::HostCallback);

    return closure->getPromise();
}

namespace {

InfoAcceptorResult monitor_property_set_transcription(v8::Isolate *isolate,
                                                      glamor::RenderHostSlotCallbackInfo& info)
{
    auto props = info.Get<glamor::Shared<glamor::Monitor::PropertySet>>(0);
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

MonitorWrap::MonitorWrap(glamor::Shared<glamor::RenderClientObject> object)
    : RenderClientObjectWrap(std::move(object))
{
    // TODO: test this class and signal
    defineSignal("properties-changed", CRSI_MONITOR_PROPERTIES_CHANGED,
                 monitor_property_set_transcription);
}

v8::Local<v8::Value> MonitorWrap::requestPropertySet()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    auto closure = PromiseClosure::New(isolate, nullptr);
    getObject()->Invoke(CROP_MONITOR_REQUEST_PROPERTIES, closure, PromiseClosure::HostCallback);

    return closure->getPromise();
}

GALLIUM_BINDINGS_GLAMOR_NS_END
