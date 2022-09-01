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

#ifndef COCOA_UTAU_AUDIOSERVICEPROVIDER_H
#define COCOA_UTAU_AUDIOSERVICEPROVIDER_H

#include <list>
#include <memory>

#include "Utau/Utau.h"
UTAU_NAMESPACE_BEGIN

class AudioPlaybackStream;

class AudioServiceProvider : public std::enable_shared_from_this<AudioServiceProvider>
{
public:
    AudioServiceProvider() = default;
    virtual ~AudioServiceProvider() = default;

    static std::shared_ptr<AudioServiceProvider> MakePipewire();

    g_nodiscard g_inline const auto& GetPlaybackStreams() const {
        return playback_streams_;
    }

    std::shared_ptr<AudioPlaybackStream> CreatePlaybackStream(const std::string& name,
                                                              MediaRole role,
                                                              int channels,
                                                              SampleFormat sample_format,
                                                              int sample_rate);

protected:
    virtual std::shared_ptr<AudioPlaybackStream> OnCreatePlaybackStream(const std::string& name,
                                                                        MediaRole role,
                                                                        int channels,
                                                                        SampleFormat sample_format,
                                                                        int sample_rate) = 0;

private:
    std::list<std::shared_ptr<AudioPlaybackStream>>     playback_streams_;
};

UTAU_NAMESPACE_END
#endif //COCOA_UTAU_AUDIOSERVICEPROVIDER_H
