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

#include "Core/EventLoop.h"
#include "Gallium/bindings/multimedia/AudioStreamService.h"
#include "Gallium/bindings/multimedia/AudioSinkStream.h"
#include "Gallium/bindings/multimedia/AChannelLayout.h"
GALLIUM_BINDINGS_MULTIMEDIA_NS_BEGIN

ffi::RetLocal<v8::Value> AudioStreamService::Connect()
{
    uv_loop_t *event_loop = EventLoop::GetCurrent()->handle();
    std::shared_ptr<utau::AudioDevice> device = utau::AudioDevice::MakePipeWire(event_loop);
    if (!device)
        return ffi::Fail(ffi::kErr, "failed to connect to PipeWire daemon");
    return ffi::JSObject::New<AudioStreamService>(v8::Isolate::GetCurrent(), device);
}

ffi::Ret<void> AudioStreamService::dispose()
{
    device_.reset();
    NotifyDisposeState(DisposeState::kDisposed);
    return {};
}

ffi::RetLocal<v8::Value>
AudioStreamService::createSinkStream(const std::string& name,
                                     ffi::Enum<SampleFormat> format,
                                     int32_t sample_rate,
                                     ffi::Class<AChannelLayout> channel_layout,
                                     bool realtime)
{
    auto sink = device_->CreateSinkStream(name, static_cast<AVSampleFormat>(format.GetInteger()),
                                          sample_rate, channel_layout->GetAVChannelLayout(), realtime);
    if (!sink)
        return ffi::Fail(ffi::kErr, "failed to create sink stream");
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<AudioSinkStream>(isolate, std::move(sink));
}

GALLIUM_BINDINGS_MULTIMEDIA_NS_END
