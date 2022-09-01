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

#include <spa/param/audio/format-utils.h>
#include <pipewire/pipewire.h>

#include "Core/Errors.h"
#include "Core/Journal.h"
#include "Core/Exception.h"
#include "Utau/PipewireAudioServiceProvider.h"
#include "Utau/PipewireAudioPlaybackStream.h"
UTAU_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Utau.PipewireAudioPlaybackStream)

namespace {

const pw_stream_events g_stream_events = {
    .version = PW_VERSION_STREAM_EVENTS,
    .process = nullptr
};

} // namespace anonymous

std::shared_ptr<PipewireAudioPlaybackStream>
PipewireAudioPlaybackStream::Make(const std::shared_ptr<PipewireAudioServiceProvider>& provider,
                                  const std::string& name,
                                  MediaRole role,
                                  int channels,
                                  SampleFormat sample_format,
                                  int sample_rate)
{
    CHECK(provider && "Null audio service provider");
    CHECK(!name.empty() && "Null stream name");

    pw_loop *pipewire_loop = provider->GetPipewireLoop();
    CHECK(pipewire_loop && "Corrupted pipewire loop");

    // Create a wrapped stream object as we need it for `pw_stream_new_simple`.
    // The wrapped stream object will be captured as closure by callbacks of stream events.
    auto playback_stream = std::make_shared<PipewireAudioPlaybackStream>(provider, name,
                                                                         StreamInfo{
                                                                            channels,
                                                                            sample_rate,
                                                                            sample_format
                                                                         });
    CHECK(playback_stream && "Bad allocation");
    playback_stream->pipewire_loop_ = pipewire_loop;

    // Create a playback stream via pipewire stream API
    pw_properties *stream_props = pw_properties_new(PW_KEY_MEDIA_TYPE, "Audio",
                                                    PW_KEY_MEDIA_CATEGORY, "Playback",
                                                    PW_KEY_MEDIA_ROLE, MediaRoleToString(role),
                                                    nullptr);
    ScopeExitAutoInvoker propsReleaser([stream_props]() {
        // FIXME: Is this redundant?
        pw_properties_clear(stream_props);
    });

    pw_stream *pipewire_stream = pw_stream_new_simple(pipewire_loop,
                                                      name.c_str(),
                                                      stream_props,
                                                      &g_stream_events,
                                                      playback_stream.get());
    if (!pipewire_stream)
    {
        QLOG(LOG_ERROR, "Failed to create a playback stream via pipewire");
        return nullptr;
    }
    playback_stream->pipewire_stream_ = pipewire_stream;

    propsReleaser.cancel();
    return playback_stream;
}

PipewireAudioPlaybackStream::~PipewireAudioPlaybackStream()
{
    if (pipewire_stream_)
        pw_stream_destroy(pipewire_stream_);
}

void PipewireAudioPlaybackStream::OnDispose()
{
    if (pipewire_stream_)
    {
        pw_stream_destroy(pipewire_stream_);
        pipewire_stream_ = nullptr;
    }
}

UTAU_NAMESPACE_END
