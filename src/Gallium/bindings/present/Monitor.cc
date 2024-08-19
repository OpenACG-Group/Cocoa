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

#include "Gallium/bindings/present/Monitor.h"
#include "Gallium/bindings/present/Promisify.h"
GALLIUM_BINDINGS_PRESENT_NS_BEGIN

namespace {

SignalArgsVector monitor_property_set_transcription(v8::Isolate *isolate, gl::PresentSignalArgs& info)
{
    auto props = info.Get<std::shared_ptr<gl::Monitor::PropertySet>>(0);
    return { ffi::Cast<ffi::ObjectLiteralMap>::ToChecked(isolate, {
        { "logicalX", v8::Int32::New(isolate, props->logical_position.x()) },
        { "logicalY", v8::Int32::New(isolate, props->logical_position.y()) },
        { "physicalWidth", v8::Int32::New(isolate, props->physical_metrics.x()) },
        { "physicalHeight", v8::Int32::New(isolate, props->physical_metrics.y()) },
        { "subpixel", v8::Int32::New(isolate, static_cast<int32_t>(props->subpixel)) },
        { "manufactureName", ffi::Cast<std::string>::ToChecked(isolate, props->manufacture_name) },
        { "modelName", ffi::Cast<std::string>::ToChecked(isolate, props->model_name) },
        { "transform", v8::Int32::New(isolate, static_cast<int32_t>(props->transform)) },
        { "modeFlags", v8::Uint32::NewFromUnsigned(isolate, props->mode_flags.value()) },
        { "modeWidth", v8::Int32::New(isolate, props->mode_size.x()) },
        { "modeHeight", v8::Int32::New(isolate, props->mode_size.y()) },
        { "refreshRate", v8::Int32::New(isolate, props->refresh_rate_mhz) },
        { "scaleFactor", v8::Int32::New(isolate, props->scale_factor) },
        { "connectorName", ffi::Cast<std::string>::ToChecked(isolate, props->connector_name) },
        { "description", ffi::Cast<std::string>::ToChecked(isolate, props->description) }
    }) };
}

} // namespace anonymous

Monitor::Monitor(std::shared_ptr<gl::Monitor> monitor)
    : monitor_(std::move(monitor))
{
    DefineSignalEventsOnEventEmitter(this, monitor_, {
        { "properties-changed",
          GLSI_MONITOR_PROPERTIES_CHANGED, monitor_property_set_transcription },
        { "detached", GLSI_MONITOR_DETACHED }
    });
}

ffi::RetLocal<v8::Value> Monitor::requestPropertySet()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return PromisifiedRemoteCall::Call(isolate, monitor_, {}, GLOP_MONITOR_REQUEST_PROPERTIES);
}

GALLIUM_BINDINGS_PRESENT_NS_END
