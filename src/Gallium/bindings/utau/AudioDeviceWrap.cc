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
#include "Utau/AudioDevice.h"
#include "Gallium/bindings/utau/Exports.h"
#include "Gallium/binder/Class.h"
GALLIUM_BINDINGS_UTAU_NS_BEGIN

v8::Local<v8::Value> AudioDeviceWrap::ConnectPipeWire()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    auto device = utau::AudioDevice::MakePipeWire(EventLoop::GetCurrent()->handle());
    if (!device)
        g_throw(Error, "Failed to connect to PipeWire daemon");

    return binder::NewObject<AudioDeviceWrap>(isolate, device);
}

void AudioDeviceWrap::unref()
{
    device_.reset();
}

v8::Local<v8::Value> AudioDeviceWrap::createSinkStream(const std::string& name)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    auto stream = device_->CreateSinkStream(name);
    if (!stream)
        g_throw(Error, "Failed to create an audio sink stream");

    return binder::NewObject<AudioSinkStreamWrap>(isolate, std::move(stream));
}

GALLIUM_BINDINGS_UTAU_NS_END
