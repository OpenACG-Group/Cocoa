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

#include "Core/EventLoop.h"
#include "Utau/AudioSinkStream.h"
#include "Gallium/bindings/utau/Exports.h"
#include "Gallium/binder/Class.h"
GALLIUM_BINDINGS_UTAU_NS_BEGIN

void AudioSinkStreamWrap::dispose()
{
    stream_->Dispose();
}

namespace {

template<typename T>
T safe_cast_enum(int32_t v, const char *argname)
{
    if (v < 0 || v > static_cast<int32_t>(T::kLast))
        g_throw(RangeError, fmt::format("Invalid enumeration value for argument `{}`", argname));

    return static_cast<T>(v);
}

} // namespace anonymous

void AudioSinkStreamWrap::connect(int32_t sample_fmt, int32_t ch_mode,
                                  int32_t sample_rate, bool realtime)
{
    auto res = stream_->Connect(safe_cast_enum<utau::SampleFormat>(sample_fmt, "sampleFormat"),
                                safe_cast_enum<utau::AudioChannelMode>(ch_mode, "channelMode"),
                                sample_rate,
                                realtime);

    if (res == utau::AudioSinkStream::ConnectStatus::kAlready)
        g_throw(Error, "Failed to connect stream: already connected");
    else if (res == utau::AudioSinkStream::ConnectStatus::kError)
        g_throw(Error, "Failed to connect stream: errors occurred");
}

void AudioSinkStreamWrap::disconnect()
{
    auto res = stream_->Disconnect();
    if (res == utau::AudioSinkStream::ConnectStatus::kAlready)
        g_throw(Error, "Failed to connect stream: already connected");
    else if (res == utau::AudioSinkStream::ConnectStatus::kError)
        g_throw(Error, "Failed to connect stream: errors occurred");
}

void AudioSinkStreamWrap::enqueue(v8::Local<v8::Value> buffer)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    auto *wrapper = binder::Class<AudioBufferWrap>::unwrap_object(isolate, buffer);
    if (!wrapper)
        g_throw(TypeError, "Argument `buffer` must be an instance of `AudioBuffer`");

    bool res = stream_->Enqueue(*wrapper->GetBuffer());

    if (!res)
        g_throw(Error, "Failed to enqueue the audio buffer");
}

double AudioSinkStreamWrap::getCurrentDelayInUs()
{
    return stream_->GetDelayInUs();
}

GALLIUM_BINDINGS_UTAU_NS_END
