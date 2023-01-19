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

#ifndef COCOA_UTAU_AUDIOSINKSTREAM_H
#define COCOA_UTAU_AUDIOSINKSTREAM_H

#include <functional>

#include "Core/Errors.h"
#include "Utau/Utau.h"
UTAU_NAMESPACE_BEGIN

class AudioDevice;
class AudioBuffer;

class AudioSinkStreamEventListener
{
public:
    virtual ~AudioSinkStreamEventListener() = default;

    virtual void OnVolumeChanged(float volume) = 0;
};

class AudioSinkStream
{
public:
    AudioSinkStream();
    virtual ~AudioSinkStream() = default;

    enum class ConnectStatus
    {
        kAlready,
        kSuccess,
        kError
    };

    g_nodiscard g_inline int32_t GetUniqueId() const {
        return unique_id_;
    }

    g_nodiscard g_inline bool IsConnected() const {
        return connected_;
    }

    g_nodiscard g_inline std::shared_ptr<AudioDevice> GetDevice() {
        return this->OnGetDevice();
    }

    g_nodiscard g_inline const auto& GetEventListener() const {
        return event_listener_;
    }

    g_inline void Dispose() {
        event_listener_.reset();
        this->OnDispose();
    }

    g_inline void SetEventListener(const std::shared_ptr<AudioSinkStreamEventListener>& listener) {
        event_listener_ = listener;
    }

    ConnectStatus Connect(SampleFormat sample_format, AudioChannelMode channel_mode,
                          int32_t sample_rate, bool realtime);

    ConnectStatus Disconnect();

    virtual bool Enqueue(const AudioBuffer& buffer) = 0;

    virtual double GetDelayInUs() = 0;
    virtual float GetVolume() = 0;
    virtual void SetVolume(float volume) = 0;

protected:
    virtual void OnDispose() = 0;
    virtual std::shared_ptr<AudioDevice> OnGetDevice() = 0;
    virtual bool OnConnect(SampleFormat sample_format,
                           AudioChannelMode channel_mode,
                           int32_t sample_rate,
                           bool realtime) = 0;
    virtual bool OnDisconnect() = 0;

private:
    int32_t         unique_id_;
    bool            connected_;
    std::shared_ptr<AudioSinkStreamEventListener>
                    event_listener_;
};

UTAU_NAMESPACE_END
#endif //COCOA_UTAU_AUDIOSINKSTREAM_H
